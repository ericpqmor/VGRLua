#ifndef RVG_DESCRIPTION_LUA_H
#define RVG_DESCRIPTION_LUA_H

#include <string>
#include <lua.hpp>

#include "description/description.h"

namespace rvg {
    namespace description {
        namespace lua {

int newdriver(lua_State *L);

XformableScene checkxformablescene(lua_State *L, int idx);

Window checkwindow(lua_State *L, int idx);

Viewport checkviewport(lua_State *L, int idx);

std::vector<std::string> checkargs(lua_State *L, int idx);

std::vector<std::string> optargs(lua_State *L, int idx, 
    const std::vector<std::string> &def = std::vector<std::string>());

} } } // namespace rvg::description::lua;

#endif
