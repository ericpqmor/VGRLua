#ifndef RVG_IMAGE_H
#define RVG_IMAGE_H

#include <vector>
#include <array>
#include <cassert>
#include <string>
#include <type_traits>
#include <algorithm>
#include <cstdint>

#include "image/iimage.h"

namespace rvg {
    namespace image {

typedef std::pair<std::string, std::string> Attribute;

template <typename FROM, typename TO>
struct Converter final {
    TO operator()(FROM v) const {
        return static_cast<TO>(v);
    }
};

template <typename T>
struct Converter<T,T> final {
    T operator()(T v) const {
        return v;
    }
};

template <>
struct Converter<uint8_t, float> final {
    float operator()(int i) const {
        constexpr float a = 1.f/254.f;
        constexpr float b = -.5f/254.f;
        return std::min(1.f, std::max(0.f, a*i+b));
    }
};

template <>
struct Converter<uint16_t, float> final {
    float operator()(int i) const {
        constexpr float a = 1.f/65534.f;
        constexpr float b = -.5f/65534.f;
        return std::min(1.f, std::max(0.f, a*i+b));
    }
};

template <>
struct Converter<float, uint8_t> final {
    uint8_t operator()(float f) const {
        f = std::min(255.f, std::max(0.f, f*256.f));
        return static_cast<uint8_t>(f);
    }
};

template <>
struct Converter<float, uint16_t> final {
    uint16_t operator()(float f) const {
        f = std::min(65535.f, std::max(0.f, f*65536.f));
        return static_cast<uint16_t>(f);
    }
};

template <>
struct Converter<uint8_t, uint16_t> final {
    uint16_t operator()(int i) const {
        Converter<uint8_t,float> a;
        Converter<float, uint16_t> b;
        return b(a(i));
    }
};

template <>
struct Converter<uint16_t, uint8_t> final {
    uint8_t operator()(int i) const {
        Converter<uint16_t,float> a;
        Converter<float, uint8_t> b;
        return b(a(i));
    }
};

template <typename T>
inline
ChannelType channel_type(void) {
    return ChannelType::channel_unknown;
}

template <>
inline
ChannelType channel_type<float>(void) {
    return ChannelType::channel_float;
}

template <>
inline
ChannelType channel_type<uint8_t>(void) {
    return ChannelType::channel_uint8_t;
}

template <>
inline
ChannelType channel_type<uint16_t>(void) {
    return ChannelType::channel_uint16_t;
}

template <typename T, size_t N>
class Image final: public IImage {

    int m_width, m_height, m_channel_size;
    Gamma m_gamma;
    std::vector<T> m_data;

private:

    template <typename C>
    void get_pixel_helper(int, const C&) const { }

    template <typename C, typename U, typename ...Args>
    void get_pixel_helper(int i, const C &convert, U &first,
        Args&... others) const {
        first = convert(m_data[i]);
        get_pixel_helper(i+m_channel_size, convert, others...);
    }

    template <typename C>
    void set_pixel_helper(int, const C&) { ; }

    template <typename C, typename U, typename ...Args>
    void set_pixel_helper(int i, const C& convert, const U first,
        Args... others) {
        m_data[i] = convert(first);
        set_pixel_helper(i+m_channel_size, convert, others...);
    }

public:

    virtual ~Image<T,N>() { ; }

    Image(void): m_width(0), m_height(0), m_gamma(Gamma::unknown) { ; }

    const std::vector<T> &data(void) const { return m_data; }

    template <typename C, typename ...Args,
        typename = typename std::enable_if<sizeof...(Args) == N>::type>
    void get_pixel(int x, int y, const C &convert, Args&... channels) const {
        get_pixel_helper(y*m_width+x, convert, channels...);
    }

    template <typename ...Args,
        typename = typename std::enable_if<sizeof...(Args) == N>::type>
    void get_pixel(int x, int y, Args&... channels) const {
        get_pixel_helper(y*m_width+x, Converter<T,T>(), channels...);
    }

    template <typename C, typename ...Args,
        typename = typename std::enable_if<sizeof...(Args) == N>::type>
    void set_pixel(int x, int y, const C &convert, Args... channels) {
        set_pixel_helper(y*m_width+x, convert, channels...);
    }

    template <typename ...Args,
        typename = typename std::enable_if<sizeof...(Args) == N>::type>
    void set_pixel(int x, int y, Args... channels) {
        set_pixel_helper(y*m_width+x, Converter<T,T>(), channels...);
    }

    template <typename C, typename ...Args,
        typename = typename std::enable_if<sizeof...(Args) == N>::type>
    void load(int width_in, int height_in, int pitch_in, int advance_in,
        const C &convert, const Args*... channels) {
        resize(width_in, height_in);
        int base_in = 0, base_out = 0;
        for (int i = 0; i < m_height; i++) {
            int offset_in = 0, offset_out = 0;
            for (int j = 0; j < m_width; j++) {
                set_pixel_helper(base_out+offset_out,
                    convert, channels[base_in+offset_in]...);
                offset_in += advance_in;
                offset_out += 1;
            }
            base_in += pitch_in;
            base_out += m_width;
        }
    }

    template <typename ...Args,
        typename = typename std::enable_if<sizeof...(Args) == N>::type>
    void load(int width_in, int height_in, int pitch_in, int advance_in,
        const Args*... channels) {
        load(width_in, height_in, pitch_in, advance_in,
            Converter<T,T>(), channels...);
    }

    template <typename C, typename ...Args,
        typename = typename std::enable_if<sizeof...(Args) == N>::type>
    void store(int width_out, int height_out, int pitch_out, int advance_out,
        const C& convert, Args*... channels) const {
        assert(width_out == m_width && height_out == m_height);
        if (width_out != m_width || height_out != m_height) return;
        int base_in = 0, base_out = 0;
        for (int i = 0; i < m_height; i++) {
            int offset_in = 0, offset_out = 0;
            for (int j = 0; j < m_width; j++) {
                get_pixel_helper(base_in+offset_in, convert,
                    channels[base_out+offset_out]...);
                offset_in += 1;
                offset_out += advance_out;
            }
            base_in += m_width;
            base_out += pitch_out;
        }
    }

    template <typename ...Args,
        typename = typename std::enable_if<sizeof...(Args) == N>::type>
    void store(int width_out, int height_out, int pitch_out, int advance_out,
        Args*... channels) {
        store(width_out, height_out, pitch_out, advance_out,
            Converter<T,T>(), channels...);
    }

protected:

    int do_width(void) const override { return m_width; }
    int do_height(void) const override { return m_height; }
    int do_channels(void) const override { return static_cast<int>(N); }

    ChannelType do_channel_type(void) const override {
        return image::channel_type<T>();
    }

    void do_resize(int width, int height) override {
        size_t old_size = m_data.size();
        size_t size = size_t(width*height*N);
        if (size > old_size || 2*size < old_size) {
            m_data.resize(size);
        }
        m_width = width;
        m_height = height;
        m_channel_size = width*height;
    };

    void do_set_float(int x, int y, int c, float v) override {
        Converter<float,T> conv;
        m_data[x+m_width*y+c*m_channel_size] = conv(v);
    }

    float do_get_float(int x, int y, int c) const override {
        Converter<T,float> conv;
        return conv(m_data[x+m_width*y+c*m_channel_size]);
    }

    void do_set_gamma(Gamma g) override {
        m_gamma = g;
    }

    Gamma do_gamma(void) const override {
        return m_gamma;
    }

};

} } // namespace rvg::image

#endif // RVG_IMAGE_H
