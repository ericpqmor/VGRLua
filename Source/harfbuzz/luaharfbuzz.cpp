#include <cstdio>
#include <new>
#include <vector>

#include <hb.h>
#include <hb-ft.h>
#include <hb-ot.h>
#include <hb-icu.h>

#include <lua.hpp>
#include <lauxlib.h>

#include "harfbuzz/luaharfbuzz.h"
#include "compat/compat.h"

enum {
    FONTUP=1,
    BUFFERUP,
    FEATURESUP
};

static hb_font_t *checkfont(lua_State *L, int idx) {
    idx = compat_abs_index(L, idx);
    if (!lua_getmetatable(L, idx)) lua_pushnil(L);
    if (!compat_is_equal(L, -1, lua_upvalueindex(FONTUP)))
        luaL_argerror(L, idx, "expected harfbuzz font");
    lua_pop(L, 1);
    return *reinterpret_cast<hb_font_t **>(lua_touserdata(L, idx));
}

hb_buffer_t *checkbuffer(lua_State *L, int idx) {
    idx = compat_abs_index(L, idx);
    if (!lua_getmetatable(L, idx)) lua_pushnil(L);
    if (!compat_is_equal(L, -1, lua_upvalueindex(BUFFERUP)))
        luaL_argerror(L, idx, "expected harfbuzz buffer");
    lua_pop(L, 1);
    return *reinterpret_cast<hb_buffer_t **>(lua_touserdata(L, idx));
}

typedef std::vector<hb_feature_t> featurevector;

static featurevector *checkfeatures(lua_State *L, int idx) {
    idx = compat_abs_index(L, idx);
    if (!lua_getmetatable(L, idx)) lua_pushnil(L);
    if (!compat_is_equal(L, -1, lua_upvalueindex(FEATURESUP)))
        luaL_argerror(L, idx, "expected harfbuzz features");
    lua_pop(L, 1);
    return reinterpret_cast<featurevector *>(lua_touserdata(L, idx));
}

static featurevector *optfeatures(lua_State *L, int idx, featurevector *def) {
    idx = compat_abs_index(L, idx);
    if (idx > lua_gettop(L) || lua_type(L, idx) == LUA_TNIL) return def;
    return checkfeatures(L, idx);
}

static int gcfeatures(lua_State *L) {
    featurevector *features = checkfeatures(L, 1);
    features->~vector();
    return 0;
}

static int tostringfeatures(lua_State *L) {
    featurevector *features = checkfeatures(L, 1);
    lua_pushfstring(L, "harfbuzz features: %p", features);
    return 1;
}

static const luaL_Reg metafeatures[] = {
    {"__gc", gcfeatures},
    {"__tostring", tostringfeatures},
    {NULL, NULL}
};

static int gcbuffer(lua_State *L) {
    hb_buffer_t *buffer = checkbuffer(L, 1);
    hb_buffer_destroy(buffer);
    return 0;
}

static int tostringbuffer(lua_State *L) {
    hb_buffer_t *buffer = checkbuffer(L, 1);
    lua_pushfstring(L, "harfbuzz buffer: %p", buffer);
    return 1;
}

static const luaL_Reg metabuffer[] = {
    {"__gc", gcbuffer},
    {"__tostring", tostringbuffer},
    {NULL, NULL}
};

static int setdirectionbuffer(lua_State *L) {
    hb_buffer_t *buffer = checkbuffer(L, 1);
    size_t len;
    const char *d = luaL_checklstring(L, 2, &len);
    hb_direction_t direction = hb_direction_from_string(d,
        static_cast<int>(len));
    if (direction == HB_DIRECTION_INVALID)
        luaL_argerror(L, 2, "invalid direction");
    hb_buffer_set_direction(buffer, direction);
    return 1;
}

static int setscriptbuffer(lua_State *L) {
    hb_buffer_t *buffer = checkbuffer(L, 1);
    size_t len;
    const char *s = luaL_checklstring(L, 2, &len);
    hb_script_t script = hb_script_from_string(s, static_cast<int>(len));
    if (script == HB_SCRIPT_INVALID)
        luaL_argerror(L, 2, "invalid script");
    hb_buffer_set_script(buffer, script);
    return 1;
}

static int setlanguagebuffer(lua_State *L) {
    hb_buffer_t *buffer = checkbuffer(L, 1);
    size_t len;
    const char *el = luaL_checklstring(L, 2, &len);
    hb_language_t language = hb_language_from_string(el, static_cast<int>(len));
    if (language == HB_LANGUAGE_INVALID)
        luaL_argerror(L, 2, "invalid language");
    hb_buffer_set_language(buffer, language);
    return 1;
}

static int addbuffer(lua_State *L) {
    hb_buffer_t *buffer = checkbuffer(L, 1);
    size_t data_len;
    const char *data = luaL_checklstring(L, 2, &data_len);
    int text_os = static_cast<int>(luaL_optinteger(L, 3, 1) - 1);
    int text_len = static_cast<int>(luaL_optinteger(L, 4, -1));
    if (text_os < 0 ||
        text_os >= (int) data_len ||
        (text_len < 0 && text_len != -1) ||
        text_os + text_len > (int) data_len)
        luaL_error(L, "invalid range");
    hb_buffer_add_utf8(buffer, data, static_cast<int>(data_len),
        text_os, text_len);
    return 0;
}

static int resetbuffer(lua_State *L) {
    hb_buffer_t *buffer = checkbuffer(L, 1);
    hb_buffer_reset(buffer);
    return 0;
}

static int shapebuffer(lua_State *L) {
    hb_buffer_t *buffer = checkbuffer(L, 1);
    hb_font_t *font = checkfont(L, 2);
    featurevector *features = optfeatures(L, 3, nullptr);
    if (features && !features->empty()) {
        hb_shape(font, buffer, &(*features)[0],
            static_cast<int>(features->size()));
    } else {
        hb_shape(font, buffer, nullptr, 0);
    }
    return 0;
}

static int normalizeglyphsbuffer(lua_State *L) {
    hb_buffer_t *buffer = checkbuffer(L, 1);
    hb_buffer_normalize_glyphs(buffer);
    return 0;
}

static int setclusterlevelbuffer(lua_State *L) {
    hb_buffer_t *buffer = checkbuffer(L, 1);
    static const char *const levels[] = {
        "monotone graphemes", "monotone characters", "characters", nullptr };
    hb_buffer_cluster_level_t level = static_cast<hb_buffer_cluster_level_t>(
        luaL_checkoption(L, 2, "monotone graphemes", levels));
    hb_buffer_set_cluster_level(buffer, level);
    return 0;
}

static int guesssegmentpropertiesbuffer(lua_State *L) {
    hb_buffer_t *buffer = checkbuffer(L, 1);
    hb_buffer_guess_segment_properties(buffer);
    return 0;
}

static int glyphinfosbuffer(lua_State *L) {
    hb_buffer_t *buffer = checkbuffer(L, 1);
    unsigned length = 0;
    hb_glyph_info_t* infos = hb_buffer_get_glyph_infos(buffer, &length);
    lua_createtable(L, length, 0); // infos
    lua_pushliteral(L, "codepoint"); // infos "cp"
    lua_pushliteral(L, "cluster"); // infos "cp" "cl"
    for (unsigned i = 0; i < length; ++i) {
        lua_createtable(L, 0, 2); // infos "cp" "cl" info
        lua_pushvalue(L, -3); // infos "cp" "cl" info "cp"
        lua_pushinteger(L, infos[i].codepoint); // infos "cp" "cl" info "cp" cp
        lua_rawset(L, -3); // infos "cp" "cl" info
        lua_pushvalue(L, -2); // infos "cp" "cl" info "cl"
        lua_pushinteger(L, infos[i].cluster); // infos "cp" "cl" info "cl" cl
        lua_rawset(L, -3); // infos "cp" "cl" info
        lua_rawseti(L, -4, i+1); // infos "cp" "cl"
    }
    lua_pop(L, 2); // infos
    lua_pushinteger(L, (int) length);
    return 2;
}

static int glyphpositionsbuffer(lua_State *L) {
    hb_buffer_t *buffer = checkbuffer(L, 1);
    unsigned length = 0;
    hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(buffer,
        &length);
    lua_createtable(L, length, 0); // poss
    lua_pushliteral(L, "x_advance"); // poss "xa"
    lua_pushliteral(L, "y_advance"); // poss "xa" "ya"
    lua_pushliteral(L, "x_offset"); // poss "xa" "ya" "xo"
    lua_pushliteral(L, "y_offset"); // poss "xa" "ya" "xo" "yo"
    for (unsigned i = 0; i < length; ++i) {
        lua_createtable(L, 0, 2); // poss ... pos
        lua_pushvalue(L, -5); // poss ... pos "xa"
        lua_pushinteger(L, pos[i].x_advance);  // poss ... pos "xa" xa
        lua_rawset(L, -3); // poss ... pos
        lua_pushvalue(L, -4); // poss ... pos "ya"
        lua_pushinteger(L, pos[i].y_advance);  // poss ... pos "ya" ya
        lua_rawset(L, -3); // poss ... pos
        lua_pushvalue(L, -3); // poss ... pos "ya"
        lua_pushinteger(L, pos[i].x_offset);  // poss ... pos "xo" xo
        lua_rawset(L, -3); // poss ... pos
        lua_pushvalue(L, -2); // poss ... pos "yo"
        lua_pushinteger(L, pos[i].y_offset);  // poss ... pos "yo" yo
        lua_rawset(L, -3); // poss ... pos
        lua_rawseti(L, -6, i+1); // poss ...
    }
    lua_pop(L, 4); // poss
    lua_pushinteger(L, (int) length);
    return 2;
}

static const luaL_Reg methodsbuffer[] = {
    {"setdirection", setdirectionbuffer},
    {"setscript", setscriptbuffer},
    {"setlanguage", setlanguagebuffer},
    {"setclusterlevel", setclusterlevelbuffer},
    {"guesssegmentproperties", guesssegmentpropertiesbuffer},
    {"add", addbuffer},
    {"reset", resetbuffer},
    {"shape", shapebuffer},
    {"normalizeglyphs", normalizeglyphsbuffer},
    {"glyphinfos", glyphinfosbuffer},
    {"glyphpositions", glyphpositionsbuffer},
    {NULL, NULL}
};

static int glyphtostringfont(lua_State *L) {
    char name[128];
    hb_font_t *font = checkfont(L, 1);
    int codepoint = static_cast<int>(luaL_checkinteger(L, 2));
    hb_font_glyph_to_string(font, codepoint, name, sizeof(name));
    lua_pushstring(L, name);
    return 1;
}

static int setscalefont(lua_State *L) {
    hb_font_t *font = checkfont(L, 1);
    int sx = static_cast<int>(luaL_checkinteger(L, 2));
    int sy = static_cast<int>(luaL_optinteger(L, 3, sx));
    hb_font_set_scale(font, sx, sy);
    return 0;
}

static int setfuncsfont(lua_State *L) {
    hb_font_t *font = checkfont(L, 1);
    hb_ft_font_set_funcs(font);
    return 0;
}

static int getupemfont(lua_State *L) {
    hb_font_t *font = checkfont(L, 1);
    hb_face_t *face = hb_font_get_face(font);
    lua_pushinteger(L, hb_face_get_upem(face));
    return 1;
}

static int setupemfont(lua_State *L) {
    hb_font_t *font = checkfont(L, 1);
    int upem = static_cast<int>(luaL_checkinteger(L, 2));
    hb_face_t *face = hb_font_get_face(font);
    hb_face_set_upem(face, upem);
    return 0;
}

static const luaL_Reg methodsfont[] = {
    {"glyphtostring", glyphtostringfont},
    {"setscale", setscalefont},
    {"setfuncs", setfuncsfont},
    {"getupem", getupemfont},
    {"setupem", setupemfont},
    {NULL, NULL}
};

static int tostringfont(lua_State *L) {
    hb_font_t *font = checkfont(L, 1);
    lua_pushfstring(L, "harfbuzz font: %p", font);
    return 1;
}

static int gcfont(lua_State *L) {
    hb_font_t *font = checkfont(L, 1);
    hb_font_destroy(font);
    return 0;
}

static const luaL_Reg metafont[] = {
    {"__gc", gcfont},
    {"__tostring", tostringfont},
    {NULL, NULL}
};

static char *loadfile(const char *name, size_t &len) {
    FILE *fp = fopen(name, "rb");
    if (!fp) return nullptr;
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    char *buf = (char *) malloc(len+1);
    if (!buf) return nullptr;
    fseek(fp, 0, SEEK_SET);
    size_t got = fread(buf, 1, len, fp);
    buf[len] = '\0';
    fclose(fp);
    if (got != len) return nullptr;
    else return buf;
}

// ??D there should be a way to share a face loaded with freetype here
static int newfont(lua_State *L) {
    const char *filename = luaL_checkstring(L, 1);
    int face_index = static_cast<int>(luaL_optinteger(L, 2, 0));
    size_t len = 0;
    char *data = loadfile(filename, len);
    if (!data) luaL_error(L, "unable to load font file");
    hb_blob_t *blob = hb_blob_create(data, static_cast<int>(len),
        HB_MEMORY_MODE_WRITABLE, data, (hb_destroy_func_t) free);
    if (blob == hb_blob_get_empty())
        luaL_error(L, "harfbuzz blob creation failed");
    hb_face_t *face = hb_face_create(blob, face_index);
    if (face == hb_face_get_empty())
        luaL_error(L, "harfbuzz face creation failed");
    hb_blob_destroy(blob);
    hb_font_t *font = hb_font_create(face);
    hb_face_destroy(face);
    hb_font_t **ud = reinterpret_cast<hb_font_t **>
        (lua_newuserdata(L, sizeof(hb_font_t *)));
    *ud = font;
    lua_pushvalue(L, lua_upvalueindex(FONTUP));
    lua_setmetatable(L, -2);
    return 1;
}

static int newbuffer(lua_State *L) {
    hb_buffer_t *buffer = hb_buffer_create();
    if (buffer == hb_buffer_get_empty())
        luaL_error(L, "harfbuzz buffer creation failed");
    hb_buffer_set_unicode_funcs(buffer, hb_icu_get_unicode_funcs());
    hb_buffer_set_direction(buffer, HB_DIRECTION_LTR); // default
    hb_buffer_t **ud = reinterpret_cast<hb_buffer_t **>
        (lua_newuserdata(L, sizeof(hb_buffer_t *)));
    *ud = buffer;
    lua_pushvalue(L, lua_upvalueindex(BUFFERUP));
    lua_setmetatable(L, -2);
    return 1;
}

static int newfeatures(lua_State *L) {
    const char *s = luaL_checkstring(L, 1);
    void *p = lua_newuserdata(L, sizeof(featurevector));
    new (p) featurevector;
    featurevector *features = reinterpret_cast<featurevector *>(p);
    lua_pushvalue(L, lua_upvalueindex(FEATURESUP));
    lua_setmetatable(L, -2);
    const char *n = nullptr;
    while (s && *s) {
        n = strchr(s, ',');
        hb_feature_t tmp;
        if (hb_feature_from_string(s, static_cast<int>(n? n-s: -1), &tmp))
            features->push_back(tmp);
        if (!n) break;
        s = n+1;
    }
    return 1;
}

static const luaL_Reg modharfbuzz[] = {
    {"font", newfont},
    {"buffer", newbuffer},
    {"features", newfeatures},
    {NULL, NULL}
};

extern "C"
#ifndef _WIN32
__attribute__((visibility("default")))
#else
__declspec(dllexport)
#endif
int luaopen_harfbuzz(lua_State *L) {
    lua_newtable(L); // mod
    lua_newtable(L); // mod m
    lua_newtable(L); // mod m mf
    lua_newtable(L); // mod m mf mb
    lua_newtable(L); // mod m mf mb mt

    lua_newtable(L); // mod m mf mb mt _idxb
    lua_pushvalue(L, -4); lua_pushvalue(L, -4); lua_pushvalue(L, -4);
    compat_setfuncs(L, methodsbuffer, 3); // mod m mf mb mt _idxb
    lua_setfield(L, -3, "__index"); // mod m mf mb mt
    lua_pushvalue(L, -2); // mod m mf mb mt mb
    lua_pushvalue(L, -4); lua_pushvalue(L, -4); lua_pushvalue(L, -4);
    compat_setfuncs(L, metabuffer, 3); // mod m mf mb mt mb
    lua_pop(L, 1); // mod m mf mb mt

    lua_newtable(L); // mod m mf mb mt _idxf
    lua_pushvalue(L, -4); lua_pushvalue(L, -4); lua_pushvalue(L, -4);
    compat_setfuncs(L, methodsfont, 3); // mod m mf mb mt _idxf
    lua_setfield(L, -4, "__index"); // mod m mf mb mt
    lua_pushvalue(L, -3); // mod m mf mb mt mf
    lua_pushvalue(L, -4); lua_pushvalue(L, -4); lua_pushvalue(L, -4);
    compat_setfuncs(L, metafont, 3); // mod m mf mb mt mf
    lua_pop(L, 1); // mod m mf mb mt

    lua_pushvalue(L, -3); lua_pushvalue(L, -3); lua_pushvalue(L, -3);
    compat_setfuncs(L, metafeatures, 3); // mod m mf mb mt

    lua_pushvalue(L, -5); // mod m mf mb mt mod
    lua_pushvalue(L, -4); lua_pushvalue(L, -4); lua_pushvalue(L, -4);
    compat_setfuncs(L, modharfbuzz, 3); // mod m mf mb mt mod
    lua_pop(L, 1);

    lua_pushliteral(L, "harfbuzz.features"); // mod m mf mb mt "hf"
    lua_setfield(L, -2, "name"); // mod m mf mb mt
    lua_setfield(L, -4, "features"); // mod m mf mb
    lua_pushliteral(L, "harfbuzz.buffer"); // mod mf mb "hb"
    lua_setfield(L, -2, "name"); // mod m mf mb
    lua_setfield(L, -3, "buffer"); // mod m mf
    lua_pushliteral(L, "harfbuzz.font"); // mod m mf "hf"
    lua_setfield(L, -2, "name"); // mod m mf
    lua_setfield(L, -2, "font"); // mod m
    lua_setfield(L, -2, "meta"); // mod

    return 1;
}
