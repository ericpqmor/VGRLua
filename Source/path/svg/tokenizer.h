#ifndef RVG_PATH_SVG_TOKENIZER_H
#define RVG_PATH_SVG_TOKENIZER_H

#include <boost/iterator/iterator_facade.hpp>
#include <cctype>

#include "path/svg/token.h"

namespace rvg {
    namespace path {
        namespace svg {

// Given a string, iterates over all tokens in it
class Tokenizer final: public boost::iterator_facade<
    Tokenizer, Token const,
    boost::forward_traversal_tag> {

public:
    explicit Tokenizer(const char *curr): m_curr(curr) { next(); }

    Tokenizer(void): m_curr(nullptr) { set(Token::Type::end, 0); }

    Tokenizer(const Tokenizer &o): m_curr(o.m_curr), m_tok(o.m_tok) { ; }

    ptrdiff_t operator-(const Tokenizer &o) { return m_curr - o.m_curr; }

private:
    friend class boost::iterator_core_access;

    void increment(void) { next(); }

    bool equal(const Tokenizer &other) const { return m_curr == other.m_curr; }

    Token const &dereference(void) const { return m_tok; }

    void set(Token::Type t, Datum v) { m_tok.type = t; m_tok.value = v; }

    void next(void) {
        // trying to move past end of string?
        if (!m_curr)
            return set(Token::Type::error, 0);

        // skip optional spaces
        while (isspace(*m_curr))
            ++m_curr;

        // skip one optional comma
        if (*m_curr== ',') {
            ++m_curr;
            // skip optional spaces following optional comma
            while (isspace(*m_curr))
                ++m_curr;
        }

        // reached end of string
        if (!(*m_curr)) {
            m_curr = nullptr;
            return set(Token::Type::end, 0);
        }

        // try command
        if (isalpha(*m_curr))
            return set(Token::Type::command, *m_curr++);

        // try number
        char *end;
        float value = strtof(m_curr, &end);
        // error parsing number
        if (end == m_curr) {
            // advance 1 and error
            ++m_curr;
            return set(Token::Type::error, 0);
        }
        m_curr = end;
        // overflow or underflow
        if (errno == ERANGE) {
            if (value == 0.f)
                return set(Token::Type::underflow, 0);
            else
                return set(Token::Type::overflow, value);
        }
        // number is ok
        return set(Token::Type::number, value);
    }

    const char *m_curr;
    Token m_tok;
};

} } } // namespace rvg::path::svg

#endif
