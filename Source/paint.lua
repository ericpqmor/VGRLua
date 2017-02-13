local _M = { }

local xform = require"xform"
local xformable = require"xformable"
local color = require"color"
local ramp = require"ramp"
local image = require"image"
local spread = require"spread"

local util = require"util"
local is_almost_zero = util.is_almost_zero

local paint_meta = {}
paint_meta.__index = {}
paint_meta.name = "paint"

function paint_meta.__tostring(self)
    return string.format("paint{%s}", self.type)
end

-- ??D as an optimization, we should cache all solid colors
-- to prevent unecessary creation. to do that, we must cache
-- rgb colors as well

-- paints can be solid_color, linear_gradient, radial_gradient,
-- or texture.
-- all paints include a xform. this xform gives the inverse
-- mapping from scene coordinates to paint coordinates. the
-- reason we use the inverse is that the inverse gets
-- transformed by the same transformations as the shapes the
-- paints are attached to.

-- solid colors do not vary in space
-- transparency is modulated by the opacity in addition to the alpha channel
function _M.solid_color(rgba, opacity)
    opacity = opacity or 1.
    assert(color.is_color(rgba), "invalid color")
    return setmetatable({
        type = "solid_color",
        rgba = rgba,
        xf = xform.identity(),
        opacity = opacity
    }, paint_meta)
end

-- a texture paint contains an image
-- this image covers the area from [0,1] x [0,1]
-- the paint color is defined by sampling the image
-- the transparency is modulated by the global opacity
-- the xf is the *inverse* of the mapping from scene coordinates
-- to texture coordinates.
-- texure coordinates are wrapped with the spread.
-- only then is the texture sampled.
function _M.texture(sp, img, opacity)
    opacity = opacity or 1.
    assert(image.is_image(img), "invalid image")
    assert(type(opacity) == "number", "invalid opacity")
    return setmetatable({
        type = "texture",
        spread = sp,
        image = img,
        xf = xform.identity(),
        opacity = opacity
    }, paint_meta)
end

-- radial gradients define an offset used to sample a ramp.
-- the offset varies with the gradient point, which is obtained
-- from the scene point by applying inverse xform.
-- radial radients define a circle from center and radius
-- the ray from the focus to the gradient point intersects
-- the circle at the circle point.
-- the offset is given by the ratio between the lengths of
-- the vector connecting the focus to the gradient point and
-- the vector connecting the focus to the circle point.
-- the offset is wrapped with the spread
-- only then the ramp is sampled.
function _M.radial_gradient(rmp, cx, cy, fx, fy, r, opacity)
    opacity = opacity or 1.
    assert(ramp.is_ramp(rmp), "invalid ramp")
    assert(type(opacity) == "number", "invalid opacity")
    assert(type(cx) == "number", "invalid cx")
    assert(type(cy) == "number", "invalid cy")
    assert(type(fx) == "number", "invalid fx")
    assert(type(fy) == "number", "invalid fy")
    assert(type(r) == "number", "invalid r")
    -- radius zero degenerates to solid with last stop in ramp
    if is_almost_zero(r) then return _M.solid_color(rmp[#rmp], opacity) end
    return setmetatable({
        type = "radial_gradient",
        ramp = rmp,
        cx = cx,
        cy = cy,
        fx = fx,
        fy = fy,
        r = r,
        opacity = opacity,
        xf = xform.identity()
    }, paint_meta)
end

-- linear gradients define an offset used to sample a ramp.
-- the offset varies with the gradient point, which is obtained
-- from the scene point by applying inverse xform.
-- linear gradients define a vector from p1=(x1,y1) to p2=(x2,y2).
-- projection of the gradient point to the vector from p1 to p2 defines
-- a projection point.
-- the offset is the ratio of the (signed) lengths between the vector
-- connecting p1 to the projection point and the vector
-- connectin p1 to p2.
-- the offset is wrapped with the spread
-- only then the ramp is sampled.
function _M.linear_gradient(rmp, x1, y1, x2, y2, opacity)
    opacity = opacity or 1.
    assert(ramp.is_ramp(rmp), "invalid ramp")
    assert(type(x1) == "number", "invalid x1")
    assert(type(y1) == "number", "invalid y1")
    assert(type(x2) == "number", "invalid x2")
    assert(type(y2) == "number", "invalid y2")
    assert(type(opacity) == "number", "invalid opacity")
    return setmetatable({
        type = "linear_gradient",
        ramp = rmp,
        x1 = x1,
        y1 = y1,
        x2 = x2,
        y2 = y2,
        opacity = opacity,
        xf = xform.identity(),
    }, paint_meta)
end

-- checks if object is a paint
function _M.is_paint(p)
    return getmetatable(p) == paint_meta
end

-- make paints xformable
xformable.setmethods(paint_meta.__index)

return _M
