#include <boost/iterator/iterator_facade.hpp>

#include "compat/compat.h"
#include "meta/meta.h"
#include "color/color.h"
#include "paint/paint.h"
#include "description/painted.h"
#include "description/stencil.h"
#include "shape/triangle.h"
#include "shape/rect.h"
#include "shape/circle.h"
#include "shape/polygon.h"
#include "image/image.h"
#include "stroke/style.h"
#include "path/svg/parse.h"
#include "path/path.h"
#include "scene/xformablescene.h"
#include "xform/xform.h"

#include "description/lua.h"

using rvg::color::RGBA8;
using rvg::image::IImagePtr;
using rvg::scene::XformableScene;
using rvg::paint::LinearGradient;
using rvg::paint::RadialGradient;
using rvg::paint::Texture;
using rvg::paint::RampPtr;
using rvg::paint::Ramp;
using rvg::paint::Stop;
using rvg::stroke::Style;
using rvg::paint::Paint;
using rvg::paint::Spread;
using rvg::stroke::Cap;
using rvg::stroke::Join;
using rvg::stroke::Method;
using rvg::description::Painted;
using rvg::description::Stencil;
using rvg::scene::WindingRule;
using rvg::shape::Shape;
using rvg::shape::Triangle;
using rvg::shape::Circle;
using rvg::shape::Rect;
using rvg::shape::Polygon;
using rvg::path::Path;
using rvg::path::svg::Token;
using rvg::xform::Xform;
using rvg::bbox::Window;
using rvg::bbox::Viewport;

static float checkfloat(lua_State *L, int idx) {
	return static_cast<float>(luaL_checknumber(L, idx));
}

static float optfloat(lua_State *L, int idx, float def) {
	return static_cast<float>(luaL_optnumber(L, idx, def));
}

static int optboolean(lua_State *L, int idx, int def) {
	if (lua_isnoneornil(L, idx)) {
		return def;
	} else if (!lua_isboolean(L, idx)) {
		luaL_argerror(L, idx, "expected boolean");
		return 0; // never reached
	} else {
		return lua_toboolean(L, idx);
	}
}

static float tofloat(lua_State *L, int idx) {
	return static_cast<float>(lua_tonumber(L, idx));
}

template <typename T> int type_metatableindex(void);

template <> int type_metatableindex<XformableScene>(void) { return 1; }
template <> int type_metatableindex<Window>(void)         { return 2; }
template <> int type_metatableindex<Viewport>(void)       { return 3; }
template <> int type_metatableindex<RGBA8>(void)          { return 4; }
template <> int type_metatableindex<Spread>(void)         { return 5; }
template <> int type_metatableindex<RampPtr>(void)        { return 6; }
template <> int type_metatableindex<Paint>(void)          { return 7; }
template <> int type_metatableindex<Painted>(void)        { return 8; }
template <> int type_metatableindex<Stencil>(void)        { return 9; }
template <> int type_metatableindex<Shape>(void)          { return 10; }
template <> int type_metatableindex<Xform>(void)          { return 11; }
template <> int type_metatableindex<Cap>(void)            { return 12; }
template <> int type_metatableindex<Join>(void)           { return 13; }
template <> int type_metatableindex<Method>(void)         { return 14; }
template <> int type_metatableindex<Style>(void)          { return 15; }

template <typename T> const char *type_name(void);

template <> const char *type_name<XformableScene>(void) { return "scene"; }
template <> const char *type_name<Viewport>(void)       { return "viewport"; }
template <> const char *type_name<Window>(void)         { return "window"; }
template <> const char *type_name<RGBA8>(void)          { return "color"; }
template <> const char *type_name<Spread>(void)         { return "spread"; }
template <> const char *type_name<RampPtr>(void)        { return "ramp"; }
template <> const char *type_name<Paint>(void)          { return "paint"; }
template <> const char *type_name<Painted>(void)        { return "painted"; }
template <> const char *type_name<Stencil>(void)        { return "stencil"; }
template <> const char *type_name<Shape>(void)          { return "shape"; }
template <> const char *type_name<Xform>(void)          { return "xform"; }
template <> const char *type_name<Cap>(void)            { return "cap"; }
template <> const char *type_name<Join>(void)           { return "join"; }
template <> const char *type_name<Method>(void)         { return "method"; }
template <> const char *type_name<Style>(void)          { return "style"; }

template <typename T> const char *type_argerror(void);

template <> const char *type_argerror<XformableScene>(void) {
    return "expected scene"; }
template <> const char *type_argerror<Viewport>(void) {
    return "expected viewport"; }
template <> const char *type_argerror<Window>(void) {
    return "expected window"; }
template <> const char *type_argerror<RGBA8>(void) {
    return "expected color"; }
template <> const char *type_argerror<Spread>(void) {
    return "expected spread"; }
template <> const char *type_argerror<RampPtr>(void) {
    return "expected ramp"; }
template <> const char *type_argerror<Paint>(void) {
    return "expected paint"; }
template <> const char *type_argerror<Painted>(void) {
    return "expected painted"; }
template <> const char *type_argerror<Stencil>(void) {
    return "expected stencil"; }
template <> const char *type_argerror<Shape>(void) {
    return "expected shape"; }
template <> const char *type_argerror<Xform>(void) {
    return "expected xform"; }
template <> const char *type_argerror<Cap>(void) {
    return "expected stroke cap style"; }
template <> const char *type_argerror<Join>(void) {
    return "expected stroke join style"; }
template <> const char *type_argerror<Method>(void) {
    return "expected stroke method"; }
template <> const char *type_argerror<Style>(void) {
    return "expected stroke style"; }

template <typename T>
void type_pushmetatable(lua_State *L, int metidx = lua_upvalueindex(1)) {
    lua_rawgeti(L, metidx, type_metatableindex<T>());
}

template <typename T>
int type_is(lua_State *L, int idx, int metidx = lua_upvalueindex(1)) {
    idx = compat_abs_index(L, idx);
    type_pushmetatable<T>(L, metidx);
    if (!lua_getmetatable(L, idx)) lua_pushnil(L);
    int ret = compat_is_equal(L, -1, -2);
    lua_pop(L, 2);
    return ret;
}

template <typename T>
T type_to(lua_State *L, int idx) {
    return *reinterpret_cast<T *>(lua_touserdata(L, idx));
}

template <typename T>
T *type_topointer(lua_State *L, int idx) {
    return reinterpret_cast<T *>(lua_touserdata(L, idx));
}

template <typename T>
int type_gc(lua_State *L) {
    T *ptr = reinterpret_cast<T *>(lua_touserdata(L, 1));
    ptr->~T();
    return 0;
}

template <typename T>
int type_tostring(lua_State *L) {
    T *ptr = reinterpret_cast<T *>(lua_touserdata(L, 1));
    lua_pushfstring(L, "%s: %p", type_name<T>(), ptr);
    return 1;
}

template <>
int type_tostring<Xform>(lua_State *L) {
    const Xform &xf = *reinterpret_cast<Xform *>(lua_touserdata(L, 1));
    lua_pushfstring(L, "xform{%f,%f,%f,%f,%f,%f}", xf[0][0], xf[0][1], xf[0][2],
       xf[1][0], xf[1][1], xf[1][2]);
    return 1;
}

template <typename T>
T type_check_pointer(lua_State *L, int idx, int metidx = lua_upvalueindex(1)) {
    if (!type_is<T>(L, idx, metidx)) luaL_argerror(L, idx, type_argerror<T>());
    return type_topointer<T>(L, idx);
}

template <typename T>
T type_check(lua_State *L, int idx, int metidx = lua_upvalueindex(1)) {
    if (!type_is<T>(L, idx, metidx)) luaL_argerror(L, idx, type_argerror<T>());
    return type_to<T>(L, idx);
}

template <>
int type_check<int>(lua_State *L, int idx, int) {
    return static_cast<int>(luaL_checkinteger(L, idx));
}

template <>
float type_check<float>(lua_State *L, int idx, int) {
    return checkfloat(L, idx);
}

template <typename T>
void type_setmetatable(lua_State *L, int objidx,
	int metidx = lua_upvalueindex(1)) {
    objidx = compat_abs_index(L, objidx);
    lua_rawgeti(L, metidx, type_metatableindex<T>());
    lua_setmetatable(L, objidx);
}

template <typename T, typename ...As, size_t ...Is>
int type_new_helper(lua_State *L, rvg::meta::sequence<Is...>) {
    int n = static_cast<int>(sizeof...(As));
    int t = lua_gettop(L);
    if (t < n) luaL_error(L, "not enough arguments (needed %d)", n);
    if (t > n) luaL_error(L, "too many arguments (needed %d)", n);
    T* p = reinterpret_cast<T*>(lua_newuserdata(L, sizeof(T)));
    new (p) T(type_check<As>(L, Is+1)...);
    type_setmetatable<T>(L, -1);
    return 1;
}

template <typename T, typename ...As>
int type_new(lua_State *L) {
    return type_new_helper<T, As...>(L,
        rvg::meta::make_sequence<sizeof...(As)>{});
}

template <typename T>
int type_push(lua_State *L, const T &value, int metidx = lua_upvalueindex(1)) {
    T* ptr = reinterpret_cast<T*>(lua_newuserdata(L, sizeof(T)));
    new (ptr) T(value);
    type_setmetatable<T>(L, -1, metidx);
	return 1;
}

template <>
int type_push<float>(lua_State *L, const float &value, int) {
	lua_pushnumber(L, value);
	return 1;
}

template <>
int type_push<int>(lua_State *L, const int &value, int) {
	lua_pushinteger(L, value);
	return 1;
}

template <>
int type_push<const char *>(lua_State *L, const char * const &value, int) {
	lua_pushstring(L, value);
	return 1;
}

template <typename T>
T load_table_item(lua_State *L, int table_index, int item_index);

template <>
Stop load_table_item(lua_State *L, int table_index, int item_index) {
    lua_rawgeti(L, table_index, item_index);
    if (lua_type(L, -1) != LUA_TTABLE)
        luaL_error(L, "index %d not a stop", item_index);
    lua_rawgeti(L, -1, 1);
    if (lua_type(L, -1) != LUA_TNUMBER)
        luaL_error(L, "invalid offset at stop %d", item_index);
    lua_rawgeti(L, -2, 2);
    if (!type_is<RGBA8>(L, -1))
        luaL_error(L, "invalid color at stop %d", item_index);
    Stop s{tofloat(L, -2), type_to<RGBA8>(L, -1)};
    lua_pop(L, 3);
    return s;
}

template <>
Stencil load_table_item(lua_State *L, int table_index, int item_index) {
    lua_rawgeti(L, table_index, item_index);
    if (!type_is<Stencil>(L, -1))
        luaL_error(L, "entry %d is not a stencil", item_index);
    Stencil s = type_to<Stencil>(L, -1);
    lua_pop(L, 1);
    return s;
}

template <>
Painted load_table_item(lua_State *L, int table_index, int item_index) {
    lua_rawgeti(L, table_index, item_index);
    if (!type_is<Painted>(L, -1))
        luaL_error(L, "entry %d is not painted", item_index);
    Painted p = type_to<Painted>(L, -1);
    lua_pop(L, 1);
    return p;
}

template <>
float load_table_item(lua_State *L, int table_index, int item_index) {
    lua_rawgeti(L, table_index, item_index);
    if (!lua_isnumber(L, -1))
        luaL_error(L, "entry %d is not a number", item_index);
    float f = tofloat(L, -1);
    lua_pop(L, 1);
    return f;
}

template <>
Token load_table_item(lua_State *L, int table_index, int item_index) {
    lua_rawgeti(L, table_index, item_index);
    if (lua_isnumber(L, -1)) {
		float f = tofloat(L, -1);
		lua_pop(L, 1);
		return Token(Token::Type::number, f);
	} else if (lua_isstring(L, -1)) {
		int i = *lua_tostring(L, -1);
		lua_pop(L, 1);
		return Token(Token::Type::command, i);
	} else {
		luaL_error(L, "entry %d is not a number or SVG command", item_index);
		return Token();
	}
}

template <>
std::string load_table_item(lua_State *L, int table_index, int item_index) {
    lua_rawgeti(L, table_index, item_index);
    if (!lua_isstring(L, -1))
        luaL_error(L, "entry %d is not a string", item_index);
    std::string str(lua_tostring(L, -1));
    lua_pop(L, 1);
    return str;
}

template <typename T>
class TableIterator: public boost::iterator_facade<
    TableIterator<T>, T const, boost::forward_traversal_tag, T> {
    lua_State *m_L;     // lua context where table lives
    int m_table_index;  // stack position of table in context
    int m_item_index;   // current item in table
    int m_nitems;       // number of items in table
public:
    TableIterator(lua_State *L, int table_index, int item_index, int nitems):
        m_L(L), m_table_index(compat_abs_index(L, table_index)),
        m_item_index(item_index), m_nitems(nitems) { ; }
private:
    friend class boost::iterator_core_access;

    void increment(void) {
        ++m_item_index;
    }

    bool equal(const TableIterator<T> &other) const {
        // assume the user is not crazy to compare iterators
        // pointing to different tables
        return m_item_index == other.m_item_index;
    }

    T dereference(void) const {
        if (m_item_index <= m_nitems) {
            return load_table_item<T>(m_L, m_table_index, m_item_index);
		} else {
            luaL_error(m_L, "iterator accessed out of bounds (%d of %d)",
                m_item_index, m_nitems);
            return T(); // never reached
        }
    }
};

void type_print(lua_State *L, int idx) {
    if (luaL_getmetafield(L, idx, "__name") == LUA_TSTRING) {
        const char *name = lua_tostring(L, -1);
        fprintf(stderr, "type at %d is %s\n", idx, name);
        lua_pop(L, 1);
    } else {
        fprintf(stderr, "type at %d is %s\n", idx, luaL_typename(L, idx));
    }
}

static Xform xf_newrotation(lua_State *L, int base) {
    using namespace rvg::xform;
    base = base-1;
	switch (lua_gettop(L)-base) {
		case 1:
			return rotation(checkfloat(L, 1+base));
		case 3:
			return rotation(checkfloat(L, 1+base), checkfloat(L, 2+base),
                checkfloat(L, 3+base));
		default:
			luaL_error(L, "invalid number of arguments");
            return identity();
	}
}

static Xform xf_newscaling(lua_State *L, int base) {
    using namespace rvg::xform;
    base = base-1;
	switch (lua_gettop(L)-base) {
		case 1:
			return scaling(checkfloat(L, 1+base));
		case 2:
			return scaling(checkfloat(L, 1+base), checkfloat(L, 2+base));
		case 3:
			return scaling(checkfloat(L, 1+base), checkfloat(L, 2+base),
                checkfloat(L, 3+base));
		case 4:
			return scaling(checkfloat(L, 1+base), checkfloat(L, 2+base),
                checkfloat(L, 3+base), checkfloat(L, 4+base));
		default:
			luaL_error(L, "invalid number of arguments");
            return identity();
	}
}

static Xform xf_newtranslation(lua_State *L, int base) {
    using namespace rvg::xform;
    base = base-1;
    Xform xf = translation(checkfloat(L, 1+base), optfloat(L, 2+base, 0.f));
    if (lua_gettop(L)-base > 2) luaL_error(L, "invalid number of arguments");
    return xf;
}

static Xform xf_newlinear(lua_State *L, int base) {
    using namespace rvg::xform;
    base = base-1;
    Xform xf = linear(checkfloat(L, 1+base), checkfloat(L, 2+base),
        checkfloat(L, 3+base), checkfloat(L, 4+base));
    if (lua_gettop(L)-base != 4)
		luaL_error(L, "invalid number of arguments");
    return xf;
}

static Xform xf_newaffinity(lua_State *L, int base) {
    using namespace rvg::xform;
    base = base-1;
    Xform xf = affinity(checkfloat(L, 1+base), checkfloat(L, 2+base),
        checkfloat(L, 3+base), checkfloat(L, 4+base),
        checkfloat(L, 5+base), checkfloat(L, 6+base));
    if (lua_gettop(L)-base != 6)
		luaL_error(L, "invalid number of arguments");
    return xf;
}

static Xform xf_newwindowviewport(lua_State *L, int base) {
    using namespace rvg::xform;
    base = base-1;
    Xform xf = windowviewport(type_check<Window>(L, 1+base),
        type_check<Viewport>(L, 2+base));
    if (lua_gettop(L)-base != 2)
		luaL_error(L, "invalid number of arguments");
    return xf;
}

template <typename T> int type_methodtranslated(lua_State *L) {
    type_push<T>(L, type_check<T>(L, 1).transformed(xf_newtranslation(L, 2)));
    return 1;
}

template <typename T> int type_methodrotated(lua_State *L) {
    type_push<T>(L, type_check<T>(L, 1).transformed(xf_newrotation(L, 2)));
    return 1;
}

template <typename T> int type_methodscaled(lua_State *L) {
    type_push<T>(L, type_check<T>(L, 1).transformed(xf_newscaling(L, 2)));
    return 1;
}

template <typename T> int type_methodlinear(lua_State *L) {
    type_push<T>(L, type_check<T>(L, 1).transformed(xf_newlinear(L, 2)));
    return 1;
}

template <typename T> int type_methodaffine(lua_State *L) {
    type_push<T>(L, type_check<T>(L, 1).transformed(xf_newaffinity(L, 2)));
    return 1;
}

template <typename T> int type_methodwindowviewport(lua_State *L) {
    type_push<T>(L, type_check<T>(L, 1).transformed(xf_newwindowviewport(L, 2)));
    return 1;
}

template <typename T> int type_methodtransformed(lua_State *L) {
    type_push<T>(L, type_check<T>(L, 1).transformed(type_check<Xform>(L, 2)));
    return 1;
}

template <typename T>
void type_setxformable(lua_State *L, int metidx) {
    metidx = compat_abs_index(L, metidx);
    luaL_Reg xf_methods[] = {
        { "translated", &type_methodtranslated<T> },
        { "rotated", &type_methodrotated<T> },
        { "scaled", &type_methodscaled<T> },
        { "linear", &type_methodlinear<T> },
        { "affine", &type_methodaffine<T> },
        { "windowviewport", &type_methodwindowviewport<T> },
        { "transformed", &type_methodtransformed<T> },
        { nullptr, nullptr }
    };
    type_pushmetatable<T>(L, metidx); // meta
    lua_getfield(L, -1, "__index");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
    } // meta index
    lua_pushvalue(L, metidx); // meta index metidx
    compat_setfuncs(L, xf_methods, 1); // meta index
    lua_setfield(L, -2, "__index"); // meta
    lua_pop(L, 1);
}

template <typename T> int type_methodstroked(lua_State *L) {
    if (lua_isnumber(L, 2)) {
        type_push<T>(L, type_check<T>(L, 1).stroked(checkfloat(L, 2)));
    } else {
        type_push<T>(L, type_check<T>(L, 1).stroked(type_check<Style>(L, 2)));
    }
    return 1;
}

template <typename T> int type_methodcapped(lua_State *L) {
    type_push<T>(L, type_check<T>(L, 1).capped(type_check<Cap>(L, 2)));
    return 1;
}

template <typename T> int type_methodjoined(lua_State *L) {
    type_push<T>(L, type_check<T>(L, 1).joined(
		type_check<Join>(L, 2), optfloat(L, 3, Style::default_miter_limit)));
    return 1;
}

template <typename T> int type_methodby(lua_State *L) {
    type_push<T>(L, type_check<T>(L, 1).by(type_check<Method>(L, 2)));
    return 1;
}

std::vector<float> checkfloatvector(lua_State *L, int idx) {
	std::vector<float> floats;
    luaL_checktype(L, idx, LUA_TTABLE);
    int n = compat_table_len(L, idx);
    TableIterator<float> begin(L, idx, 1, n), end(L, idx, n+1, n);
    floats.insert(floats.begin(), begin, end);
fprintf(stderr, "produced %d entries\n", (int) floats.size());
	return floats;
}

template <typename T> int type_methoddashed(lua_State *L) {
    type_push<T>(L, type_check<T>(L, 1).dashed(checkfloatvector(L, 2),
		optfloat(L, 3, Style::default_initial_phase),
		optboolean(L, 4, Style::default_phase_reset)));
    return 1;
}

template <typename T>
void type_setstrokable(lua_State *L, int metidx) {
    metidx = compat_abs_index(L, metidx);
    luaL_Reg st_methods[] = {
        { "stroked", &type_methodstroked<T> },
        { "capped", &type_methodcapped<T> },
        { "joined", &type_methodjoined<T> },
        { "dashed", &type_methoddashed<T> },
        { "by", &type_methodby<T> },
        { nullptr, nullptr }
    };
    type_pushmetatable<T>(L, metidx); // meta
    lua_getfield(L, -1, "__index");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
    } // meta index
    lua_pushvalue(L, metidx); // meta index metidx
    compat_setfuncs(L, st_methods, 1); // meta index
    lua_setfield(L, -2, "__index"); // meta
    lua_pop(L, 1);
}

template <typename T>
void type_createmetatable(lua_State *L, int metidx,
    const luaL_Reg *override_meta = nullptr) {
    metidx = compat_abs_index(L, metidx);
    luaL_Reg default_meta[] = {
        { "__gc", &type_gc<T> },
        { "__tostring", &type_tostring<T> },
        { nullptr, nullptr },
    };
    lua_newtable(L); // T_meta
    lua_pushstring(L, type_name<T>());
    lua_setfield(L, -2, "__name");
    lua_pushvalue(L, metidx); // T_meta mettab
    compat_setfuncs(L, default_meta, 1); // T_meta
    // if there are metamethods to be overriden, do so
    if (override_meta) {
        lua_pushvalue(L, metidx); // T_meta mettab
        compat_setfuncs(L, override_meta, 1); // T_meta
    }
    lua_pushstring(L, type_name<T>()); // T_meta T_name
    lua_pushvalue(L, -2); // T_meta T_name T_meta
    lua_settable(L, metidx); // T_meta
    lua_rawseti(L, metidx, type_metatableindex<T>()); //
}


static int newramp(lua_State *L) {
    Spread spread = type_check<Spread>(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int n = compat_table_len(L, 2);
    TableIterator<Stop> begin(L, 2, 1, n), end(L, 2, n+1, n);
    type_push<RampPtr>(L, std::make_shared<Ramp>(spread, begin, end));
    return 1;
}

static int newlineargradient(lua_State *L) {
    RampPtr ramp_ptr = type_check<RampPtr>(L, 1);
    float x1 = checkfloat(L, 2);
    float y1 = checkfloat(L, 3);
    float x2 = checkfloat(L, 4);
    float y2 = checkfloat(L, 5);
    uint8_t opacity = rvg::color::unorm_to_uint8_t(optfloat(L, 6, 1.f));
    type_push<Paint>(L, Paint(std::make_shared<LinearGradient>(ramp_ptr,
		x1, y1, x2, y2), opacity));
    return 1;
}

static IImagePtr checkimage(lua_State *L, int idx) {
    idx = compat_abs_index(L, idx);
    IImagePtr *p_ptr = reinterpret_cast<IImagePtr *>(lua_touserdata(L, idx));
    if (!p_ptr) luaL_argerror(L, idx, "expected image");
    if (!lua_getmetatable(L, idx)) lua_pushnil(L);
    lua_getfield(L, LUA_REGISTRYINDEX, "image::IImagePtr");
    if (!lua_istable(L, -1)) luaL_argerror(L, idx, "image library not loaded?");
    if (!compat_is_equal(L, -1, -2)) luaL_argerror(L, idx, "expected image");
    lua_pop(L, 2);
    return *p_ptr;
}

static int newradialgradient(lua_State *L) {
    RampPtr ramp_ptr = type_check<RampPtr>(L, 1);
    float cx = checkfloat(L, 2);
    float cy = checkfloat(L, 3);
    float fx = checkfloat(L, 4);
    float fy = checkfloat(L, 5);
    float r = checkfloat(L, 6);
    uint8_t opacity = rvg::color::unorm_to_uint8_t(optfloat(L, 7, 1.f));
    type_push<Paint>(L, Paint(std::make_shared<RadialGradient>(ramp_ptr,
		cx, cy, fx, fy, r), opacity));
    return 1;
}

static int newtexture(lua_State *L) {
    Spread spread = type_check<Spread>(L, 1);
    IImagePtr img_ptr = checkimage(L, 2);
    uint8_t opacity = rvg::color::unorm_to_uint8_t(optfloat(L, 3, 1.f));
    type_push<Paint>(L, Paint(std::make_shared<Texture>(spread,
		img_ptr), opacity));
    return 1;
}

static int newsolidcolor(lua_State *L) {
    RGBA8 rgba8 = type_check<RGBA8>(L, 1);
    uint8_t opacity = rvg::color::unorm_to_uint8_t(optfloat(L, 2, 1.f));
    type_push<Paint>(L, Paint(rgba8, opacity));
    return 1;
}

template <typename T>
struct NamedValue {
    const char *name;
    T value;
};

static NamedValue<Spread> named_spreads[] = {
    {"repeat", Spread::repeat},
    {"reflect", Spread::reflect},
    {"pad", Spread::pad},
    {"transparent", Spread::transparent},
    { nullptr, Spread::transparent},
};

static NamedValue<Cap> named_caps[] = {
    {"butt", Cap::butt},
    {"round", Cap::round},
    {"square", Cap::square},
    {"triangle", Cap::triangle},
    {"fletching", Cap::fletching},
    { nullptr, Cap::butt},
};

static NamedValue<Join> named_joins[] = {
    {"arcs", Join::arcs},
    {"miter", Join::miter},
    {"miterclip", Join::miterclip},
    {"round", Join::round},
    {"bevel", Join::bevel},
    { nullptr, Join::miter},
};

static NamedValue<Method> named_methods[] = {
    { "driver", Method::driver},
    { "curves", Method::curves},
    { "lines", Method::lines},
    { nullptr, Method::driver},
};

static NamedValue<const char *> named_commands[] = {
	{"T", "T"}, {"t", "t"}, {"R", "R"}, {"r", "r"},
	{"A", "A"}, {"a", "a"}, {"C", "C"}, {"c", "c"},
	{"H", "H"}, {"h", "h"}, {"L", "L"}, {"l", "l"},
	{"M", "M"}, {"m", "m"}, {"Q", "Q"}, {"q", "q"},
	{"S", "S"}, {"s", "s"}, {"V", "V"}, {"v", "v"},
	{"Z", "Z"}, {nullptr, nullptr}
};

template <typename T>
void setconstants(lua_State *L, int metidx, NamedValue<T> *pairs, int tabidx) {
    tabidx = compat_abs_index(L, tabidx);
    metidx = compat_abs_index(L, metidx);
	while (pairs->name) {
        lua_pushstring(L, pairs->name);
        type_push<T>(L, pairs->value, metidx);
        lua_settable(L, tabidx);
        pairs++;
    }
}

static int newstencil(lua_State *L, WindingRule winding_rule) {
    type_push<Stencil>(L, rvg::description::make_stencil_primitive(winding_rule,
		type_check<Shape>(L, 1)));
    return 1;
}

static int newostencil(lua_State *L) {
    return newstencil(L, WindingRule::odd);
}

static int newestencil(lua_State *L) {
    return newstencil(L, WindingRule::even);
}

static int newnzstencil(lua_State *L) {
    return newstencil(L, WindingRule::non_zero);
}

static int newzstencil(lua_State *L) {
    return newstencil(L, WindingRule::zero);
}

static int newfill(lua_State *L, WindingRule winding_rule) {
    if (type_is<RGBA8>(L, 2)) {
        type_push<Painted>(L, rvg::description::make_painted_primitive(
			winding_rule, type_check<Shape>(L, 1), Paint(
				type_to<RGBA8>(L, 2), 255)));
        return 1;
    } else {
        type_push<Painted>(L, rvg::description::make_painted_primitive(
			winding_rule, type_check<Shape>(L, 1), type_check<Paint>(L, 2)));
        return 1;
    }
}

static int newofill(lua_State *L) {
    return newfill(L, WindingRule::odd);
}

static int newefill(lua_State *L) {
    return newfill(L, WindingRule::even);
}

static int newnzfill(lua_State *L) {
    return newfill(L, WindingRule::non_zero);
}

static int newzfill(lua_State *L) {
    return newfill(L, WindingRule::zero);
}

template <typename T, typename ...As, size_t ...Is>
int newshape_helper(lua_State *L, rvg::meta::sequence<Is...>) {
    int n = static_cast<int>(sizeof...(As));
    int t = lua_gettop(L);
    if (t < n) luaL_error(L, "not enough arguments (needed %d)", n);
    if (t > n) luaL_error(L, "too many arguments (needed %d)", n);
    Shape* p = reinterpret_cast<Shape *>(lua_newuserdata(L, sizeof(Shape)));
    new (p) Shape(std::make_shared<T>(type_check<As>(L, Is+1)...));
    type_setmetatable<Shape>(L, -1);
    return 1;
}

template <typename T, typename ...As>
int newshape(lua_State *L) {
    return newshape_helper<T, As...>(L,
        rvg::meta::make_sequence<sizeof...(As)>{});
}

static int newpolygon(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    if (lua_gettop(L) > 1) luaL_error(L, "too many arguments (needed 1)");
    int n = compat_table_len(L, 1);
    TableIterator<float> begin(L, 1, 1, n), end(L, 1, n+1, n);
    Shape* p = reinterpret_cast<Shape *>(lua_newuserdata(L, sizeof(Shape)));
    new (p) Shape(std::make_shared<Polygon>(begin, end));
    type_setmetatable<Shape>(L, -1);
    return 1;
}

static int newpathfromstring(lua_State *L) {
    const char *svg = lua_tostring(L, 1);
    Shape* s = reinterpret_cast<Shape *>(lua_newuserdata(L, sizeof(Shape)));
    std::shared_ptr<Path> p = std::make_shared<Path>();
    if (!rvg::path::svg::tokens_to_instructions(svg, *p)) {
        p->clear();
        luaL_argerror(L, 1, "invalid SVG path data");
    }
    new (s) Shape(p);
    type_setmetatable<Shape>(L, -1);
    return 1;
}

static int newpathfromtable(lua_State *L) {
    int n = compat_table_len(L, 1);
    Shape* s = reinterpret_cast<Shape *>(lua_newuserdata(L, sizeof(Shape)));
    std::shared_ptr<Path> p = std::make_shared<Path>();
    TableIterator<Token> begin(L, 1, 1, n), end(L, 1, n+1, n);
    if (!rvg::path::svg::tokens_to_instructions(begin, end, *p)) {
        p->clear();
        luaL_argerror(L, 1, "invalid SVG path data");
    }
    new (s) Shape(p);
    type_setmetatable<Shape>(L, -1);
    return 1;
}

static int newpath(lua_State *L) {
    if (lua_gettop(L) > 1) luaL_error(L, "too many arguments (needed 1)");
    if (lua_type(L, 1) == LUA_TSTRING) return newpathfromstring(L);
    else if (lua_type(L, 1) == LUA_TTABLE) return newpathfromtable(L);
    else return luaL_argerror(L, 1, "expected table or string");
}

static int newrotation(lua_State *L) {
    return type_push<Xform>(L, xf_newrotation(L, 1));
}

static int newscaling(lua_State *L) {
    return type_push<Xform>(L, xf_newscaling(L, 1));
}

static int newtranslation(lua_State *L) {
    return type_push<Xform>(L, xf_newtranslation(L, 1));
}

static int newwindowviewport(lua_State *L) {
    return type_push<Xform>(L, xf_newwindowviewport(L, 1));
}

static int newidentity(lua_State *L) {
    using rvg::xform::identity;
    int n = lua_gettop(L);
    type_push<Xform>(L, identity());
    if (n > 0) luaL_error(L, "too many arguments");
    return 1;
}

static int newlinear(lua_State *L) {
    return type_push<Xform>(L, xf_newlinear(L, 1));
}

static int newaffinity(lua_State *L) {
    return type_push<Xform>(L, xf_newaffinity(L, 1));
}

template <typename T>
int is_singleton_or_list_of(lua_State *L, int idx) {
    if (lua_type(L, idx) == LUA_TTABLE) {
        lua_rawgeti(L, idx, 1);
        int ret = type_is<T>(L, -1);
        lua_pop(L, 1);
        return ret;
    } else {
        return type_is<T>(L, idx);
    }
}

template <typename T>
int newclip_helper(lua_State *L) {
    std::vector<Stencil> clipper;
    if (lua_type(L, 1) == LUA_TTABLE) {
        int n = compat_table_len(L, 1);
        TableIterator<Stencil> begin(L, 1, 1, n), end(L, 1, n+1, n);
        clipper.insert(clipper.begin(), begin, end);
    } else {
        clipper.push_back(type_check<Stencil>(L, 1));
    }
    std::vector<T> clippee;
    if (lua_type(L, 2) == LUA_TTABLE) {
        int n = compat_table_len(L, 2);
        TableIterator<T> begin(L, 2, 1, n), end(L, 2, n+1, n);
        clippee.insert(clippee.begin(), begin, end);
    } else {
        clippee.push_back(type_check<T>(L, 2));
    }
    type_push<T>(L, clip(clipper, clippee));
    return 1;
}

int newclip(lua_State *L) {
    using rvg::description::scene;
    if (!is_singleton_or_list_of<Stencil>(L, 1))
        return luaL_argerror(L, 1, "expected stencil or list of");
    if (is_singleton_or_list_of<Stencil>(L, 2)) {
        return newclip_helper<Stencil>(L);
    } else if (is_singleton_or_list_of<Painted>(L, 2)) {
        return newclip_helper<Painted>(L);
    } else
        return luaL_argerror(L, 2, "expected stencil, painted, or list of");
}

template <typename T>
int newtransform_helper(lua_State *L) {
    std::vector<T> totransform;
    if (lua_type(L, 2) == LUA_TTABLE) {
        int n = compat_table_len(L, 2);
        TableIterator<T> begin(L, 2, 1, n), end(L, 2, n+1, n);
        totransform.insert(totransform.begin(), begin, end);
    } else {
        totransform.push_back(type_check<T>(L, 2));
    }
    type_push<T>(L, transform(type_check<Xform>(L, 1), totransform));
    return 1;
}

int newtransform(lua_State *L) {
    using rvg::description::scene;
    if (is_singleton_or_list_of<Stencil>(L, 2)) {
        return newtransform_helper<Stencil>(L);
    } else if (is_singleton_or_list_of<Painted>(L, 2)) {
        return newtransform_helper<Painted>(L);
    } else
        return luaL_argerror(L, 2, "expected stencil, painted, or list of");
}

int newfade(lua_State *L) {
    using rvg::description::scene;
    if (is_singleton_or_list_of<Painted>(L, 2)) {
        std::vector<Painted> tofade;
        if (lua_type(L, 2) == LUA_TTABLE) {
            int n = compat_table_len(L, 2);
            TableIterator<Painted> begin(L, 2, 1, n), end(L, 2, n+1, n);
            tofade.insert(tofade.begin(), begin, end);
        } else {
            tofade.push_back(type_to<Painted>(L, 2));
        }
        type_push<Painted>(L, fade(rvg::color::unorm_to_uint8_t(
                checkfloat(L, 1)), tofade));
        return 1;
    } else
        return luaL_argerror(L, 2, "expected painted, or list of");
}

int newblur(lua_State *L) {
    using rvg::description::scene;
    if (is_singleton_or_list_of<Painted>(L, 2)) {
        std::vector<Painted> toblur;
        if (lua_type(L, 2) == LUA_TTABLE) {
            int n = compat_table_len(L, 2);
            TableIterator<Painted> begin(L, 2, 1, n), end(L, 2, n+1, n);
            toblur.insert(toblur.begin(), begin, end);
        } else {
            toblur.push_back(type_to<Painted>(L, 2));
        }
        type_push<Painted>(L, blur(checkfloat(L, 1), toblur));
        return 1;
    } else
        return luaL_argerror(L, 2, "expected painted, or list of");
}

int newscene(lua_State *L) {
    using rvg::description::scene;
    luaL_checktype(L, 1, LUA_TTABLE);
    if (lua_gettop(L) > 1) luaL_error(L, "too many arguments");
    int n = compat_table_len(L, 1);
    TableIterator<Painted> begin(L, 1, 1, n), end(L, 1, n+1, n);
    // ??D We could avoid creating this array if we exposed
    // a version of the scene() function that receives iterators...
    std::vector<Painted> painted(begin, end);
    scene(painted);
    type_push<XformableScene>(L, scene(painted));
    return 1;
}

static int newstroke(lua_State *L) {
    return type_push<Style>(L, Style().stroked(checkfloat(L, 1)));
}

static luaL_Reg driver[] = {
    {"viewport", &type_new<Viewport, int,int,int,int> },
    {"window", &type_new<Window, float,float,float,float> },
    {"rgba8", &type_new<RGBA8, int,int,int,int> },
    {"rgb8", &type_new<RGBA8, int,int,int> },
    {"rgba", &type_new<RGBA8, float,float,float,float> },
    {"rgb", &type_new<RGBA8, float,float,float> },
    {"triangle", &newshape<Triangle, float,float,float,float,float,float> },
    {"circle", &newshape<Circle, float,float,float> },
    {"rect", &newshape<Rect, float,float,float,float> },
    {"ramp", newramp },
    {"solid_color", newsolidcolor },
    {"linear_gradient", newlineargradient },
    {"radial_gradient", newradialgradient },
    {"texture", newtexture },
    {"stencil", newnzstencil },
    {"nzstencil", newnzstencil },
    {"zstencil", newzstencil },
    {"estencil", newestencil },
    {"eostencil", newostencil },
    {"ostencil", newostencil },
    {"fill", newnzfill },
    {"nzfill", newnzfill },
    {"zfill", newzfill },
    {"efill", newefill },
    {"eofill", newofill },
    {"ofill", newofill },
    {"polygon", newpolygon },
    {"path", newpath },
    {"identity", newidentity },
    {"scaling", newscaling },
    {"rotation", newrotation },
    {"translation", newtranslation },
    {"windowviewport", newwindowviewport },
    {"linear", newlinear },
    {"scene", newscene },
    {"affinity", newaffinity },
    {"clip", newclip },
    {"blur", newblur },
    {"fade", newfade },
    {"transform", newtransform },
    {"stroke", newstroke },
    { nullptr, nullptr }
};

template <typename BBOX> int bboxindex(lua_State *L) {
    BBOX *b = type_topointer<BBOX>(L, 1);
    if (lua_isnumber(L, 2)) {
        int i = static_cast<int>(luaL_checkinteger(L, 2));
        i = i-1;
        if (i >= 0 && i < static_cast<int>(b->corners().size())) {
            type_push<typename BBOX::value_type>(L, b->corners()[i]);
        } else {
            lua_pushnil(L);
        }
    } else {
        lua_pushnil(L);
    }
    return 1;
}

template <typename BBOX> int bboxlen(lua_State *L) {
    BBOX *b = type_topointer<BBOX>(L, 1);
    lua_pushinteger(L, static_cast<int>(b->corners().size()));
    return 1;
}

static luaL_Reg metawindow[] = {
    {"__index", &bboxindex<Window>},
    {"__len", &bboxlen<Window>},
    {nullptr, nullptr}
};

static luaL_Reg metaviewport[] = {
    {"__index", &bboxindex<Viewport>},
    {"__len", &bboxlen<Viewport>},
    {nullptr, nullptr}
};

static void createmetatables(lua_State *L, int metidx) {
    type_createmetatable<XformableScene>(L, metidx);
        type_setxformable<XformableScene>(L, metidx);
    type_createmetatable<Window>(L, metidx, metawindow);
    type_createmetatable<Viewport>(L, metidx, metaviewport);
    type_createmetatable<RGBA8>(L, metidx);
    type_createmetatable<Spread>(L, metidx);
    type_createmetatable<RampPtr>(L, metidx);
    type_createmetatable<Paint>(L, metidx);
        type_setxformable<Paint>(L, metidx);
    type_createmetatable<Painted>(L, metidx);
        type_setxformable<Painted>(L, metidx);
    type_createmetatable<Stencil>(L, metidx);
        type_setxformable<Stencil>(L, metidx);
    type_createmetatable<Shape>(L, metidx);
        type_setxformable<Shape>(L, metidx);
        type_setstrokable<Shape>(L, metidx);
    type_createmetatable<Xform>(L, metidx);
        type_setxformable<Xform>(L, metidx);
    type_createmetatable<Cap>(L, metidx);
    type_createmetatable<Join>(L, metidx);
    type_createmetatable<Method>(L, metidx);
    type_createmetatable<Style>(L, metidx);
        type_setstrokable<Style>(L, metidx);
}

static void setcolors(lua_State *L, int metidx, int coloridx) {
    metidx = compat_abs_index(L, metidx);
    coloridx = compat_abs_index(L, coloridx);
    for (const auto &color: rvg::color::named::string_to_rgba8) {
        type_push<RGBA8>(L, color.second, metidx);
        lua_setfield(L, coloridx, color.first.c_str());
    }
}

void mergetables(lua_State *L, int srcidx, int destidx) {
    srcidx = compat_abs_index(L, srcidx);
    destidx = compat_abs_index(L, destidx);
    lua_pushnil(L);
    while (lua_next(L, srcidx) != 0) {
        lua_pushvalue(L, -2);
        lua_pushvalue(L, -2);
        lua_settable(L, destidx);
        lua_pop(L, 1);
    }
}

namespace rvg {
    namespace description {
        namespace lua {

// creates the module table with all functions and a another
// table with all metatables created
int newdriver(lua_State *L) {
    lua_newtable(L); // driver
    lua_newtable(L); // driver mettab
    createmetatables(L, -1); // driver mettab
	lua_newtable(L); // driver mettab spread
    setconstants(L, -2, named_spreads, -1);
	lua_setfield(L, -3, "spread");
	lua_newtable(L); // driver mettab cap
    setconstants(L, -2, named_caps, -1);
	lua_setfield(L, -3, "cap");
	lua_newtable(L); // driver mettab join
    setconstants(L, -2, named_joins, -1);
	lua_setfield(L, -3, "join");
	lua_newtable(L); // driver mettab method
    setconstants(L, -2, named_methods, -1);
	lua_setfield(L, -3, "method"); // driver mettab
	setconstants(L, -1, named_commands, -2);
	lua_newtable(L); // driver mettab color
    setcolors(L, -2, -1);
	lua_setfield(L, -3, "color"); // driver mettab
    lua_pushvalue(L, -2); // driver mettab driver
    lua_pushvalue(L, -2); // driver mettab driver mettab
    compat_setfuncs(L, driver, 1); // driver mettab driver
    compat_require(L, "image"); // driver mettab driver image
    lua_setfield(L, -2, "image");
    compat_require(L, "base64"); // driver mettab driver base64
    lua_setfield(L, -2, "base64");
    compat_require(L, "math"); // driver mettab driver math
    lua_getglobal(L, "type"); // driver mettab driver math type
    mergetables(L, -2, -3); // driver mettab driver math type
    lua_setfield(L, -3, "type");
    lua_pop(L, 2);
    return 2;
}

XformableScene checkxformablescene(lua_State *L, int idx) {
    return type_check<XformableScene>(L, idx);
}

Window checkwindow(lua_State *L, int idx) {
    return type_check<Window>(L, idx);
}

Viewport checkviewport(lua_State *L, int idx) {
    return type_check<Viewport>(L, idx);
}

std::vector<std::string> checkargs(lua_State *L, int idx) {
    luaL_checktype(L, idx, LUA_TTABLE);
    int n = compat_table_len(L, idx);
    std::vector<std::string> args;
    TableIterator<std::string> begin(L, idx, 1, n), end(L, idx, n+1, n);
    args.insert(args.begin(), begin, end);
    return args;
}

std::vector<std::string> optargs(lua_State *L, int idx,
    const std::vector<std::string> &def) {
    if (!lua_istable(L, idx)) return def;
    else return checkargs(L, idx);
}

} } } // namespace rvg::description::lua
