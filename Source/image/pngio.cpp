#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <png.h>
#include <zlib.h>
#include <iostream>

#include "meta/meta.h"
#include "image/image.h"
#include "image/pngio.h"

using namespace rvg;

static void user_error_fn(png_structp png_ptr,
	png_const_charp error_msg) {
	(void) png_ptr;
	fprintf(stderr, "libpng error: %s\n", error_msg);
}

static void user_warning_fn(png_structp png_ptr,
	png_const_charp warning_msg) {
	(void) png_ptr;
	fprintf(stderr, "libpng warning: %s\n", warning_msg);
}

template <typename T> int to_bit_depth(void);
template <> int to_bit_depth<png_uint_16>(void) { return 16; }
template <> int to_bit_depth<png_byte>(void) { return 8; }

struct Io {
    union {
        struct {
            char *data;
            size_t pos, size;
        } memory;
        FILE *file;
    } context;
    size_t (*io)(Io *self, char *data, size_t len);
    void (*seek)(Io *self, size_t to);
    size_t (*tell)(Io *self);
    void (*done)(Io *self);
};

static void io_file_seek(Io *self, size_t to) {
    fseek(self->context.file, SEEK_SET, static_cast<long int>(to));
}

static size_t io_file_tell(Io *self) {
    return ftell(self->context.file);
}

static void io_file_done(Io *self) {
    (void) self;
}

static size_t io_file_reader_io(Io *self, char *out, size_t len) {
    return fread(out, 1, len, self->context.file);
}

static size_t io_file_writer_io(Io *self, char *in, size_t len) {
    return fwrite(in, 1, len, self->context.file);
}

void io_file_reader_init(Io *self, FILE *file) {
    self->io = io_file_reader_io;
    self->seek = io_file_seek;
    self->tell = io_file_tell;
    self->done = io_file_done;
    self->context.file = file;
}

void io_file_writer_init(Io *self, FILE *file) {
    self->io = io_file_writer_io;
    self->seek = io_file_seek;
    self->tell = io_file_tell;
    self->done = io_file_done;
    self->context.file = file;
}

static void io_memory_seek(Io *self, size_t to) {
    self->context.memory.pos = std::min(self->context.memory.size, to);
}

static size_t io_memory_tell(Io *self) {
    return self->context.memory.pos;
}

static void io_memory_writer_done(Io *self) {
    free(self->context.memory.data);
}

static void io_memory_reader_done(Io *self) {
    (void) self;
}

static size_t io_memory_reader_io(Io *self, char *out, size_t len) {
    len = std::min(len, self->context.memory.size - self->context.memory.pos);
    memcpy(out, self->context.memory.data + self->context.memory.pos, len);
    self->context.memory.pos += len;
    return len;
}

static size_t io_memory_writer_io(Io *self, char *in, size_t len) {
    // need to realloc
    if (self->context.memory.pos + len > self->context.memory.size) {
        size_t newsize = std::max(2*self->context.memory.size, static_cast<size_t>(64*1024));
        char *tmp = reinterpret_cast<char *>(realloc(self->context.memory.data,
            newsize));
        if (!tmp) return 0;
        self->context.memory.data = tmp;
        self->context.memory.size = newsize;
    }
    memcpy(self->context.memory.data + self->context.memory.pos, in, len);
    self->context.memory.pos += len;
    return len;
}

void io_memory_writer_init(Io *self) {
    self->io = io_memory_writer_io;
    self->seek = io_memory_seek;
    self->tell = io_memory_tell;
    self->done = io_memory_writer_done;
    self->context.memory.data = nullptr;
    self->context.memory.pos = 0;
    self->context.memory.size = 0;
}

void io_memory_reader_init(Io*self, char *data, size_t len) {
    self->io = io_memory_reader_io;
    self->seek = io_memory_seek;
    self->tell = io_memory_tell;
    self->done = io_memory_reader_done;
    self->context.memory.data = data;
    self->context.memory.pos = 0;
    self->context.memory.size = len;
}

void io_fn(png_structp png_ptr, png_bytep data, png_size_t len) {
    Io *driver = reinterpret_cast<Io *>(png_get_io_ptr(png_ptr));
    if (!driver) {
        fprintf(stderr, "invalid pointer\n");
        longjmp(png_jmpbuf(png_ptr), 1);
    }
    size_t done = driver->io(driver, reinterpret_cast<char *>(data),
        static_cast<size_t>(len));
    if (done != len) {
        fprintf(stderr, "IO error\n");
        longjmp(png_jmpbuf(png_ptr), 1);
    }
}


template <typename U, typename T, size_t N, size_t... Is>
void load_helper(int width, int height, const U *base, image::Image<T,N> *out,
    meta::sequence<Is...>) {
    constexpr int n = static_cast<int>(N);
    out->load(width, height, n*width, n, image::Converter<U,T>(), (base+Is)...);
}

template <typename U, typename T, size_t N, size_t... Is>
void store_helper(int width, int height, const image::Image<T,N> &in,
    U *base, meta::sequence<Is...>) {
    constexpr int n = static_cast<int>(N);
    in.store(width, height, n*width, n, image::Converter<T,U>(), (base+Is)...);
}

namespace rvg {
    namespace image{
        namespace pngio {

int describe(Io *reader, int *out_width, int *out_height,
    int *out_channels, int *out_bit_depth) {
    // save reader position
    size_t position = reader->tell(reader);
    // libpng structures
    png_structp png_ptr = nullptr;
    png_infop info_ptr = nullptr;
    // check if it is a PNG file
    char signature[8];
    if (reader->io(reader, signature, 8) < 8) {
        fprintf(stderr, "unable to read from file\n");
        return 0;
    }
    if (png_sig_cmp((unsigned char *)signature, 0, 8)) {
        fprintf(stderr, "not a PNG");
        return 0;
    }
    // allocate reading structures
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr,
        user_error_fn, user_warning_fn);
    if (png_ptr) {
        info_ptr = png_create_info_struct(png_ptr);
    }
    if (!png_ptr || !info_ptr) {
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        fprintf(stderr, "unable to allocate structures\n");
        return 0;
    }
    // setup long jump for error return
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        return 0;
    }
    png_set_read_fn(png_ptr, reader, io_fn);
    png_set_sig_bytes(png_ptr, 8); // already skept 8 bytes
    // load image information
    png_read_info(png_ptr, info_ptr);
    // get image information
    if (out_height)
        *out_height = png_get_image_height(png_ptr, info_ptr);
    if (out_width)
        *out_width = png_get_image_width(png_ptr, info_ptr);
    int color_type = png_get_color_type(png_ptr, info_ptr);
    int bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    bool has_alpha = (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) ||
        (color_type == PNG_COLOR_TYPE_RGB_ALPHA);
    // get transparency from chunk if available
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        has_alpha = true;
    }
    bool has_color = (color_type == PNG_COLOR_TYPE_RGB ||
        color_type == PNG_COLOR_TYPE_RGB_ALPHA);
    // check palletes for color and alpha
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_colorp plte_colors;
        int num_plte_colors;
        if (png_get_PLTE(png_ptr, info_ptr, &plte_colors, &num_plte_colors)) {
            bit_depth = 8;
            for (int i = 0; i < num_plte_colors; i++) {
                const png_color &c = plte_colors[i];
                if (c.red != c.green || c.green != c.blue) {
                    has_color = true;
                    break;
                }
            }
        }
        png_sPLT_tp splt;
        int num_splt = png_get_sPLT(png_ptr, info_ptr, &splt);
        for (int i = 0; i < num_splt; i++) {
            bit_depth = (bit_depth < splt[i].depth)? splt[i].depth: bit_depth;
            for (int j = 0; j < splt[i].nentries; j++) {
                const png_sPLT_entry &c = splt[i].entries[j];
                if (c.red != c.green || c.green != c.blue) {
                    has_color = true;
                }
                if ((bit_depth == 8 && c.alpha < 255) ||
                    (bit_depth == 16 && c.alpha < 65535)) {
                    has_alpha = true;
                }
                if (has_alpha && has_color) break;
            }
        }
    }
    if (bit_depth < 8)
        bit_depth = 8;
    if (out_channels) {
        int channels = 1;
        if (has_color) channels += 2;
        if (has_alpha) channels += 1;
        *out_channels = channels;
    }
    if (out_bit_depth) {
        *out_bit_depth = bit_depth;
    }
    // restore stream position
    reader->seek(reader, position);
    // release used memory
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    return 1;
}

int describe(FILE *file, int *width, int *height,
    int *channels, int *bit_depth) {
    Io reader;
    io_file_reader_init(&reader, file);
    int ret = describe(&reader, width, height, channels, bit_depth);
    reader.done(&reader);
    return ret;
}

int describe(const std::string &memory, int *width, int *height,
    int *channels, int *bit_depth) {
    Io reader;
    io_memory_reader_init(&reader, const_cast<char *>(&memory[0]),
        memory.size());
    int ret = describe(&reader, width, height, channels, bit_depth);
    reader.done(&reader);
    return ret;
}

template <typename T, size_t N,
    typename = typename std::enable_if<(N > 0 && N <= 4)>::type>
int load(Io *reader, image::Image<T,N> *out,
    std::vector<image::Attribute> *attrs) {
    // temporary image storage
    png_bytepp volatile row_pointers = nullptr;
    png_bytep volatile buffer = nullptr;
    // libpng structures
    png_structp png_ptr = nullptr;
    png_infop info_ptr = nullptr;
    // check if it is a PNG file
    char signature[8];
    if (reader->io(reader, signature, 8) < 8) {
        fprintf(stderr, "unable to read from file\n");
        return 0;
    }
    if (png_sig_cmp((unsigned char *)signature, 0, 8)) {
        fprintf(stderr, "not a PNG");
        return 0;
    }
    // allocate reading structures
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr,
        user_error_fn, user_warning_fn);
    if (png_ptr) {
        info_ptr = png_create_info_struct(png_ptr);
    }
    if (!png_ptr || !info_ptr) {
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        fprintf(stderr, "unable to allocate structures\n");
        return 0;
    }
    // setup long jump for error return
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_free(png_ptr, row_pointers);
        png_free(png_ptr, buffer);
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        return 0;
    }
    png_set_read_fn(png_ptr, reader, io_fn);
    png_set_sig_bytes(png_ptr, 8); // already skept 8 bytes
    // load image information
    png_read_info(png_ptr, info_ptr);
    // do not premultiply alpha
    png_set_alpha_mode(png_ptr, PNG_ALPHA_PNG, PNG_DEFAULT_sRGB);
    // set output gamma to sRGB
    png_set_gamma(png_ptr, PNG_DEFAULT_sRGB, PNG_DEFAULT_sRGB);
    // get image information
    int height = png_get_image_height(png_ptr, info_ptr);
    int width = png_get_image_width(png_ptr, info_ptr);
    int color_type = png_get_color_type(png_ptr, info_ptr);
    int bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    // should we convert from pallete?
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr);
    }
    // get transparency from chunk if available
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png_ptr);
    }
    // should we get rid of the alpha channel?
    static bool drop_alpha = (N == 1 || N == 3);
    if (drop_alpha) {
        if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA ||
            color_type == PNG_COLOR_TYPE_RGB_ALPHA ||
            png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
            png_color_16 background;
            background.red = background.green = 0xFFFF;
            background.blue = background.gray = 0xFFFF;
            png_set_background(png_ptr, &background,
                PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);
         }
    }
    // should we add an alpha channel?
    if ((color_type == PNG_COLOR_TYPE_RGB ||
        color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_PALETTE) && (N == 2 || N == 4)) {
        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
            png_set_tRNS_to_alpha(png_ptr);
        } else {
            png_set_add_alpha(png_ptr, 0xFFFF, PNG_FILLER_AFTER);
        }
    }
    // should we convert from rgb to gray?
    if ((color_type == PNG_COLOR_TYPE_RGB ||
        color_type == PNG_COLOR_TYPE_RGB_ALPHA ||
        color_type == PNG_COLOR_TYPE_PALETTE) && N < 3) {
        png_set_rgb_to_gray(png_ptr, 1, 0.212671, 0.715160);
    }
    // should we convert from gray to rgb?
    if ((color_type == PNG_COLOR_TYPE_GRAY ||
         color_type == PNG_COLOR_TYPE_GRAY_ALPHA) && (N > 2)) {
        png_set_gray_to_rgb(png_ptr);
    }
    // should we flip endianness?
    static bool depth_16 = (sizeof(T) > 1);
    if (depth_16) {
        long int a = 1;
        int swap = (*((unsigned char *) &a) == 1);
        if (swap) {
            png_set_swap(png_ptr);
        }
    }
    // should we expand to 16 bits
    if (bit_depth < 16 && sizeof(T) > 1) {
        png_set_expand_16(png_ptr);
    }
    // should we expand to 8 bits?
    if (bit_depth < 8 && sizeof(T) == 1) {
        png_set_expand(png_ptr);
    }
    // should we strip to 8 bits?
    if (bit_depth > 8 && sizeof(T) == 1) {
        png_set_strip_16(png_ptr);
    }
    // automatically handle interlacing
    png_set_interlace_handling(png_ptr);
    // get and check image information after transformations
    png_read_update_info(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);
    width = png_get_image_width(png_ptr, info_ptr);
    color_type = png_get_color_type(png_ptr, info_ptr);
    bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    assert((bit_depth == 8 && sizeof(T) == 1) ||
           (bit_depth == 16 && sizeof(T) > 1));
    assert(color_type == PNG_COLOR_TYPE_GRAY || N != 1);
    assert(color_type == PNG_COLOR_TYPE_GRAY_ALPHA || N != 2);
    assert(color_type == PNG_COLOR_TYPE_RGB || N != 3);
    assert(color_type == PNG_COLOR_TYPE_RGB_ALPHA || N != 4);
    int pixel_size = (bit_depth == 8)? N: 2*N;
    // allocate data for reading
    buffer = reinterpret_cast<png_bytep>(
        png_malloc(png_ptr, height*width*pixel_size));
    row_pointers = reinterpret_cast<png_bytepp>(
        png_malloc(png_ptr, height*sizeof(png_bytep)));
    for (int i = 0; i < height; i++) {
        row_pointers[i] = &buffer[(height-1-i)*width*pixel_size];
    }
    // read attributes
    if (attrs) {
        png_textp png_attrs = nullptr;
        int num_text = 0;
        png_get_text(png_ptr, info_ptr, &png_attrs, &num_text);
        for (int i = 0; i < num_text; i++)
            attrs->emplace_back(png_attrs[i].key, png_attrs[i].text);
    }
    // read image
    png_read_image(png_ptr, row_pointers);
    //// save to image object
    if (bit_depth == 8) {
        uint8_t *base = reinterpret_cast<uint8_t *>(buffer);
        load_helper(width, height, base, out, meta::make_sequence<N>{});
    } else {
        uint16_t *base = reinterpret_cast<uint16_t *>(buffer);
        load_helper(width, height, base, out, meta::make_sequence<N>{});
    }
    // finish advancing file pointer to end of stream (useful?)
    png_read_end(png_ptr, nullptr);
    // release data;
    png_free(png_ptr, buffer);
    png_free(png_ptr, row_pointers);
    // clean-up and we are done
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    // let everybody know the data is encoded with gamma
    out->set_gamma(image::Gamma::sRGB);
    return 1;
}

template <typename T, size_t N>
int load(FILE *file, image::Image<T,N> *out,
    std::vector<image::Attribute> *attrs) {
    Io reader;
    io_file_reader_init(&reader, file);
    int ret = load(&reader, out, attrs);
    reader.done(&reader);
    return ret;
}

template <typename T, size_t N>
int load(const std::string &memory, image::Image<T,N> *out,
    std::vector<image::Attribute> *attrs) {
    Io reader;
    io_memory_reader_init(&reader, const_cast<char *>(&memory[0]),
        memory.size());
    int ret = load(&reader, out, attrs);
    reader.done(&reader);
    return ret;
}

// instantiate all required overloads
template int load(FILE *file, image::Image<uint8_t, 1> *out,
    std::vector<image::Attribute> *attrs);
template int load(FILE *file, image::Image<uint8_t, 2> *out,
    std::vector<image::Attribute> *attrs);
template int load(FILE *file, image::Image<uint8_t, 3> *out,
    std::vector<image::Attribute> *attrs);
template int load(FILE *file, image::Image<uint8_t, 4> *out,
    std::vector<image::Attribute> *attrs);

template int load(FILE *file, image::Image<uint16_t, 1> *out,
    std::vector<image::Attribute> *attrs);
template int load(FILE *file, image::Image<uint16_t, 2> *out,
    std::vector<image::Attribute> *attrs);
template int load(FILE *file, image::Image<uint16_t, 3> *out,
    std::vector<image::Attribute> *attrs);
template int load(FILE *file, image::Image<uint16_t, 4> *out,
    std::vector<image::Attribute> *attrs);

template int load(FILE *file, image::Image<float, 1> *out,
    std::vector<image::Attribute> *attrs);
template int load(FILE *file, image::Image<float, 2> *out,
    std::vector<image::Attribute> *attrs);
template int load(FILE *file, image::Image<float, 3> *out,
    std::vector<image::Attribute> *attrs);
template int load(FILE *file, image::Image<float, 4> *out,
    std::vector<image::Attribute> *attrs);

template int load(const std::string &memory,
    image::Image<uint8_t, 1> *out,
    std::vector<image::Attribute> *attrs);
template int load(const std::string &memory,
    image::Image<uint8_t, 2> *out,
    std::vector<image::Attribute> *attrs);
template int load(const std::string &memory,
    image::Image<uint8_t, 3> *out,
    std::vector<image::Attribute> *attrs);
template int load(const std::string &memory,
    image::Image<uint8_t, 4> *out,
    std::vector<image::Attribute> *attrs);

template int load(const std::string &memory,
    image::Image<uint16_t, 1> *out,
    std::vector<image::Attribute> *attrs);
template int load(const std::string &memory,
    image::Image<uint16_t, 2> *out,
    std::vector<image::Attribute> *attrs);
template int load(const std::string &memory,
    image::Image<uint16_t, 3> *out,
    std::vector<image::Attribute> *attrs);
template int load(const std::string &memory,
    image::Image<uint16_t, 4> *out,
    std::vector<image::Attribute> *attrs);

template int load(const std::string &memory,
    image::Image<float, 1> *out,
    std::vector<image::Attribute> *attrs);
template int load(const std::string &memory,
    image::Image<float, 2> *out,
    std::vector<image::Attribute> *attrs);
template int load(const std::string &memory,
    image::Image<float, 3> *out,
    std::vector<image::Attribute> *attrs);
template int load(const std::string &memory,
    image::Image<float, 4> *out,
    std::vector<image::Attribute> *attrs);

template <typename T, size_t N>
image::IImagePtr load(Io *reader, std::vector<image::Attribute> *attrs) {
    auto p = std::make_shared<image::Image<T,N>>();
    if (load(reader, p.get(), attrs)) return p;
    else return nullptr;
}

template <typename T>
image::IImagePtr load(Io *reader, int wanted_channels,
    std::vector<image::Attribute> *attrs) {
    switch (wanted_channels) {
        case 1: return load<T,1>(reader, attrs);
        case 2: return load<T,2>(reader, attrs);
        case 3: return load<T,3>(reader, attrs);
        case 4: return load<T,4>(reader, attrs);
        default: return nullptr;
    }
}

image::IImagePtr load(Io *reader, int wanted_channels,
    std::vector<image::Attribute> *attrs) {
    int input_channels, bit_depth;
    if (describe(reader, nullptr, nullptr, &input_channels, &bit_depth)) {
        wanted_channels = (wanted_channels != 0)?
            wanted_channels: input_channels;
        if (bit_depth > 8) {
            return load<uint16_t>(reader, wanted_channels, attrs);
        } else {
            return load<uint8_t>(reader, wanted_channels, attrs);
        }
    } else return nullptr;
}

image::IImagePtr load(FILE *file, int wanted_channels,
    std::vector<image::Attribute> *attrs) {
    Io reader;
    io_file_reader_init(&reader, file);
    image::IImagePtr ret = load(&reader, wanted_channels, attrs);
    reader.done(&reader);
    return ret;
}

image::IImagePtr load(const std::string &memory, int wanted_channels,
    std::vector<image::Attribute> *attrs) {
    Io reader;
    io_memory_reader_init(&reader, const_cast<char *>(&memory[0]),
        memory.size());
    image::IImagePtr ret = load(&reader, wanted_channels, attrs);
    reader.done(&reader);
    return ret;
}

template <typename U, typename T, size_t N,
    typename = typename std::enable_if<(N > 0 && N <= 4) &&
        (std::is_same<U,uint8_t>::value ||
         std::is_same<U,uint16_t>::value) >::type>
int store(Io *writer, const image::Image<T,N> &in,
    const std::vector<image::Attribute > &attrs) {
    // temporary image storage
    png_bytepp volatile row_pointers = nullptr;
    png_bytep  volatile buffer = nullptr;
    png_text * volatile png_attrs = nullptr;
    // libpng structures
    png_structp png_ptr = nullptr;
    png_infop info_ptr = nullptr;
    // allocate reading structures
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr,
        user_error_fn, user_warning_fn);
    if (png_ptr) {
        info_ptr = png_create_info_struct(png_ptr);
    }
    if (!png_ptr || !info_ptr) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fprintf(stderr, "unable to allocate structures\n");
        return 0;
    }
    // setup long jump for error return
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_free(png_ptr, row_pointers);
        png_free(png_ptr, buffer);
        png_free(png_ptr, png_attrs);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return 0;
    }
    // copy attributes to PNG format
    if (!attrs.empty()) {
        png_attrs = (png_text *) png_malloc(png_ptr,
                attrs.size()*sizeof(png_text));
        for (unsigned i = 0; i < attrs.size(); ++i) {
            png_attrs[i].compression = PNG_TEXT_COMPRESSION_NONE;
            png_attrs[i].key = const_cast<char *>(attrs[i].first.c_str());
            png_attrs[i].text = const_cast<char *>(attrs[i].second.c_str());
        }
        png_set_text(png_ptr, info_ptr, png_attrs, (int) attrs.size());
    }
    png_set_write_fn(png_ptr, writer, io_fn, nullptr);
    int height = in.height();
    int width = in.width();
    int color_type;
    switch (N) {
        case 1: color_type = PNG_COLOR_TYPE_GRAY; break;
        case 2: color_type = PNG_COLOR_TYPE_GRAY_ALPHA; break;
        case 3: color_type = PNG_COLOR_TYPE_RGB; break;
        case 4: color_type = PNG_COLOR_TYPE_RGB_ALPHA; break;
    }
    int bit_depth = sizeof(U) == 1? 8: 16;
    int pixel_size = N * bit_depth/8;
    // set basic image parameters
    png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth,
        color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT);
    static bool has_color = (N == 3 || N == 4);
    // set gamma
    switch (in.gamma()) {
        case image::Gamma::sRGB:
            if (has_color) {
                png_set_sRGB_gAMA_and_cHRM(png_ptr, info_ptr,
                    PNG_sRGB_INTENT_RELATIVE);
            } else {
                png_set_gAMA(png_ptr, info_ptr, 1.0/2.2);
            }
            break;
        case image::Gamma::linear:
            png_set_gAMA(png_ptr, info_ptr, 1.0);
            break;
        default:
            break;
    }
    // allocate temporary image buffer and row_pointers
    buffer = reinterpret_cast<png_bytep>(png_malloc(png_ptr,
        height*width*pixel_size));
    row_pointers = reinterpret_cast<png_bytepp>(png_malloc(png_ptr,
        height*sizeof(png_bytep)));
    // set row pointers to flip image
    for (int i = 0; i < height; i++) {
        row_pointers[i] = &buffer[(height-i-1)*width*pixel_size];
    }
    // copy into buffer
    if (bit_depth == 8) {
        uint8_t *base = reinterpret_cast<uint8_t *>(buffer);
        store_helper(width, height, in, base, meta::make_sequence<N>{});
    } else {
        uint16_t *base = reinterpret_cast<uint16_t *>(buffer);
        store_helper(width, height, in, base, meta::make_sequence<N>{});
    }
    // write image info
    png_write_info(png_ptr, info_ptr);
    // should we flip endianness?
    static bool depth_16 = (sizeof(U) > 1);
    if (depth_16) {
        long int a = 1;
        int swap = (*((unsigned char *) &a) == 1);
        if (swap) {
            png_set_swap(png_ptr);
        }
    }
    // write image buffer
    png_write_image(png_ptr, row_pointers);
    // finish advancing file pointer to end of stream (useful?)
    png_write_end(png_ptr, nullptr);
    // release data
    png_free(png_ptr, buffer);
    png_free(png_ptr, row_pointers);
    png_free(png_ptr, png_attrs);
    // clean-up and we are done
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return 1;
}

template <typename U, typename T, size_t N>
int store(FILE *file, const image::Image<T,N> &in,
    const std::vector<image::Attribute> &attrs) {
    Io writer;
    io_file_writer_init(&writer, file);
    int ret = store<U>(&writer, in, attrs);
    writer.done(&writer);
    return ret;
}

template <typename U, typename T, size_t N>
int store(std::string *memory, const image::Image<T,N> &in,
    const std::vector<image::Attribute> &attrs) {
    Io writer;
    io_memory_writer_init(&writer);
    int ret = store<U>(&writer, in, attrs);
    memory->insert(memory->end(), writer.context.memory.data,
        writer.context.memory.data+ writer.context.memory.size);
    return ret;
}

// instantiate all required overloads
template int store<uint8_t>(FILE *file, const image::Image<uint8_t, 1> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint8_t>(FILE *file, const image::Image<uint8_t, 2> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint8_t>(FILE *file, const image::Image<uint8_t, 3> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint8_t>(FILE *file, const image::Image<uint8_t, 4> &out,
    const std::vector<image::Attribute> &attrs);

template int store<uint8_t>(FILE *file, const image::Image<uint16_t, 1> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint8_t>(FILE *file, const image::Image<uint16_t, 2> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint8_t>(FILE *file, const image::Image<uint16_t, 3> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint8_t>(FILE *file, const image::Image<uint16_t, 4> &out,
    const std::vector<image::Attribute> &attrs);

template int store<uint8_t>(FILE *file, const image::Image<float, 1> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint8_t>(FILE *file, const image::Image<float, 2> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint8_t>(FILE *file, const image::Image<float, 3> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint8_t>(FILE *file, const image::Image<float, 4> &out,
    const std::vector<image::Attribute> &attrs);

template int store<uint8_t>(std::string *memory,
    const image::Image<uint8_t, 1> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint8_t>(std::string *memory,
    const image::Image<uint8_t, 2> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint8_t>(std::string *memory,
    const image::Image<uint8_t, 3> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint8_t>(std::string *memory,
    const image::Image<uint8_t, 4> &out,
    const std::vector<image::Attribute> &attrs);

template int store<uint8_t>(std::string *memory,
    const image::Image<uint16_t, 1> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint8_t>(std::string *memory,
    const image::Image<uint16_t, 2> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint8_t>(std::string *memory,
    const image::Image<uint16_t, 3> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint8_t>(std::string *memory,
    const image::Image<uint16_t, 4> &out,
    const std::vector<image::Attribute> &attrs);

template int store<uint8_t>(std::string *memory,
    const image::Image<float, 1> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint8_t>(std::string *memory,
    const image::Image<float, 2> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint8_t>(std::string *memory,
    const image::Image<float, 3> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint8_t>(std::string *memory,
    const image::Image<float, 4> &out,
    const std::vector<image::Attribute> &attrs);

template int store<uint16_t>(FILE *file, const image::Image<uint8_t, 1> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint16_t>(FILE *file, const image::Image<uint8_t, 2> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint16_t>(FILE *file, const image::Image<uint8_t, 3> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint16_t>(FILE *file, const image::Image<uint8_t, 4> &out,
    const std::vector<image::Attribute> &attrs);

template int store<uint16_t>(FILE *file, const image::Image<uint16_t, 1> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint16_t>(FILE *file, const image::Image<uint16_t, 2> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint16_t>(FILE *file, const image::Image<uint16_t, 3> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint16_t>(FILE *file, const image::Image<uint16_t, 4> &out,
    const std::vector<image::Attribute> &attrs);

template int store<uint16_t>(FILE *file, const image::Image<float, 1> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint16_t>(FILE *file, const image::Image<float, 2> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint16_t>(FILE *file, const image::Image<float, 3> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint16_t>(FILE *file, const image::Image<float, 4> &out,
    const std::vector<image::Attribute> &attrs);

template int store<uint16_t>(std::string *memory,
    const image::Image<uint8_t, 1> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint16_t>(std::string *memory,
    const image::Image<uint8_t, 2> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint16_t>(std::string *memory,
    const image::Image<uint8_t, 3> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint16_t>(std::string *memory,
    const image::Image<uint8_t, 4> &out,
    const std::vector<image::Attribute> &attrs);

template int store<uint16_t>(std::string *memory,
    const image::Image<uint16_t, 1> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint16_t>(std::string *memory,
    const image::Image<uint16_t, 2> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint16_t>(std::string *memory,
    const image::Image<uint16_t, 3> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint16_t>(std::string *memory,
    const image::Image<uint16_t, 4> &out,
    const std::vector<image::Attribute> &attrs);

template int store<uint16_t>(std::string *memory,
    const image::Image<float, 1> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint16_t>(std::string *memory,
    const image::Image<float, 2> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint16_t>(std::string *memory,
    const image::Image<float, 3> &out,
    const std::vector<image::Attribute> &attrs);
template int store<uint16_t>(std::string *memory,
    const image::Image<float, 4> &out,
    const std::vector<image::Attribute> &attrs);

template <typename U, typename T, size_t N>
int store(Io *writer, const image::IImagePtr &in_ptr,
    const std::vector<image::Attribute> &attrs) {
    return store<U>(writer, *reinterpret_cast<image::Image<T, N> *>(
        in_ptr.get()), attrs);
}

template <typename U, typename T>
int store(Io *writer, const image::IImagePtr &in_ptr,
    const std::vector<image::Attribute> &attrs) {
    switch (in_ptr->channels()) {
        case 1: return store<U, T, 1>(writer, in_ptr, attrs);
        case 2: return store<U, T, 2>(writer, in_ptr, attrs);
        case 3: return store<U, T, 3>(writer, in_ptr, attrs);
        case 4: return store<U, T, 4>(writer, in_ptr, attrs);
        default: return 0;
    }
}

template <typename U>
int store(Io *writer, const image::IImagePtr &in_ptr,
    const std::vector<image::Attribute> &attrs) {
    switch (in_ptr->channel_type()) {
        case image::ChannelType::channel_uint8_t:
            return store<U, uint8_t>(writer, in_ptr, attrs);
        case image::ChannelType::channel_uint16_t:
            return store<U, uint16_t>(writer, in_ptr, attrs);
        case image::ChannelType::channel_float:
            return store<U, float>(writer, in_ptr, attrs);
        default:
            return 0;
    }
}

template <typename U>
int store(FILE *file, const image::IImagePtr &in_ptr,
    const std::vector<image::Attribute> &attrs) {
    Io writer;
    io_file_writer_init(&writer, file);
    int ret = store<U>(&writer, in_ptr, attrs);
    writer.done(&writer);
    return ret;
}

template <typename U>
int store(std::string *memory, const image::IImagePtr &in_ptr,
    const std::vector<image::Attribute> &attrs) {
    Io writer;
    io_memory_writer_init(&writer);
    int ret = store<U>(&writer, in_ptr, attrs);
    memory->insert(memory->end(), writer.context.memory.data,
        writer.context.memory.data+writer.context.memory.size);
    return ret;
}

template int store<uint8_t>(FILE *file, const image::IImagePtr &in_ptr,
    const std::vector<image::Attribute> &attrs);
template int store<uint16_t>(FILE *file, const image::IImagePtr &in_ptr,
    const std::vector<image::Attribute> &attrs);

template int store<uint8_t>(std::string *memory,
    const image::IImagePtr &in_ptr, const std::vector<image::Attribute> &attrs);
template int store<uint16_t>(std::string *memory,
    const image::IImagePtr &in_ptr, const std::vector<image::Attribute> &attrs);

} } } // namespaces rvg::image::pngio
