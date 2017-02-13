#ifndef LUACHRONOS_H
#define LUACHRONOS_H

#include <lua.hpp>

extern "C"
#ifndef _WIN32
__attribute__((visibility("default")))
#else
__declspec(dllexport)
#endif
int luaopen_chronos(lua_State *L);

#endif // LUACHRONOS_H
