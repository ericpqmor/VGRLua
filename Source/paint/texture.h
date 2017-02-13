#ifndef RVG_PAINT_TEXTURE_H
#define RVG_PAINT_TEXTURE_H

#include "paint/spread.h"
#include "image/iimage.h"

namespace rvg {
    namespace paint {

class Texture {
private:
    Spread m_spread;
    image::IImagePtr m_image_ptr;
public:
    Texture(Spread spread, const image::IImagePtr &image_ptr):
        m_spread(spread), m_image_ptr(image_ptr) { }
    const image::IImagePtr image_ptr(void) const { return m_image_ptr; }
    const image::IImage &image(void) const { return *m_image_ptr; }
    Spread spread(void) const { return m_spread; }
};

using TexturePtr = std::shared_ptr<Texture>;

} } // namespace rvg::paint

#endif
