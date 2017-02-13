#ifndef RVG_DESCRIPTION_PAINTED_H
#define RVG_DESCRIPTION_PAINTED_H

#include <vector>

#include "shape/shape.h"
#include "paint/paint.h"
#include "xform/xform.h"
#include "xform/xformable.h"
#include "meta/meta.h"
#include "description/stencil.h"

namespace rvg {
    namespace description {

class Painted final: public xform::Xformable<Painted> {

    using Base = xform::Xformable<Painted>;
    using Xform = xform::Xform;
    using Shape = shape::Shape;
    using Paint = paint::Paint;
    using WindingRule = scene::WindingRule;

public:
    enum class Type {
        primitive,
        blurred,
        xformed,
        faded,
        clipped,
        empty
    };

    class Primitive {
    private:
        WindingRule m_winding_rule;
        Shape m_shape;
        Paint m_paint;
    public:
        template <typename S, typename P>
        Primitive(WindingRule winding_rule, S &&shape, P &&paint,
            typename std::enable_if<
                meta::forward_same_or_convertible<S, Shape>::value &&
                meta::forward_same_or_convertible<P, Paint>::value
            >::type * = nullptr):
            m_winding_rule(winding_rule),
            m_shape(std::forward<S>(shape)),
            m_paint(std::forward<P>(paint)) { ; }
        WindingRule winding_rule(void) const { return m_winding_rule; }
        const Shape &shape(void) const { return m_shape; }
        const Paint &paint(void) const { return m_paint; }
    };

    class Blurred {
        float m_radius;
        std::vector<Painted> m_painted;
    public:
        template <typename P>
        Blurred(float radius, P &&painted,
            typename std::enable_if<
                meta::forward_same_or_convertible<P,
                    std::vector<Painted>>::value
            >::type * = nullptr):
            m_radius(radius),
            m_painted(std::forward<P>(painted)) { ; }
        const std::vector<Painted> &painted(void) const { return m_painted; }
        const float &radius(void) const { return m_radius; }
    };

    class Xformed {
        std::vector<Painted> m_painted;
    public:
        template <typename P>
        Xformed(P &&painted,
            typename std::enable_if<
                meta::forward_same_or_convertible<P,
                    std::vector<Painted>>::value
            >::type * = nullptr):
            m_painted(std::forward<P>(painted)) { ; }
        const std::vector<Painted> &painted(void) const { return m_painted; }
    };

    class Faded{
        uint8_t m_opacity;
        std::vector<Painted> m_painted;
    public:
        template <typename P>
        Faded(uint8_t opacity, P &&painted,
            typename std::enable_if<
                meta::forward_same_or_convertible<P,
                    std::vector<Painted>>::value
            >::type * = nullptr):
            m_opacity(opacity),
            m_painted(std::forward<P>(painted)) { ; }
        const std::vector<Painted> &painted(void) const { return m_painted; }
        uint8_t opacity(void) const { return m_opacity; }
    };

    class Clipped{
        std::vector<Stencil> m_clipper;
        std::vector<Painted> m_clippee;
    public:
        template <typename R, typename P>
        Clipped(R &&clipper, P &&clippee,
            typename std::enable_if<
                meta::forward_same_or_convertible<R,
                    std::vector<Stencil>>::value &&
                meta::forward_same_or_convertible<P,
                    std::vector<Painted>>::value
            >::type * = nullptr):
            m_clipper(std::forward<R>(clipper)),
            m_clippee(std::forward<P>(clippee)) { ; }
        const std::vector<Stencil> &clipper(void) const { return m_clipper; }
        const std::vector<Painted> &clippee(void) const { return m_clippee; }
    };

private:

    Type m_type;

    union Union {
        Union() { ; }
        ~Union() { ; }
        Primitive primitive;
        Xformed xformed;
        Faded faded;
        Clipped clipped;
        Blurred blurred;
    } m_union;


public:

    Painted(): m_type(Type::empty) {
        memset(&m_union, 0, sizeof(m_union));
    }

    template <typename P>
    Painted(P &&primitive, typename std::enable_if<
        std::is_same<
            typename std::remove_reference<P>::type,
            Primitive
        >::value>::type * = nullptr): m_type(Type::primitive) {
        new (&m_union.primitive) Primitive{std::forward<P>(primitive)};
    }

    template <typename B>
    Painted(B &&b, typename std::enable_if<
        std::is_same<
            typename std::remove_reference<B>::type,
            Blurred
        >::value>::type * = nullptr): m_type(Type::blurred) {
        new (&m_union.blurred) Blurred{std::forward<B>(b)};
    }

    template <typename C>
    Painted(C &&c, typename std::enable_if<
        std::is_same<
            typename std::remove_reference<C>::type,
            Clipped
        >::value>::type * = nullptr): m_type(Type::clipped) {
        new (&m_union.clipped) Clipped{std::forward<C>(c)};
    }

    template <typename X>
    Painted(X &&t, typename std::enable_if<
        std::is_same<
            typename std::remove_reference<X>::type,
            Xformed
        >::value>::type * = nullptr): m_type(Type::xformed) {
        new (&m_union.xformed) Xformed{std::forward<X>(t)};
    }

    template <typename T>
    Painted(T &&t, typename std::enable_if<
        std::is_same<
            typename std::remove_reference<T>::type,
            Faded
        >::value>::type * = nullptr): m_type(Type::faded) {
        new (&m_union.faded) Faded{std::forward<T>(t)};
    }

    Painted(Painted &&other):
        Base(std::move(other)),
        m_type(std::move(other.m_type)) {
        switch (m_type) {
            case Type::primitive:
                new (&m_union.primitive)
                    Primitive(std::move(other.m_union.primitive));
                break;
            case Type::blurred:
                new (&m_union.blurred)
                    Blurred(std::move(other.m_union.blurred));
                break;
            case Type::xformed:
                new (&m_union.xformed)
                    Xformed(std::move(other.m_union.xformed));
                break;
            case Type::faded:
                new (&m_union.faded)
                    Faded(std::move(other.m_union.faded));
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

    Painted(const Painted &other):
        Base(other),
        m_type(other.m_type) {
        switch (m_type) {
            case Type::primitive:
                new (&m_union.primitive) Primitive(other.m_union.primitive);
                break;
            case Type::blurred:
                new (&m_union.blurred) Blurred(other.m_union.blurred);
                break;
            case Type::xformed:
                new (&m_union.xformed) Xformed(other.m_union.xformed);
                break;
            case Type::faded:
                new (&m_union.faded) Faded(other.m_union.faded);
                break;
            case Type::clipped:
                new (&m_union.clipped) Clipped(other.m_union.clipped);
                break;
            case Type::empty:
                memset(&m_union, 0, sizeof(m_union));
                break;
        }
    }

    const char *type_name(void) {
        switch (m_type) {
            case Type::primitive: return "primitive";
            case Type::blurred: return "blurred";
            case Type::xformed: return "xformed";
            case Type::faded: return "faded";
            case Type::clipped: return "clipped";
            case Type::empty: return "empty";
            default: return "(unitialized?)";
        }
    }

    ~Painted() {
        switch (m_type) {
            case Type::primitive:
                m_union.primitive.~Primitive();
                break;
            case Type::xformed:
                m_union.xformed.~Xformed();
                break;
            case Type::faded:
                m_union.faded.~Faded();
                break;
            case Type::clipped:
                m_union.clipped.~Clipped();
                break;
            case Type::blurred:
                m_union.blurred.~Blurred();
                break;
            case Type::empty:
                break;
        }
    }

    Painted &operator=(Painted &&other) {
        // ??D we should optimize for case where other.m_type == m_type
        // in this case we could avoid destruction and construction
        // and invoke the move assignment directly in the m_union field
        this->~Painted();
        new (this) Painted(std::move(other));
        return *this;
    }

    Painted &operator=(const Painted &other) {
        // ??D we should optimize for case where other.m_type == m_type
        // in this case we could avoid destruction and construction
        // and invoke the copy assignment directly in the m_union field
        this->~Painted();
        new (this) Painted(other);
        return *this;
    }

    const Type &type(void) const
    { return m_type; }

    const Primitive &primitive(void) const
    { return m_union.primitive; }

    const Xformed &xformed(void) const
    { return m_union.xformed; }

    const Blurred &blurred(void) const
    { return m_union.blurred; }

    const Faded &faded(void) const
    { return m_union.faded; }

    const Clipped &clipped(void) const
    { return m_union.clipped; }
};

template <typename P>
Painted make_painted_blurred(float radius, P &&painted,
    typename std::enable_if<
        std::is_same<
            typename std::remove_reference<P>::type::value_type,
            Painted
        >::value
    >::type * = nullptr) {
    return Painted(Painted::Blurred{radius, std::forward<P>(painted)});
}

template <typename X, typename P>
Painted make_painted_xformed(X &xf, P &&painted,
    typename std::enable_if<
        meta::forward_same_or_convertible<X, xform::Xform>::value &&
        meta::forward_same_or_convertible<P, std::vector<Painted>>::value
    >::type * = nullptr) {
    return Painted(Painted::Xformed{std::forward<P>(painted)}).transformed(xf);
}

template <typename P>
Painted make_painted_faded(uint8_t opacity, P &&painted,
    typename std::enable_if<
        meta::forward_same_or_convertible<P, std::vector<Painted>>::value
    >::type * = nullptr) {
    return Painted(Painted::Faded{opacity, std::forward<P>(painted)});
}

template <typename S, typename P>
Painted make_painted_primitive(scene::WindingRule winding_rule, S &&shape,
    P &&paint, typename std::enable_if<
        meta::forward_same_or_convertible<S, shape::Shape>::value &&
        meta::forward_same_or_convertible<P, paint::Paint>::value
    >::type * = nullptr) {
    return Painted(Painted::Primitive(winding_rule, std::forward<S>(shape),
        std::forward<P>(paint)));
}

template <typename R, typename P>
Painted make_painted_clipped(R &&clipper, P &&clippee,
    typename std::enable_if<
        meta::forward_same_or_convertible<R, std::vector<Stencil>>::value &&
        meta::forward_same_or_convertible<P, std::vector<Painted>>::value
    >::type * = nullptr) {
    return Painted(Painted::Clipped(std::forward<R>(clipper),
        std::forward<P>(clippee)));
}

} } // namespace rvg::scene::description

#endif
