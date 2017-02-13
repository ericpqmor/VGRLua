local _M = {}

local viewport_meta = {}
viewport_meta.__index = {}
viewport_meta.name = "viewport"

local unpack = unpack or table.unpack

function viewport_meta.__tostring(self)
    return string.format("viewport{%d,%d,%d,%d}", unpack(self, 1, 4))
end

function _M.viewport(xmin, ymin, xmax, ymax)
    xmin = math.floor(xmin+.5)
    ymin = math.floor(ymin+.5)
    xmax = math.floor(xmax+.5)
    ymax = math.floor(ymax+.5)
    return setmetatable({xmin, ymin, xmax, ymax}, viewport_meta)
end

function _M.is_viewport(v)
    return getmetatable(v) == viewport_meta
end

return _M
