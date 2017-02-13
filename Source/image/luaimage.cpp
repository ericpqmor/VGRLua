#include <cstdio>
#include <array>
#include <new>

#include <lua.hpp>
#include <lauxlib.h>

#include "image/image.h"
#include "image/iimage.h"
#include "image/pngio.h"
#include "compat/compat.h"
#include "meta/meta.h"

#include "image/luaimage.h"

using namespace rvg::image;

static IImage *checkimage(lua_State *L, int idx) {
    idx = compat_abs_index(L, idx);
    IImagePtr *p_img_ptr = reinterpret_cast<IImagePtr *>(lua_touserdata(L, idx));
    if (!lua_getmetatable(L, idx)) lua_pushnil(L);
    if (!compat_is_equal(L, -1, lua_upvalueindex(1)))
        luaL_argerror(L, idx, "expected image");
    lua_pop(L, 1);
    return p_img_ptr->get();
}

static IImagePtr checkimageptr(lua_State *L, int idx) {
    idx = compat_abs_index(L, idx);
    IImagePtr *p_img_ptr = reinterpret_cast<IImagePtr *>(lua_touserdata(L, idx));
    if (!lua_getmetatable(L, idx)) lua_pushnil(L);
    if (!compat_is_equal(L, -1, lua_upvalueindex(1)))
        luaL_argerror(L, idx, "expected image");
    lua_pop(L, 1);
    return *p_img_ptr;
}

template <typename T, size_t N>
IImage *pushimage(lua_State *L, int width, int height) {
    IImagePtr *p_img_ptr = reinterpret_cast<IImagePtr *>(lua_newuserdata(L,
        sizeof(IImagePtr)));
    new (p_img_ptr) IImagePtr(new Image<T,N>);
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_setmetatable(L, -2);
    (*p_img_ptr)->resize(width, height);
    (*p_img_ptr)->set_gamma(rvg::image::Gamma::sRGB);
    return p_img_ptr->get();
}

template <typename T>
IImage *pushimage(lua_State *L, int width, int height, int channels) {
    switch (channels) {
        case 1:
            return pushimage<T,1>(L, width, height);
        case 2:
            return pushimage<T,2>(L, width, height);
        case 3:
            return pushimage<T,3>(L, width, height);
        case 4:
            return pushimage<T,4>(L, width, height);
        default:
            lua_pushnil(L);
            return nullptr;
    }
}

static int newimage(lua_State *L) {
    int width = static_cast<int>(luaL_checkinteger(L, 1));
    if (width <= 0) luaL_argerror(L, 1, "invalid width");
    int height = static_cast<int>(luaL_checkinteger(L, 2));
    if (height <= 0) luaL_argerror(L, 2, "invalid height");
    int channels = static_cast<int>(luaL_checkinteger(L, 3));
    if (channels <= 0 || channels > 4)
        luaL_argerror(L, 3, "invalid number of channels");
    const char *const lst[] = { "uint8_t", "uint16_t", "float", nullptr };
    int channel_type = luaL_checkoption(L, 4, "uint8_t", lst);
    switch (channel_type) {
        case 0:
            pushimage<uint8_t>(L, width, height, channels);
            return 1;
        case 1:
            pushimage<uint16_t>(L, width, height, channels);
            return 1;
        case 2:
            pushimage<float>(L, width, height, channels);
            return 1;
        default:
            return 0;
    }
}

static int isimage(lua_State *L) {
    if (!lua_getmetatable(L, 1)) {
        lua_pushnil(L);
        return 1;
    }
    int ret = compat_is_equal(L, -1, lua_upvalueindex(1));
    lua_pop(L, 1);
    if (ret) lua_pushnumber(L, 1);
    else lua_pushnil(L);
    return 1;
}

static const luaL_Reg modimage[] = {
    {"image", newimage},
    {"is_image", isimage},
    {NULL, NULL}
};

const char *channel_type_name(ChannelType channel_type) {
    switch (channel_type) {
        case ChannelType::channel_float: return "float";
        case ChannelType::channel_uint8_t: return "uint8_t";
        case ChannelType::channel_uint16_t: return "uint16_t";
        case ChannelType::channel_unknown:
        default:
            return "unknown";
    }
}

static int tostringimage(lua_State *L) {
    IImage* img = checkimage(L, 1);
    lua_pushfstring(L, "image{%d,%d,%d,%s}", img->width(), img->height(),
        img->channels(), channel_type_name(img->channel_type()));
    return 1;
}

static int gcimage(lua_State *L) {
	IImagePtr *p_img_ptr = reinterpret_cast<IImagePtr *>(lua_touserdata(L, 1));
    p_img_ptr->~IImagePtr();
    return 0;
}

static int setpixelimage(lua_State *L) {
    IImage *img = checkimage(L, 1);
    int x = static_cast<int>(luaL_checkinteger(L, 2))-1;
    int y = static_cast<int>(luaL_checkinteger(L, 3))-1;
    if (x >= 0 && x < img->width() && y >= 0 && y < img->height()) {
        for (int c = 0; c < img->channels(); c++) {
            img->set_float(x, y, c,
                static_cast<float>(luaL_checknumber(L, 4+c)));
        }
        return 0;
    } else {
        return luaL_error(L, "index out of bounds");
    }
}

static int getpixelimage(lua_State *L) {
    IImage *img = checkimage(L, 1);
    int x = static_cast<int>(luaL_checkinteger(L, 2))-1;
    int y = static_cast<int>(luaL_checkinteger(L, 3))-1;
    if (x >= 0 && x < img->width() && y >= 0 && y < img->height()) {
        for (int c = 0; c < img->channels(); c++) {
            lua_pushnumber(L, img->get_float(x, y, c));
        }
        return img->channels();
    } else {
        return luaL_error(L, "index out of bounds");
    }
}

static int indeximage(lua_State *L) {
    const char *const lst[] = {"set_pixel", "get_pixel",
        "width", "height", "channels", "channel_type", "other", nullptr};
    int i = luaL_checkoption(L, 2, "other", lst);
    switch (i) {
        case 0:
        case 1: {
            lua_getmetatable(L, 1);
            lua_insert(L, 1);
            lua_rawget(L, 1);
            break;
        }
        case 2: {
            IImage *img = checkimage(L, 1);
            lua_pushinteger(L, img->width());
            break;
        }
        case 3: {
            IImage *img = checkimage(L, 1);
            lua_pushinteger(L, img->height());
            break;
        }
        case 4: {
            IImage *img = checkimage(L, 1);
            lua_pushinteger(L, img->channels());
            break;
        }
        case 5: {
            IImage *img = checkimage(L, 1);
            lua_pushstring(L, channel_type_name(img->channel_type()));
            break;
        }
        default:
            lua_pushnil(L);
            break;
    }
    return 1;
}

static const luaL_Reg metaimage[] = {
    {"__gc", gcimage},
    {"__index", indeximage},
    {"__tostring", tostringimage},
    {NULL, NULL}
};

static void describe_helper(lua_State *L, int width, int height,
    int channels, int depth) {
    lua_createtable(L, 0, 4);
    lua_pushinteger(L, width);
    lua_setfield(L, -2, "width");
    lua_pushinteger(L, height);
    lua_setfield(L, -2, "height");
    lua_pushinteger(L, channels);
    lua_setfield(L, -2, "channels");
    lua_pushinteger(L, depth);
    lua_setfield(L, -2, "depth");
}

static int describepng(lua_State *L) {
    int width, height, channels, depth;
    // try to load from string
    if (lua_isstring(L, 1)) {
        size_t len = 0;
        const char *str = lua_tolstring(L, 1, &len);
        if (!pngio::describe(std::string(str, len), &width, &height,
            &channels, &depth)) {
            lua_pushnil(L);
            lua_pushliteral(L, "failed to load PNG from memory");
            return 2;
        } else {
            describe_helper(L, width, height, channels, depth);
            return 1;
        }
    // else try to load from file
    } else {
        FILE *f = compat_check_file(L, 1);
        if (!pngio::describe(f, &width, &height, &channels, &depth)) {
            lua_pushnil(L);
            lua_pushliteral(L, "failed to load PNG from file");
            return 2;
        } else {
            describe_helper(L, width, height, channels, depth);
            return 1;
        }
    }
}

static void copyattrs(lua_State *L, int idx,
	const std::vector<Attribute> &attrs) {
    idx = compat_abs_index(L, idx);
    for (auto &attr: attrs) {
        lua_pushstring(L, attr.first.c_str());
        lua_pushstring(L, attr.second.c_str());
        lua_settable(L, idx);
    }
}


template <typename R>
static int loadpngbyinputtype(lua_State *L, R &&r) {
    int wanted_channels = static_cast<int>(luaL_optinteger(L, 2, 0));
    std::vector<Attribute> attrs;
    std::vector<Attribute> *attrs_ptr =
        lua_istable(L, 3)? &attrs: nullptr;
	IImagePtr img_ptr = pngio::load(std::forward<R>(r), wanted_channels,
        attrs_ptr);
	if (img_ptr) {
		IImagePtr *p_img_ptr = reinterpret_cast<IImagePtr *>(lua_newuserdata(L,
			sizeof(IImagePtr)));
		new (p_img_ptr) IImagePtr(std::move(img_ptr));
		lua_pushvalue(L, lua_upvalueindex(1));
		lua_setmetatable(L, -2);
		if (attrs_ptr) {
            copyattrs(L, 2, attrs);
			return 2;
		} else {
			return 1;
		}
    } else {
        lua_pushnil(L);
        lua_pushliteral(L, "failed to load image");
        return 2;
    }
}

static int loadpng(lua_State *L) {
    // try to load from string
    if (lua_isstring(L, 1)) {
        size_t len = 0;
        const char *str = lua_tolstring(L, 1, &len);
        return loadpngbyinputtype(L, std::string(str, len));
    // else try to load from file
    } else {
        return loadpngbyinputtype(L, compat_check_file(L, 1));
    }
}

static std::vector<std::pair<std::string, std::string> >
optattrs(lua_State *L, int idx) {
    std::vector<std::pair<std::string, std::string> > attrs;
    idx = compat_abs_index(L, idx);
    if (idx <= lua_gettop(L)) {
        luaL_checktype(L, idx, LUA_TTABLE);
        lua_pushnil(L);
        while (lua_next(L, idx) != 0) {
            lua_pushvalue(L, -2);
            const char *key = lua_tostring(L, -1);
            const char *value = lua_tostring(L, -2);
            if (value && key) attrs.emplace_back(key, value);
            lua_pop(L, 2);
        }
    }
    return attrs;
}

template <typename U>
static int stringpng(lua_State *L) {
    IImagePtr img_ptr = checkimageptr(L, 1);
    auto attrs = optattrs(L, 2);
    std::string str;
    if (!pngio::store<U>(&str, img_ptr, attrs))
        luaL_error(L, "store to memory failed");
    lua_pushlstring(L, str.data(), str.length());
    return 1;
}

template <typename U>
static int storepng(lua_State *L) {
    FILE *f = compat_check_file(L, 1);
    IImagePtr img_ptr = checkimageptr(L, 2);
    auto attrs = optattrs(L, 3);
    if (!pngio::store<U>(f, img_ptr, attrs))
        luaL_error(L, "store to file failed");
    lua_pushinteger(L, 1);
    return 1;
}

static const luaL_Reg modpng[] = {
    {"load", loadpng},
    {"describe", describepng},
    {"store8", &storepng<uint8_t>},
    {"store16", &storepng<uint16_t>},
    {"string8", &stringpng<uint8_t>},
    {"string16", &stringpng<uint8_t>},
    {NULL, NULL}
};

extern "C"
#ifndef _WIN32
__attribute__((visibility("default")))
#else
__declspec(dllexport)
#endif
int luaopen_image(lua_State *L) {
    lua_newtable(L); // modimage
    lua_pushliteral(L, "image");
    lua_setfield(L, -2, "name");
    lua_newtable(L); // modimage metaimage
    lua_newtable(L); // modimage metaimate idx
    lua_pushvalue(L, -1); // modimage metaimage metaimage
    compat_setfuncs(L, metaimage, 1); // modimage metaimage
    lua_pushvalue(L, -1); // modimage metaimage metaimage
    lua_setfield(L, LUA_REGISTRYINDEX, "image::IImagePtr");
    lua_pushvalue(L, -1); // modimage metaimage metaimage
    lua_pushcclosure(L, setpixelimage, 1); // modimage metaimage setpixel
    lua_setfield(L, -2, "set_pixel"); // modimage metaimage
    lua_pushvalue(L, -1); // modimage metaimage metaimage
    lua_pushcclosure(L, getpixelimage, 1); // modimage metaimage getpixel
    lua_setfield(L, -2, "get_pixel"); // modimage metaimage
	lua_newtable(L); // modimage metaimage modpng
	lua_pushvalue(L, -2); // modimage metaimage modpng metaimage
    compat_setfuncs(L, modpng, 1); // modimage metaimage modpng
    lua_setfield(L, -3, "png"); // modimage metaimage
    compat_setfuncs(L, modimage, 1); // modimage metaimage
    return 1;
}
