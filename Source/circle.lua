local _M = { }

local circle_meta = {}
circle_meta.__index = {}
circle_meta.name = "circle"

local xform = require"xform"
local xformable = require"xformable"
local strokable = require"strokable"

local path = require"path"

function _M.circle(cx, cy, r)
    return setmetatable({
        type = "circle",
        cx = cx,
        cy = cy,
        r = r,
        xf = xform.identity()
    }, circle_meta)
end

local command = require"command"
local M = command.M
local R = command.R
local Z = command.Z

-- convert shape to a path, preserving the original xf in the shape
function circle_meta.__index.as_path(circle, screen_xf)
    -- transform unit circle to given center and radius
    local xf = xform.scaling(circle.r):translated(circle.cx, circle.cy)
    -- we start with a unit circle centered at the origin
    -- it is formed by 3 arcs covering each third of the unit circle
    -- then transform the control points accordingly
    local s = 0.5           -- sin(pi/6)
    local c = 0.86602540378 -- cos(pi/6)
    local w = s
    local x0, y0 = xf:apply(0., 1.)
    local x1, y1, w1 = xf:apply(-c,  s,  w)
    local x2, y2 = xf:apply(-c, -s)
    local x3, y3, w3 = xf:apply(0., -1.,  w)
    local x4, y4 = xf:apply(c, -s)
    local x5, y5, w5 = xf:apply(c,  s,  w)
    return path.path{
        M,  x0, y0,
        R,  x1, y1, w1, x2, y2,
        R,  x3, y3, w3, x4, y4,
        R,  x5, y5, w5, x0, y0,
        Z
    }:transformed(circle.xf)
end

function _M.iscircle(c)
    return getmetatable(c) == circle_meta
end

xformable.setmethods(circle_meta.__index)
strokable.setmethods(circle_meta.__index)

return _M
