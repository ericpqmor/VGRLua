local _M = {}

local triangle_meta = {}
triangle_meta.__index = {}
triangle_meta.name = "triangle"

local xform = require"xform"
local xformable = require"xformable"
local strokable = require"strokable"
local path = require"path"
local command = require"command"

-- a triangle is given by its three vertices in the obvious way
-- it also holds a xform to be applied to all vertices
function _M.triangle(x1, y1, x2, y2, x3, y3)
    return setmetatable({
        type = "triangle",
        xf = xform.identity(),
        x1 = x1, y1 = y1,
        x2 = x2, y2 = y2,
        x3 = x3, y3 = y3,
    }, triangle_meta)
end

local M = command.M
local Z = command.Z

-- convert shape to a path, preserving the original xf in the shape
function triangle_meta.__index.as_path(t, screen_xf)
    return path.path{M, t.x1, t.y1, t.x2, t.y2, t.x3, t.y3, Z}:transformed(t.xf)
end

function _M.is_triangle(t)
    return getmetatable(t) == triangle_meta
end

xformable.setmethods(triangle_meta.__index)
strokable.setmethods(triangle_meta.__index)

return _M
