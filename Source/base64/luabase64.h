#ifndef LUABASE64_H
#define LUABASE64_H

#include <lua.hpp>


extern "C"
#ifndef _WIN32
__attribute__((visibility("default")))
#else
__declspec(dllexport)
#endif
int luaopen_base64(lua_State *L);

#endif // LUABASE64_H
