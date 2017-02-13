#include <memory>
#include <tuple>

#include <lua.hpp>

#include "description/lua.h"
#include "compat/compat.h"

#include "image/image.h"
#include "image/pngio.h"
#include "chronos/chronos.h"

#include "driver/cpp/png.h"

namespace rvg {
    namespace driver {
        namespace png {

// This is one of the functions you must implement. It
// receives a scene and a viewport. It returns an
// acceleration datastructure that contains all scene
// information in a form that enables fast sampling.
// For now, it simply returns the scene itself.
Accelerated accelerate(const XformableScene &xs, const Viewport &vp) {
Chronos time;
    (void) vp;
fprintf(stderr, "preprocessing in %.3fs\n", time.elapsed());
    return xs;
}

using Pixel = std::tuple<float, float, float, float>;

// This is the other function you have to implement.
// It receives the acceleration datastructure and a sampling
// position. It returns the color at that position.
static Pixel sample(const Accelerated &accel, float x, float y) {
    (void) accel; (void) x; (void) y;
    // Implement your own;
    return Pixel(0.f,0.f,0.f,1.f);
}

// In theory, you don't have to change this function.
// It simply allocates the image, samples each pixel center,
// and saves the image into the file.
void render(const Accelerated &accel, const Viewport &vp, FILE *out,
    const std::vector<std::string> &args) {
    (void) args;
Chronos time;
    // Get viewport
    int xl, yb, xr, yt;
    std::tie(xl, yb) = vp.bl();
    std::tie(xr, yt) = vp.tr();
    int width = std::abs(xr-xl);
    int height = std::abs(yt-yb);
    int xmin = std::min(xl, xr);
    int ymin = std::min(yt, yb);
    // Allocate image
    rvg::image::Image<float, 4> img;
    img.resize(width, height);
    // Rendering loop
time.reset();
    for (int i = 0; i < height; ++i) {
        float y = static_cast<float>(ymin+i)+.5f;
fprintf(stderr, "\r%5g%%", std::floor(1000.f*(i+1)/height)/10.f);
        for (int j = 0; j < width; j++) {
            float x = static_cast<float>(xmin+j)+.5f;
            float r, g, b, a;
            std::tie(r, g, b, a) = sample(accel, x, y);
            img.set_pixel(j, i, r, g, b, a);
        }
    }
fprintf(stderr, "\n");
fprintf(stderr, "rendering in %.3fs\n", time.elapsed());
time.reset();
    rvg::image::pngio::store<uint8_t>(out, img);
fprintf(stderr, "saved in %.3fs\n", time.elapsed());
}

} } } // namespace rvg::driver::png

// This is the part that exposes the render and accelerate
// functions to Lua.

using rvg::driver::png::Accelerated;

// pushes a new userdata with with an Accelerated object
template <typename A>
typename std::enable_if<
    rvg::meta::forward_same_or_convertible<A, Accelerated>::value,
int>::type pushaccel(lua_State *L, A &&accel) {
    Accelerated *p = reinterpret_cast<Accelerated *>(
        lua_newuserdata(L, sizeof(Accelerated)));
    new (p) Accelerated(std::forward<A>(accel));
    lua_pushvalue(L, lua_upvalueindex(2));
    lua_setmetatable(L, -2);
    return 1;
}

// checks and returns an Accelerated object from a userdata
Accelerated checkaccel(lua_State *L, int idx) {
    idx = compat_abs_index(L, idx);
    if (!lua_getmetatable(L, idx)) lua_pushnil(L);
    if (!compat_is_equal(L, -1, lua_upvalueindex(2)))
        luaL_argerror(L, idx, "expected accelerated (png)");
    lua_pop(L, 1);
    return *reinterpret_cast<Accelerated *>(lua_touserdata(L, idx));
}

// Lua version of the rvg::driver::png::accelerate function
static int luaaccelerate(lua_State *L) {
   return pushaccel(L, rvg::driver::png::accelerate(
        rvg::description::lua::checkxformablescene(L, 1),
        rvg::description::lua::checkviewport(L, 2)));
}

// Lua version of the rvg::driver::png::render function
static int luarender(lua_State *L) {
    rvg::driver::png::render(
        checkaccel(L, 1),
        rvg::description::lua::checkviewport(L, 2),
        compat_check_file(L, 3),
        rvg::description::lua::optargs(L, 4));
    return 0;
}

// List of Lua functions exported into driver table
static const luaL_Reg modpng[] = {
    {"render", luarender },
    {"accelerate", luaaccelerate },
    {NULL, NULL}
};

// __gc metamethod for Accelerated userdata
static int gcaccel(lua_State *L) {
    Accelerated *p = reinterpret_cast<Accelerated*>(lua_touserdata(L, 1));
    p->~Accelerated();
    return 0;
}

// __tostring metamethod for Accelerated userdata
static int tostringaccel(lua_State *L) {
    Accelerated *p = reinterpret_cast<Accelerated*>(lua_touserdata(L, 1));
    lua_pushfstring(L, "accelerated (png): %p", p);
    return 1;
}

// Metamethods for Accelerated userdata
static const luaL_Reg metaccel[] = {
    {"__gc", gcaccel},
    {"__tostring", tostringaccel},
    {NULL, NULL}
};

// Lua function invoked to be invoked by require"driver.png"
extern "C"
#ifndef _WIN32
__attribute__((visibility("default")))
#else
__declspec(dllexport)
#endif
int luaopen_driver_cpp_png(lua_State *L) {
    // Create driver with all Lua functions needed to build
    // the scene description
    rvg::description::lua::newdriver(L); // driver mettab
    // initialize the Accelerated userdata type
    lua_newtable(L); // driver mettab metaccel
    lua_pushvalue(L, -2); // driver mettab metaccel mettab
    lua_pushvalue(L, -2); // driver mettab metaccel mettab metaccel
    compat_setfuncs(L, metaccel, 2); // driver mettab metaccel
    // add our accelereate and render functions to driver
    compat_setfuncs(L, modpng, 2); // driver
    return 1;
}
