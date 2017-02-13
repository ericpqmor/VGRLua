#include <sstream>

#define BUFFERSIZE 1<<16
#include <b64/encode.h>
#include <b64/decode.h>

namespace rvg {
    namespace base64 {

std::string encode(const std::string &input) {
    std::istringstream sin(input);
    std::ostringstream sout;
    ::base64::encoder E;
    E.encode(sin, sout);
    return sout.str();
}

std::string decode(const std::string &input) {
    std::istringstream sin(input);
    std::ostringstream sout;
    ::base64::decoder E;
    E.decode(sin, sout);
    return sout.str();
}

} } // namespace rvg::base64
