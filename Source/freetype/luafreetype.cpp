#include <cstdio>
#include <lua.hpp>
#include <lauxlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_GLYPH_H
#include FT_BBOX_H

#define LIBRARYIDX (lua_upvalueindex(1))
#define METAFACEIDX (lua_upvalueindex(2))

#include "compat/compat.h"
#include "image/luaimage.h"
#include "image/image.h"

#include "freetype/luafreetype.h"

using namespace rvg;

static int indexface(lua_State *L) {
    compat_getuservalue(L, 1);
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    return 1;
}

static int next(lua_State *L) {
    lua_settop(L, 2);
    if (lua_next(L, 1))
       return 2;
    else {
        lua_pushnil(L);
        return 1;
    }
}

static int pairs(lua_State *L) {
    lua_pushcfunction(L, next);
    compat_getuservalue(L, 1);
    lua_pushnil(L);
    return 3;
}

FT_Face *checkface(lua_State *L, int idx) {
    idx = compat_abs_index(L, idx);
    if (!lua_getmetatable(L, idx)) lua_pushnil(L);
    if (!compat_is_equal(L, -1, METAFACEIDX))
        luaL_argerror(L, idx, "expected face");
    lua_pop(L, 1);
    return reinterpret_cast<FT_Face *>(lua_touserdata(L, idx));
}

static int tostringface(lua_State *L) {
    FT_Face *face = checkface(L, 1);
    lua_pushfstring(L, "face{%s,%s}", (*face)->family_name,
            (*face)->style_name);
    return 1;
}

static int  gcface(lua_State *L) {
    FT_Face *face = checkface(L, 1);
    FT_Done_Face(*face);
    return 0;
}

static const luaL_Reg metaface[] = {
    {"__gc", gcface},
    {"__tostring", tostringface},
    {"__index", indexface},
    {"__pairs", pairs},
    {NULL, NULL}
};

enum class FixedPoint {
    no,
    yes
};

void newglyphalpha(lua_State *L, FT_Bitmap *bitmap, int pixels_per_EM,
    int bitmap_top, int bitmap_left) {
    int width = bitmap->width;
    int height = bitmap->rows;
    int pitch = bitmap->pitch;
    lua_newtable(L);
        lua_pushinteger(L, pixels_per_EM);
        lua_setfield(L, -2, "pixels_per_EM");
        image::Image<uint8_t, 1> *p_img = new image::Image<uint8_t,1>();
        if (!p_img) luaL_error(L, "out of memory");
        image::IImagePtr *p_img_ptr = reinterpret_cast<image::IImagePtr *>(
            lua_newuserdata(L, sizeof(image::IImagePtr)));
        new (p_img_ptr) image::IImagePtr(p_img);
        // get mono image metatable
        lua_getfield(L, LUA_REGISTRYINDEX, "image::IImagePtr");
        lua_setmetatable(L, -2);
        p_img->load(width, height, -pitch, 1, bitmap->buffer+(height-1)*pitch);
        lua_setfield(L, -2, "image");
        lua_pushinteger(L, bitmap_left);
        lua_setfield(L, -2, "left");
        lua_pushinteger(L, bitmap_top);
        lua_setfield(L, -2, "top");
        lua_pushinteger(L, bitmap_top-static_cast<int>(bitmap->rows));
        lua_setfield(L, -2, "bottom");
}

static int glyphalphaface(lua_State *L) {
    FT_Face face = *checkface(L, 1);
    int index = static_cast<int>(luaL_checkinteger(L, 2));
    int pixels_per_EM = static_cast<int>(luaL_checkinteger(L, 3));
    FT_Set_Pixel_Sizes(face, 0, pixels_per_EM);
    if (!FT_Load_Glyph(face, index,
        FT_LOAD_IGNORE_TRANSFORM | FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT) &&
        !FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) {
        newglyphalpha(L, &face->glyph->bitmap, pixels_per_EM,
            face->glyph->bitmap_top, face->glyph->bitmap_left);
        lua_pushvalue(L, 1);
        lua_setfield(L, -2, "face");
        return 1;
    } else {
        lua_pushnil(L);
        return 1;
    }
}

static void newglyphmetrics(lua_State *L, FT_GlyphSlot glyph) {
    lua_newtable(L);
        lua_newtable(L);
            lua_pushinteger(L, glyph->metrics.width);
            lua_setfield(L, -2, "width");
            lua_pushinteger(L, glyph->metrics.height);
            lua_setfield(L, -2, "height");
            lua_pushinteger(L, glyph->metrics.horiBearingX);
            lua_setfield(L, -2, "horiBearingX");
            lua_pushinteger(L, glyph->metrics.horiBearingY);
            lua_setfield(L, -2, "horiBearingY");
            lua_pushinteger(L, glyph->metrics.horiAdvance);
            lua_setfield(L, -2, "horiAdvance");
            lua_pushinteger(L, glyph->metrics.vertBearingX);
            lua_setfield(L, -2, "vertBearingX");
            lua_pushinteger(L, glyph->metrics.vertBearingY);
            lua_setfield(L, -2, "vertBearingY");
            lua_pushinteger(L, glyph->metrics.vertAdvance);
            lua_setfield(L, -2, "vertAdvance");
        lua_setfield(L, -2, "metrics");
        lua_pushinteger(L, glyph->linearHoriAdvance);
        lua_setfield(L, -2, "linearHoriAdvance");
        lua_pushinteger(L, glyph->linearVertAdvance);
        lua_setfield(L, -2, "linearVertAdvance");
        lua_newtable(L);
            lua_pushinteger(L, glyph->advance.x);
            lua_setfield(L, -2, "x");
            lua_pushinteger(L, glyph->advance.y);
            lua_setfield(L, -2, "y");
        lua_setfield(L, -2, "advance");
}

static int glyphmetricsface(lua_State *L) {
    FT_Face face = *checkface(L, 1);
    int index = static_cast<int>(luaL_checkinteger(L, 2));
    if (!FT_Load_Glyph(face, index,
        FT_LOAD_LINEAR_DESIGN | FT_LOAD_NO_SCALE | FT_LOAD_IGNORE_TRANSFORM)) {
        newglyphmetrics(L, face->glyph);
        lua_pushvalue(L, 1);
        lua_setfield(L, -2, "face");
        return 1;
    } else {
        lua_pushnil(L);
        return 1;
    }
}

struct Context {
    lua_State *L;
    int tabidx, cmdidx;
};

static int moveto(const FT_Vector* p0, void *user) {
    Context *c = reinterpret_cast<Context *>(user);
    if (c->cmdidx > 1) {
        lua_pushliteral(c->L, "close_path");
        lua_rawseti(c->L, c->tabidx, c->cmdidx++);
    }
    lua_pushliteral(c->L, "move_to_abs");
    lua_rawseti(c->L, c->tabidx, c->cmdidx++);
    lua_pushinteger(c->L, p0->x);
    lua_rawseti(c->L, c->tabidx, c->cmdidx++);
    lua_pushinteger(c->L, p0->y);
    lua_rawseti(c->L, c->tabidx, c->cmdidx++);
    return 0;
}

static int lineto(const FT_Vector* p0, void *user) {
    Context *c = reinterpret_cast<Context *>(user);
    lua_pushliteral(c->L, "line_to_abs");
    lua_rawseti(c->L, c->tabidx, c->cmdidx++);
    lua_pushinteger(c->L, p0->x);
    lua_rawseti(c->L, c->tabidx, c->cmdidx++);
    lua_pushinteger(c->L, p0->y);
    lua_rawseti(c->L, c->tabidx, c->cmdidx++);
    return 0;
}

static int quadto(const FT_Vector* p0, const FT_Vector *p1, void *user) {
    Context *c = reinterpret_cast<Context *>(user);
    lua_pushliteral(c->L, "quad_to_abs");
    lua_rawseti(c->L, c->tabidx, c->cmdidx++);
    lua_pushinteger(c->L, p0->x);
    lua_rawseti(c->L, c->tabidx, c->cmdidx++);
    lua_pushinteger(c->L, p0->y);
    lua_rawseti(c->L, c->tabidx, c->cmdidx++);
    lua_pushinteger(c->L, p1->x);
    lua_rawseti(c->L, c->tabidx, c->cmdidx++);
    lua_pushinteger(c->L, p1->y);
    lua_rawseti(c->L, c->tabidx, c->cmdidx++);
    return 0;
}

static int cubicto(const FT_Vector* p0, const FT_Vector *p1,
    const FT_Vector *p2, void *user) {
    Context *c = reinterpret_cast<Context *>(user);
    lua_pushliteral(c->L, "cubic_to_abs");
    lua_rawseti(c->L, c->tabidx, c->cmdidx++);
    lua_pushinteger(c->L, p0->x);
    lua_rawseti(c->L, c->tabidx, c->cmdidx++);
    lua_pushinteger(c->L, p0->y);
    lua_rawseti(c->L, c->tabidx, c->cmdidx++);
    lua_pushinteger(c->L, p1->x);
    lua_rawseti(c->L, c->tabidx, c->cmdidx++);
    lua_pushinteger(c->L, p1->y);
    lua_rawseti(c->L, c->tabidx, c->cmdidx++);
    lua_pushinteger(c->L, p2->x);
    lua_rawseti(c->L, c->tabidx, c->cmdidx++);
    lua_pushinteger(c->L, p2->y);
    lua_rawseti(c->L, c->tabidx, c->cmdidx++);
    return 0;
}

static void newglyphoutline(lua_State *L, FT_Outline *outline) {
    static FT_Outline_Funcs funcs{moveto, lineto, quadto, cubicto, 0, 0};
    lua_newtable(L);
    int tabidx = lua_gettop(L);
    Context ctx{L, tabidx, 1};
    FT_Outline_Decompose(outline, &funcs, &ctx);
    if (ctx.cmdidx > 1) {
        lua_pushliteral(L, "close_path");
        lua_rawseti(L, tabidx, ctx.cmdidx);
    }
}

static int glyphoutlineface(lua_State *L) {
    FT_Face face = *checkface(L, 1);
    int index = static_cast<int>(luaL_checkinteger(L, 2));
    if (!FT_Load_Glyph(face, index,
        FT_LOAD_LINEAR_DESIGN | FT_LOAD_NO_SCALE | FT_LOAD_IGNORE_TRANSFORM)) {
        newglyphoutline(L, &face->glyph->outline);
        lua_pushvalue(L, 1);
        lua_setfield(L, -2, "face");
        return 1;
    } else {
        lua_pushnil(L);
        return 1;
    }
}

static int kernface(lua_State *L) {
    FT_Face face = *checkface(L, 1);
    int previndex = static_cast<int>(luaL_checkinteger(L, 2));
    int index = static_cast<int>(luaL_checkinteger(L, 3));
    if (FT_HAS_KERNING(face)) {
        FT_Vector delta;
        FT_Get_Kerning(face, previndex, index, FT_KERNING_UNSCALED, &delta);
        lua_pushinteger(L, delta.x);
        lua_pushinteger(L, delta.y);
        return 2;
    } else {
        lua_pushinteger(L, 0);
        lua_pushinteger(L, 0);
        return 2;
    }
}

static int charindexface(lua_State *L) {
    lua_pushinteger(L, FT_Get_Char_Index(*checkface(L, 1),
        static_cast<int>(luaL_checkinteger(L, 2))));
    return 1;
}

static const luaL_Reg methodsface[] = {
    {"glyphoutline", glyphoutlineface},
    {"glyphalpha", glyphalphaface},
    {"glyphmetrics", glyphmetricsface},
    {"kern", kernface},
    {"charindex", charindexface},
    {NULL, NULL}
};

FT_Library upvaluelibrary(lua_State *L) {
    return *reinterpret_cast<FT_Library *>(lua_touserdata(L, LIBRARYIDX));
}

enum class ProbeOnly {
    no,
    yes
};

void copyfaceattribs(lua_State *L, FT_Face face, int idx, ProbeOnly probe_only) {
    idx = compat_abs_index(L, idx);
    lua_pushinteger(L, face->num_faces);
    lua_setfield(L, idx, "num_faces");
    lua_pushinteger(L, face->style_flags);
    lua_setfield(L, idx, "style_flags");
    if (probe_only == ProbeOnly::no) {
        lua_pushinteger(L, face->face_index);
        lua_setfield(L, idx, "face_index");
        lua_pushinteger(L, face->num_glyphs);
        lua_setfield(L, idx, "num_glyphs");
        if (face->family_name) {
            lua_pushstring(L, face->family_name);
            lua_setfield(L, idx, "face_family");
        }
        if (face->style_name) {
            lua_pushstring(L, face->style_name);
            lua_setfield(L, idx, "style_name");
        }
        lua_pushinteger(L, face->units_per_EM);
        lua_setfield(L, idx, "units_per_EM");
        lua_pushinteger(L, face->ascender);
        lua_setfield(L, idx, "ascender");
        lua_pushinteger(L, face->descender);
        lua_setfield(L, idx, "descender");
        lua_pushinteger(L, face->height);
        lua_setfield(L, idx, "height");
        lua_pushinteger(L, face->max_advance_width);
        lua_setfield(L, idx, "max_advance_width");
        lua_pushinteger(L, face->max_advance_height);
        lua_setfield(L, idx, "max_advance_height");
        lua_pushinteger(L, face->underline_position);
        lua_setfield(L, idx, "underline_position");
        lua_pushinteger(L, face->underline_thickness);
        lua_setfield(L, idx, "underline_thickness");
        lua_newtable(L);
        lua_pushinteger(L, face->bbox.xMin);
        lua_rawseti(L, -2, 1);
        lua_pushinteger(L, face->bbox.yMin);
        lua_rawseti(L, -2, 2);
        lua_pushinteger(L, face->bbox.xMax);
        lua_rawseti(L, -2, 3);
        lua_pushinteger(L, face->bbox.yMax);
        lua_rawseti(L, -2, 4);
        lua_setfield(L, idx, "bounding_box");
    }
}

int newface(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    int face_index = static_cast<int>(luaL_optinteger(L, 2, 0));
    FT_Face *face = reinterpret_cast<FT_Face *>(
        lua_newuserdata(L, sizeof(FT_Face)));
    if (FT_New_Face(upvaluelibrary(L), path, face_index, face))
        luaL_error(L, "error loading face %d of %s", face_index, path);
    lua_pushvalue(L, METAFACEIDX);
    lua_setmetatable(L, -2);
    ProbeOnly probe_only = face_index < 0? ProbeOnly::yes: ProbeOnly::no;
    if (probe_only == ProbeOnly::no) {
        if (!FT_IS_SCALABLE((*face)))
            luaL_error(L, "error face %d of %s is not scalable", face_index, path);
        if (FT_IS_TRICKY((*face)))
            luaL_error(L, "face %d of %s is 'tricky' and not supported",
                face_index, path);
        FT_Set_Char_Size((*face), 0, 0, 0, 0); // dummy call
    }
    lua_newtable(L);
    lua_pushvalue(L, LIBRARYIDX);
    lua_pushvalue(L, METAFACEIDX);
    compat_setfuncs(L, methodsface, 2);
    copyfaceattribs(L, *face, -1, probe_only);
    compat_setuservalue(L, -2);
    return 1;
}

static const luaL_Reg modfreetype2[] = {
    {"face", newface},
    {NULL, NULL}
};

int gclibrary(lua_State *L) {
    FT_Done_FreeType(*reinterpret_cast<FT_Library *>(lua_touserdata(L, 1)));
    return 0;
}

static void newlibrary(lua_State *L) {
    FT_Library *library = reinterpret_cast<FT_Library *>(
        lua_newuserdata(L, sizeof(FT_Library)));
    if (FT_Init_FreeType(library))
        luaL_error(L, "error loading FreeType");
    lua_newtable(L);
    lua_pushcfunction(L, gclibrary);
    lua_setfield(L, -2, "__gc");
    lua_setmetatable(L, -2);
}

extern "C"
#ifndef _WIN32
__attribute__((visibility("default")))
#else
__declspec(dllexport)
#endif
int luaopen_freetype(lua_State *L) {
    // make sure image library is loaded
    compat_require(L, "image"); lua_pop(L, 1);
    lua_newtable(L); // mod
    newlibrary(L); // mod lib
    lua_newtable(L); // mod lib fm
    lua_pushliteral(L, "freetype.face");
    lua_setfield(L, -2, "name");
    lua_pushvalue(L, -2); lua_pushvalue(L, -2);
    // mod lib fm lib fm
    compat_setfuncs(L, metaface, 2); // mod lib fm
    compat_setfuncs(L, modfreetype2, 2); // mod
    return 1;
}
