local _M = {}

local window_meta = {}
window_meta.__index = {}
window_meta.name = "window"

local unpack = unpack or table.unpack

function window_meta.__tostring(self)
    return string.format("window{%g,%g,%g,%g}", unpack(self, 1, 4))
end

function _M.window(xmin, ymin, xmax, ymax)
    return setmetatable({xmin, ymin, xmax, ymax}, window_meta)
end

function _M.is_window(w)
    return getmetatable(w) == window_meta
end

return _M
