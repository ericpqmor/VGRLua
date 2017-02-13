#include <cstdio>
#include <lua.hpp>
#include <lauxlib.h>
#include <sstream>

#define BUFFERSIZE 1<<16
#include <b64/encode.h>
#include <b64/decode.h>

#include "compat/compat.h"
#include "base64/luabase64.h"

static std::string encode_base64(const std::string &input) {
    std::istringstream sin(input);
    std::ostringstream sout;
    ::base64::encoder E;
    E.encode(sin, sout);
    return sout.str();
}

static std::string decode_base64(const std::string &input) {
    std::istringstream sin(input);
    std::ostringstream sout;
    ::base64::decoder E;
    E.decode(sin, sout);
    return sout.str();
}

static int encode(lua_State *L) {
    size_t len = 0;
    const char *str = luaL_checklstring(L, 1, &len);
    std::string encoded = encode_base64(std::string(str, len));
    lua_pushlstring(L, encoded.data(), encoded.length());
    return 1;
}

static int decode(lua_State *L) {
    size_t len = 0;
    const char *str = luaL_checklstring(L, 1, &len);
    std::string decoded = decode_base64(std::string(str, len));
    lua_pushlstring(L, decoded.data(), decoded.length());
    return 1;
}

static const luaL_Reg mod[] = {
    {"encode", encode},
    {"decode", decode},
    {NULL, NULL}
};

extern "C"
#ifndef _WIN32
__attribute__((visibility("default")))
#else
__declspec(dllexport)
#endif
int luaopen_base64(lua_State *L) {
    lua_newtable(L);
    compat_setfuncs(L, mod, 0);
    return 1;
}
