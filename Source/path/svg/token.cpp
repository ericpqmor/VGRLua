#include <iostream>

#include "path/svg/token.h"

namespace rvg {
    namespace path {
        namespace svg {

std::ostream &operator<<(std::ostream &out, const Token &token) {
    out << "Token{";
    switch (token.type) {
        case Token::Type::command:
            out << "command, '" << static_cast<char>(token.value.i) << '\'';
            break;
        case Token::Type::number:
            out << "number, " << token.value.f;
            break;
        case Token::Type::underflow: out << "underflow, " << token.value.i;
            break;
        case Token::Type::overflow: out << "overflow, " << token.value.i;
            break;
        case Token::Type::error: out << "error, " << token.value.i;
            break;
        case Token::Type::end: out << "end";
            break;
    }
    out << '}';
    return out;
}

} } } // namespace rvg::path::svg
