local _M = {}

local ramp_meta = {}
ramp_meta.__index = {}
ramp_meta.name = "ramp"

local color = require"color"

-- the color ramp is an array of offset/color pairs
-- { t1, color1, t2, color2 ... tn colorn }
function ramp_meta.__tostring(self)
    return "ramp"
end

function _M.ramp(spread, stops)
    assert(type(stops) == "table", "expecting table")
    assert(#stops > 0, "empty ramp")
    local stops_copy = {}
    local last = -1
    for i,s in ipairs(stops) do
        assert(type(s[1]) == "number", "stop offset not a number")
        assert(s[1] >= last, "stop offset out of order")
        assert(color.is_color(s[2]), "invalid stop color")
        last = s[1]
        if last < 0. then last = 0. end
        if last > 1. then last = 1. end
        stops_copy[i] = {last, s[2]}
    end
    return setmetatable({
        type = "ramp",
        stops = stops_copy,
        spread = spread,
    }, ramp_meta)
end

function _M.is_ramp(r)
    return getmetatable(r) == ramp_meta
end

return _M
