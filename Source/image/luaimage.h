#ifndef LUAIMAGE_H
#define LUAIMAGE_H

#include <lua.hpp>

extern "C"
#ifndef _WIN32
__attribute__((visibility("default")))
#else
__declspec(dllexport)
#endif
int luaopen_image(lua_State *L);

#endif // LUAIMAGE_H
