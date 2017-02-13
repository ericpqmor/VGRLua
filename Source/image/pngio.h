#ifndef RVG_IMAGE_PNGIO_H
#define RVG_IMAGE_PNGIO_H

#include <string>

#include "image/image.h"

namespace rvg {
    namespace image {
        namespace pngio {

    int describe(FILE *file, int *width, int *height,
        int *channels, int *bit_depth);

    int describe(const std::string &memory, int *width, int *height,
        int *channels, int *bit_depth);

    // load png image from file
    template <typename T, size_t N>
    int load(FILE *file, image::Image<T,N> *out,
        std::vector<image::Attribute> *attrs = nullptr);

    image::IImagePtr load(FILE *file, int wanted_channels = 0,
        std::vector<image::Attribute> *attrs = nullptr);

    // load png image from memory
    template <typename T, size_t N>
    int load(const std::string &memory, image::Image<T,N> *out,
        std::vector<image::Attribute> *attrs = nullptr);

    image::IImagePtr load(const std::string &memory, int wanted_channels = 0,
        std::vector<image::Attribute> *attrs = nullptr);

    // store png image to file
    template <typename U, typename T, size_t N>
    int store(FILE *file, const image::Image<T,N> &out,
        const std::vector<image::Attribute> &attrs =
            std::vector<image::Attribute>());

    template <typename U>
    int store(FILE *file, const image::IImagePtr &out,
        const std::vector<image::Attribute> &attrs =
            std::vector<image::Attribute>());

    // store png to memory
    template <typename U, typename T, size_t N>
    int store(std::string *memory, const image::Image<T,N> &out,
        const std::vector<image::Attribute> &attrs =
            std::vector<image::Attribute>());

    template <typename U>
    int store(std::string *memory, const image::IImagePtr &out,
        const std::vector<image::Attribute> &attrs =
            std::vector<image::Attribute>());

    // delcare explicit instantiations for all image types
    extern template int load(FILE *file, image::Image<uint8_t, 1> *out,
        std::vector<image::Attribute> *attrs);
    extern template int load(FILE *file, image::Image<uint8_t, 2> *out,
        std::vector<image::Attribute> *attrs);
    extern template int load(FILE *file, image::Image<uint8_t, 3> *out,
        std::vector<image::Attribute> *attrs);
    extern template int load(FILE *file, image::Image<uint8_t, 4> *out,
        std::vector<image::Attribute> *attrs);

    extern template int load(FILE *file, image::Image<uint16_t, 1> *out,
        std::vector<image::Attribute> *attrs);
    extern template int load(FILE *file, image::Image<uint16_t, 2> *out,
        std::vector<image::Attribute> *attrs);
    extern template int load(FILE *file, image::Image<uint16_t, 3> *out,
        std::vector<image::Attribute> *attrs);
    extern template int load(FILE *file, image::Image<uint16_t, 4> *out,
        std::vector<image::Attribute> *attrs);

    extern template int load(FILE *file, image::Image<float, 1> *out,
        std::vector<image::Attribute> *attrs);
    extern template int load(FILE *file, image::Image<float, 2> *out,
        std::vector<image::Attribute> *attrs);
    extern template int load(FILE *file, image::Image<float, 3> *out,
        std::vector<image::Attribute> *attrs);
    extern template int load(FILE *file, image::Image<float, 4> *out,
        std::vector<image::Attribute> *attrs);

    extern template int load(const std::string &memory,
        image::Image<uint8_t, 1> *out,
        std::vector<image::Attribute> *attrs);
    extern template int load(const std::string &memory,
        image::Image<uint8_t, 2> *out,
        std::vector<image::Attribute> *attrs);
    extern template int load(const std::string &memory,
        image::Image<uint8_t, 3> *out,
        std::vector<image::Attribute> *attrs);
    extern template int load(const std::string &memory,
        image::Image<uint8_t, 4> *out,
        std::vector<image::Attribute> *attrs);

    extern template int load(const std::string &memory,
        image::Image<uint16_t, 1> *out,
        std::vector<image::Attribute> *attrs);
    extern template int load(const std::string &memory,
        image::Image<uint16_t, 2> *out,
        std::vector<image::Attribute> *attrs);
    extern template int load(const std::string &memory,
        image::Image<uint16_t, 3> *out,
        std::vector<image::Attribute> *attrs);
    extern template int load(const std::string &memory,
        image::Image<uint16_t, 4> *out,
        std::vector<image::Attribute> *attrs);

    extern template int load(const std::string &memory,
        image::Image<float, 1> *out,
        std::vector<image::Attribute> *attrs);
    extern template int load(const std::string &memory,
        image::Image<float, 2> *out,
        std::vector<image::Attribute> *attrs);
    extern template int load(const std::string &memory,
        image::Image<float, 3> *out,
        std::vector<image::Attribute> *attrs);
    extern template int load(const std::string &memory,
        image::Image<float, 4> *out,
        std::vector<image::Attribute> *attrs);

    extern template int store<uint8_t>(FILE *file,
        const image::Image<uint8_t, 1> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint8_t>(FILE *file,
        const image::Image<uint8_t, 2> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint8_t>(FILE *file,
        const image::Image<uint8_t, 3> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint8_t>(FILE *file,
        const image::Image<uint8_t, 4> &in,
        const std::vector<image::Attribute> &attrs);

    extern template int store<uint8_t>(FILE *file,
        const image::Image<uint16_t, 1> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint8_t>(FILE *file,
        const image::Image<uint16_t, 2> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint8_t>(FILE *file,
        const image::Image<uint16_t, 3> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint8_t>(FILE *file,
        const image::Image<uint16_t, 4> &in,
        const std::vector<image::Attribute> &attrs);

    extern template int store<uint8_t>(FILE *file,
        const image::Image<float, 1> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint8_t>(FILE *file,
        const image::Image<float, 2> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint8_t>(FILE *file,
        const image::Image<float, 3> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint8_t>(FILE *file,
        const image::Image<float, 4> &in,
        const std::vector<image::Attribute> &attrs);

    extern template int store<uint16_t>(FILE *file,
        const image::Image<uint8_t, 1> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint16_t>(FILE *file,
        const image::Image<uint8_t, 2> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint16_t>(FILE *file,
        const image::Image<uint8_t, 3> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint16_t>(FILE *file,
        const image::Image<uint8_t, 4> &in,
        const std::vector<image::Attribute> &attrs);

    extern template int store<uint16_t>(FILE *file,
        const image::Image<uint16_t, 1> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint16_t>(FILE *file,
        const image::Image<uint16_t, 2> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint16_t>(FILE *file,
        const image::Image<uint16_t, 3> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint16_t>(FILE *file,
        const image::Image<uint16_t, 4> &in,
        const std::vector<image::Attribute> &attrs);

    extern template int store<uint16_t>(FILE *file,
        const image::Image<float, 1> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint16_t>(FILE *file,
        const image::Image<float, 2> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint16_t>(FILE *file,
        const image::Image<float, 3> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint16_t>(FILE *file,
        const image::Image<float, 4> &in,
        const std::vector<image::Attribute> &attrs);

    extern template int store<uint8_t>(std::string *memory,
        const image::Image<uint8_t, 1> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint8_t>(std::string *memory,
        const image::Image<uint8_t, 2> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint8_t>(std::string *memory,
        const image::Image<uint8_t, 3> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint8_t>(std::string *memory,
        const image::Image<uint8_t, 4> &in,
        const std::vector<image::Attribute> &attrs);

    extern template int store<uint8_t>(std::string *memory,
        const image::Image<uint16_t, 1> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint8_t>(std::string *memory,
        const image::Image<uint16_t, 2> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint8_t>(std::string *memory,
        const image::Image<uint16_t, 3> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint8_t>(std::string *memory,
        const image::Image<uint16_t, 4> &in,
        const std::vector<image::Attribute> &attrs);

    extern template int store<uint8_t>(std::string *memory,
        const image::Image<float, 1> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint8_t>(std::string *memory,
        const image::Image<float, 2> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint8_t>(std::string *memory,
        const image::Image<float, 3> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint8_t>(std::string *memory,
        const image::Image<float, 4> &in,
        const std::vector<image::Attribute> &attrs);

    extern template int store<uint16_t>(std::string *memory,
        const image::Image<uint8_t, 1> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint16_t>(std::string *memory,
        const image::Image<uint8_t, 2> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint16_t>(std::string *memory,
        const image::Image<uint8_t, 3> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint16_t>(std::string *memory,
        const image::Image<uint8_t, 4> &in,
        const std::vector<image::Attribute> &attrs);

    extern template int store<uint16_t>(std::string *memory,
        const image::Image<uint16_t, 1> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint16_t>(std::string *memory,
        const image::Image<uint16_t, 2> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint16_t>(std::string *memory,
        const image::Image<uint16_t, 3> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint16_t>(std::string *memory,
        const image::Image<uint16_t, 4> &in,
        const std::vector<image::Attribute> &attrs);

    extern template int store<uint16_t>(std::string *memory,
        const image::Image<float, 1> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint16_t>(std::string *memory,
        const image::Image<float, 2> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint16_t>(std::string *memory,
        const image::Image<float, 3> &in,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint16_t>(std::string *memory,
        const image::Image<float, 4> &in,
        const std::vector<image::Attribute> &attrs);

    extern template int store<uint8_t>(std::string *memory,
        const image::IImagePtr &in_ptr,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint16_t>(std::string *memory,
        const image::IImagePtr &in_ptr,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint8_t>(FILE *file,
        const image::IImagePtr &in_ptr,
        const std::vector<image::Attribute> &attrs);
    extern template int store<uint16_t>(FILE *file,
        const image::IImagePtr &in_ptr,
        const std::vector<image::Attribute> &attrs);

} } } // namespace rvg::image::pngio

#endif // RVG_PNGIO_H
