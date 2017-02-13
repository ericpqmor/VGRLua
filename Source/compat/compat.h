#ifndef COMPAT_H
#define COMPAT_H

#include <cstdio>
#include <lua.hpp>
#include <lauxlib.h>

void compat_require(lua_State *L, const char* modname);
FILE* compat_check_file(lua_State *L, int idx);
int compat_abs_index(lua_State *L, int idx);
int compat_is_equal(lua_State *L, int idx1, int idx2);
int compat_table_len(lua_State *L, int idx);
void compat_setfuncs(lua_State *L, const luaL_Reg *l, int nup);
void compat_setuservalue(lua_State *L, int idx);
void compat_getuservalue(lua_State *L, int idx);
void compat_checknargs(lua_State *L, int n);

#endif
