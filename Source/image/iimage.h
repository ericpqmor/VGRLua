#ifndef RVG_IMAGE_IIMAGE_H
#define RVG_IMAGE_IIMAGE_H

#include <memory>

namespace rvg {
    namespace image {

// types of channel
enum class ChannelType {
    channel_float,
    channel_uint8_t,
    channel_uint16_t,
    channel_unknown
};

// ??D it would be useful to have gamma information as well
// for now, we assume all images use sRGB
enum class Gamma {
    sRGB,
    linear,
    unknown
};

// ??D still need to implement packed instead of planar layout
// also, implement a row pitch. One issue to sort out is
// that the pitch for the packed configuration requires a different
// amount of padding than the pitch for the planar configuration.
// maybe the solution is to simply not support pitch?
enum class Format {
    packed,
    planar
};

class IImage {
public:

    virtual ~IImage() { ; }

    int width(void) const {
        return do_width();
    }

    int height(void) const {
        return do_height();
    }

    int channels(void) const {
        return do_channels();
    }

    void resize(int width, int height) {
        return do_resize(width, height);
    }

    void set_float(int x, int y, int c, float v) {
        return do_set_float(x, y, c, v);
    }

    float get_float(int x, int y, int c) const {
        return do_get_float(x, y, c);
    }

    ChannelType channel_type(void) const {
        return do_channel_type();
    }

    void set_gamma(Gamma g) {
        return do_set_gamma(g);
    }

    Gamma gamma(void) const {
        return do_gamma();
    }

protected:
    virtual int do_width(void) const = 0;
    virtual int do_height(void) const = 0;
    virtual int do_channels(void) const = 0;
    virtual ChannelType do_channel_type(void) const = 0;
    virtual void do_set_float(int x, int y, int c, float v) = 0;
    virtual float do_get_float(int x, int y, int c) const = 0;
    virtual Gamma do_gamma(void) const = 0;
    virtual void do_set_gamma(Gamma g) = 0;
    virtual void do_resize(int width, int height) = 0;
};

using IImagePtr = std::shared_ptr<IImage>;

} } // namespace rvg::image


#endif

