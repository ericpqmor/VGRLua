#ifndef RVG_BASE64_H
#define RVG_BASE64_H

#include <string>

namespace rvg {
    namespace base64 {

std::string encode(const std::string &input);

std::string decode(const std::string &input);

} } // namespace rvg::base64

#endif
