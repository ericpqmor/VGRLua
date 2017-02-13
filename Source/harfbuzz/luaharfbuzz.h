#ifndef LUAHARFBUZZ_H
#define LUAHARFBUZZ_H

#include <lua.hpp>

extern "C"
#ifndef _WIN32
__attribute__((visibility("default")))
#else
__declspec(dllexport)
#endif
int luaopen_harfbuzz(lua_State *L);

#endif // LUAHARFBUZZ_H
