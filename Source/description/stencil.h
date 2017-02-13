#ifndef RVG_DESCRIPTION_REGION_H
#define RVG_DESCRIPTION_REGION_H

#include <vector>
#include "xform/xform.h"
#include "xform/xformable.h"
#include "shape/shape.h"
#include "meta/meta.h"
#include "scene/scene.h"

namespace rvg {
    namespace description {

class Stencil final: public rvg::xform::Xformable<Stencil> {

    using Base = rvg::xform::Xformable<Stencil>;
    using Xform = rvg::xform::Xform;
    using Shape = rvg::shape::Shape;
    using WindingRule = rvg::scene::WindingRule;

public:

    enum class Type {
        primitive,
        xformed,
        clipped,
        empty,
    };

    class Primitive {
        WindingRule m_winding_rule;
        Shape m_shape;
    public:
        template <typename S>
        Primitive(WindingRule winding_rule, S &&shape,
            typename std::enable_if<
                meta::forward_same_or_convertible<S, Shape>::value
            >::type * = nullptr):
            m_winding_rule(winding_rule),
            m_shape(std::forward<S>(shape)) { ; }
        const Shape &shape(void) const { return m_shape; }
        WindingRule winding_rule(void) const { return m_winding_rule; }
    };

    class Xformed {
        std::vector<Stencil> m_stencil;
    public:
        template <typename S>
        Xformed(S &&stencil,
            typename std::enable_if<
                meta::forward_same_or_convertible<S,
                    std::vector<Stencil>>::value
            >::type * = nullptr):
            m_stencil(std::forward<S>(stencil)) { ; }
        const std::vector<Stencil> &stencil(void) const { return m_stencil; }
    };

    class Clipped {
        std::vector<Stencil> m_clipper;
        std::vector<Stencil> m_clippee;
    public:
        template <typename ER, typename EE>
        Clipped(ER &&clipper, EE &&clippee,
            typename std::enable_if<
                meta::forward_same_or_convertible<ER,
                    std::vector<Stencil>>::value &&
                meta::forward_same_or_convertible<EE,
                    std::vector<Stencil>>::value
            >::type * = nullptr):
            m_clipper(std::forward<ER>(clipper)),
            m_clippee(std::forward<EE>(clippee)) { ; }
        const std::vector<Stencil> &clipper(void) const { return m_clipper; }
        const std::vector<Stencil> &clippee(void) const { return m_clippee; }
    };

private:

    Type m_type;

    union Union {
        Union() { ; }
        ~Union() { ; }
        Primitive primitive;
        Xformed xformed;
        Clipped clipped;
    } m_union;


public:

    Stencil(void): m_type(Type::empty) {
        memset(&m_union, 0, sizeof(m_union));
    }

    template <typename P>
    Stencil(P &&primitive, typename std::enable_if<
        std::is_same<
            typename std::remove_reference<P>::type,
            Primitive
        >::value>::type * = nullptr): m_type(Type::primitive) {
        new (&m_union.primitive) Primitive{std::forward<P>(primitive)};
    }

    template <typename C>
    Stencil(C &&c, typename std::enable_if<
        std::is_same<
            typename std::remove_reference<C>::type,
            Clipped
        >::value>::type * = nullptr): m_type(Type::clipped) {
        new (&m_union.clipped) Clipped{std::forward<C>(c)};
    }

    template <typename X>
    Stencil(X &&t, typename std::enable_if<
        std::is_same<
            typename std::remove_reference<X>::type,
            Xformed
        >::value>::type * = nullptr): m_type(Type::xformed) {
        new (&m_union.xformed) Xformed{std::forward<X>(t)};
    }

    Stencil(Stencil &&other):
        Base(std::move(other)),
        m_type(std::move(other.m_type)) {
        switch (m_type) {
            case Type::primitive:
                new (&m_union.primitive)
                    Primitive(std::move(other.m_union.primitive));
                break;
            case Type::xformed:
                new (&m_union.xformed)
                    Xformed(std::move(other.m_union.xformed));
                break;
            case Type::clipped:
                new (&m_union.clipped)
                    Clipped(std::move(other.m_union.clipped));
                break;
            case Type::empty:
                memset(&m_union, 0, sizeof(m_union));
                break;
        }
    }

    Stencil(const Stencil &other):
        Base(other),
        m_type(other.m_type) {
        switch (m_type) {
            case Type::primitive:
                new (&m_union.primitive) Primitive(other.m_union.primitive);
                break;
            case Type::xformed:
                new (&m_union.xformed) Xformed(other.m_union.xformed);
                break;
            case Type::clipped:
                new (&m_union.clipped) Clipped(other.m_union.clipped);
                break;
            case Type::empty:
                memset(&m_union, 0, sizeof(m_union));
                break;
        }
    }

    ~Stencil() {
        switch (m_type) {
            case Type::primitive:
                m_union.primitive.~Primitive();
                break;
            case Type::xformed:
                m_union.xformed.~Xformed();
                break;
            case Type::clipped:
                m_union.clipped.~Clipped();
                break;
            case Type::empty:
                break;
        }
    }

    Stencil &operator=(Stencil &&other) {
        // ??D we should optimize for case where other.m_type == m_type
        // in this case we could avoid destruction and construction
        // and invoke the move assignment directly in the m_union field
        this->~Stencil();
        new (this) Stencil(std::move(other));
        return *this;
    }

    Stencil &operator=(const Stencil &other) {
        // ??D we should optimize for case where other.m_type == m_type
        // in this case we could avoid destruction and construction
        // and invoke the copy assignment directly in the m_union field
        this->~Stencil();
        new (this) Stencil(other);
        return *this;
    }

    const Type &type(void) const
    { return m_type; }

    const Primitive &primitive(void) const
    { return m_union.primitive; }

    const Xformed &xformed(void) const
    { return m_union.xformed; }

    const Clipped &clipped(void) const
    { return m_union.clipped; }

};

template <typename S>
Stencil make_stencil_primitive(scene::WindingRule rule, S &&shape,
    typename std::enable_if<
        meta::forward_same_or_convertible<S, shape::Shape>::value
    >::type * = nullptr) {
    return Stencil(Stencil::Primitive(rule, std::forward<S>(shape)));
}

template <typename R, typename P>
Stencil make_stencil_clipped(R &&clipper, P &&clippee,
    typename std::enable_if<
        meta::forward_same_or_convertible<R, std::vector<Stencil>>::value &&
        meta::forward_same_or_convertible<P, std::vector<Stencil>>::value
    >::type * = nullptr) {
    return Stencil(Stencil::Clipped(std::forward<R>(clipper),
        std::forward<P>(clippee)));
}

template <typename X, typename S>
Stencil make_stencil_xformed(X &xf, S &&stencil,
    typename std::enable_if<
        meta::forward_same_or_convertible<X, xform::Xform>::value &&
        meta::forward_same_or_convertible<S, std::vector<Stencil>>::value
    >::type * = nullptr) {
    return Stencil(Stencil::Xformed{std::forward<S>(stencil)}).transformed(xf);
}

} } // namespace rvg::scene::description

#endif
