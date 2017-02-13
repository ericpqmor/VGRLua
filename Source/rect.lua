local _M = {}

local rect_meta = {}
rect_meta.__index = {}
rect_meta.name = "rect"

local xform = require"xform"
local xformable = require"xformable"
local strokable = require"strokable"
local path = require"path"
local command = require"command"

-- a rect is given by lower left corner, width and height
function _M.rect(x, y, width, height)
    return setmetatable({
        type = "rect",
        xf = xform.identity(),
        x = x, y = y,
        width = width, height = height
    }, rect_meta)
end

local M = command.M
local Z = command.Z

-- convert shape to a path, preserving the original xf in the shape
function rect_meta.__index.as_path(r, screen_xf)
    local xr, yr = r.x+r.width, r.y+r.height
    return path.path{
        M,
        r.x, r.y,
        xr, r.y,
        xr, yr,
        r.x, yr,
        Z
     }:transformed(r.xf)
end

function _M.is_rect(r)
    return getmetatable(r) == rect_meta
end

xformable.setmethods(rect_meta.__index)
strokable.setmethods(rect_meta.__index)

return _M
