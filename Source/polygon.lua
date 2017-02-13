local _M = {}

local polygon_meta = {}
polygon_meta.__index = {}
polygon_meta.name = "polygon"

local xform = require"xform"
local xformable = require"xformable"
local strokable = require"strokable"

local path = require"path"
local command = require"command"

-- the polygon data is an array with the coordinates of each vertex
-- { x1, y1, x2, y2, ... xn, yn }
-- it also has room for a xform to be applied to all vertices
function _M.polygon(original)
    local copy = {}
    assert(type(original) == "table", "expected table with coordinates")
    assert(#original % 2 == 0, "invalid number of coordinates in polygon")
    for i,v in ipairs(original) do
        assert(type(v) == "number", "coordinate " .. i .. " not a number")
        copy[i] = v
    end
    return setmetatable({
        type = "polygon",
        data = copy,
        xf = xform.identity()
    }, polygon_meta)
end

local M = command.M
local Z = command.Z

-- convert shape to a path, preserving the original xf in the shape
function polygon_meta.__index.as_path(polygon, screen_xf)
    local data = polygon.data
    local n = #data
    local p = { M }
    for i=1,n do
        p[i+1] = data[i]
    end
    p[n+2] = Z
    return path.path(p):transformed(polygon.xf)
end

function _M.is_polygon(p)
    return getmetatable(p) == polygon_meta
end

xformable.setmethods(polygon_meta.__index)
strokable.setmethods(polygon_meta.__index)

return _M
