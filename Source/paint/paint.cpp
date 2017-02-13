#include <iostream>

#include "paint/spread.h"

namespace rvg {
    namespace paint {

std::ostream &operator<<(std::ostream &out, const Spread spread) {
    switch (spread) {
        case Spread::pad: out << "pad"; break;
        case Spread::repeat: out << "repeat"; break;
        case Spread::reflect: out << "reflect"; break;
        case Spread::transparent: out << "transparent"; break;
    }
    return out;
}

} } // namespace rvg::paint
