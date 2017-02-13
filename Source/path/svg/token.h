#ifndef RVG_PATH_SVG_TOKEN_H
#define RVG_PATH_SVG_TOKEN_H

#include <iosfwd>
#include <type_traits>
#include <iterator>

#include "path/datum.h"

namespace rvg {
    namespace path {
        namespace svg {

struct Token {
    enum class Type { command, number, underflow, overflow, error, end };

    bool is_command(void) const { return type == Type::command; }
    bool is_number(void) const { return type == Type::number; }

    Token(Token::Type t, float f): type(t), value(f) { ; }

    Token(Token::Type t, int32_t i): type(t), value(i) { ; }

    Token(): type(Type::error), value(0) { ; }

    // automatic convertion from float to Token
    Token(float f): type(Type::number), value(f) { ; }

    Type type;

    Datum value;
};

std::ostream &operator<<(std::ostream &out, const Token &token);

template <typename IT>
using is_token_iterator = std::integral_constant<bool,
    std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<IT>::iterator_category>::value &&
    std::is_same<typename std::iterator_traits<IT>::value_type, Token>::value>;

} } } // namespace rvg::path::svg

#endif
