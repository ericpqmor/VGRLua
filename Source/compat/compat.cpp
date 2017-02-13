#include "compat/compat.h"

void compat_checknargs(lua_State *L, int n) {
    int t = lua_gettop(L);
    if (t > n) {
        luaL_argerror(L, t, "too many arguments");
    } else if (t < n) {
        luaL_argerror(L, n, "too few arguments");
    }
}

void compat_require(lua_State *L, const char* modname) {
    lua_getglobal(L, "require");
    lua_pushstring(L, modname);
    lua_call(L, 1, 1);
}

void compat_setuservalue(lua_State *L, int idx) {
#if LUA_VERSION_NUM > 501
    lua_setuservalue(L, idx);
#else
    lua_setfenv(L, idx);
#endif
}

void compat_getuservalue(lua_State *L, int idx) {
#if LUA_VERSION_NUM > 501
    lua_getuservalue(L, idx);
#else
    lua_getfenv(L, idx);
#endif
}

int compat_abs_index(lua_State *L, int idx) {
#if LUA_VERSION_NUM > 501
    return lua_absindex(L, idx);
#else
    // if not a pseudo index, convert to absolute
    if (idx <= LUA_REGISTRYINDEX) return idx;
    else return idx < 0? lua_gettop(L) + idx + 1: idx;
#endif
}

FILE* compat_check_file(lua_State *L, int idx) {
#if LUA_VERSION_NUM > 501
    luaL_Stream *ls = (luaL_Stream *) luaL_checkudata(L, idx, LUA_FILEHANDLE);
    if (ls->closef == NULL) luaL_argerror(L, idx, "file is closed");
    return ls->f;
#else
    FILE **fpp=(FILE **)luaL_checkudata(L, idx, LUA_FILEHANDLE);
    if (!*fpp) luaL_argerror(L, idx, "file is closed");
    return *fpp;
#endif
}

int compat_is_equal(lua_State *L, int idx1, int idx2) {
#if LUA_VERSION_NUM > 501
    return lua_compare(L, idx1, idx2, LUA_OPEQ);
#else
    return lua_equal(L, idx1, idx2);
#endif
}

int compat_table_len(lua_State *L, int idx) {
#if LUA_VERSION_NUM > 501
    return static_cast<int>(lua_rawlen(L, idx));
#else
    return static_cast<int>(lua_objlen(L, idx));
#endif
}

void compat_setfuncs(lua_State *L, const luaL_Reg *l, int nup) {
#if LUA_VERSION_NUM < 502
  // copied from Lua 5.2
  luaL_checkstack(L, nup, "too many upvalues");
  for (; l->name != NULL; l++) {  /* fill the table with given functions */
    int i;
    for (i = 0; i < nup; i++)  /* copy upvalues to the top */
      lua_pushvalue(L, -nup);
    lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
    lua_setfield(L, -(nup + 2), l->name);
  }
  lua_pop(L, nup);  /* remove upvalues */
#else
  return luaL_setfuncs(L, l, nup);
#endif
}
