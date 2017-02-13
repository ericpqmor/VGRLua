#ifndef LUAFREETYPE_H
#define LUAFREETYPE_H

#include <lua.hpp>

extern "C"
#ifndef _WIN32
__attribute__((visibility("default")))
#else
__declspec(dllexport)
#endif
int luaopen_freetype(lua_State *L);

#endif // LUAFREETYPE_H
