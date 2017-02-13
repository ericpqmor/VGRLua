local _M = {}

local util = require"util"
local uint8_t_to_unorm = util.uint8_t_to_unorm

local color_meta = {}
color_meta.__index = {}
color_meta.name = "color"

local unpack = unpack or table.unpack

local function color_to_string(c)
    local r, g, b, a = unpack(c, 1, 4)
    if a < 1 then
        return string.format("rgba(%g,%g,%g,%g)", r, g, b, a)
    else
        return string.format("rgb(%g,%g,%g)", r, g, b)
    end
end

function color_meta.__tostring(self)
    return color_to_string(self)
end

function _M.rgb(r, g, b, a)
    a = a or 1.
    assert(type(r) == "number" and r <= 1 and r >= 0, "invalid red component")
    assert(type(g) == "number" and g <= 1 and g >= 0, "invalid green component")
    assert(type(b) == "number" and b <= 1 and b >= 0, "invalid blue component")
    assert(type(a) == "number" and a <= 1 and a >= 0, "invalid alpha component")
    return setmetatable({r, g, b, a}, color_meta)
end

_M.rgba = _M.rgb

function _M.rgb8(r, g, b, a)
    a = a or 255.
    assert(type(r) == "number" and r <= 255. and r >= 0., "invalid red component")
    assert(type(g) == "number" and g <= 255. and g >= 0., "invalid gren component")
    assert(type(b) == "number" and b <= 255. and b >= 0., "invalid blue component")
    assert(type(a) == "number" and a <= 255. and a >= 0., "invalid alpha component")
    return setmetatable({
        uint8_t_to_unorm(r),
        uint8_t_to_unorm(g),
        uint8_t_to_unorm(b),
        uint8_t_to_unorm(a)
    }, color_meta)
end

_M.rgba8 = _M.rgb8

_M.rgbx = function(s)
    local r, g, b
    local n = #s
    assert(n == 3 or n == 4 or n == 6 or n == 7,  "invalid hex color " .. s)
    if n == 3 or n == 4 then
        r, g, b = string.match(s, "^%s*[%#]?(%x)(%x)(%x)%s*$")
        assert(r and g and b, "invalid hex color " .. s)
        r = assert(tonumber(r, 16), "invalid red component")*17
        g = assert(tonumber(g, 16), "invalid green component")*17
        b = assert(tonumber(b, 16), "invalid blue component")*17
    else
        r, g, b = string.match(s, "^%s*[%#]?(%x%x)(%x%x)(%x%x)%s*$")
        assert(r and g and b, "invalid hex color " .. s)
        r = assert(tonumber(r, 16), "invalid red component")
        g = assert(tonumber(g, 16), "invalid green component")
        b = assert(tonumber(b, 16), "invalid blue component")
    end
    return _M.rgb8(r, g, b)
end

function _M.gray(v, a)
    a = a or 1.
    assert(type(v) == "number", "invalid value component")
    assert(type(a) == "number", "invalid alpha component")
    return setmetatable({v, v, v, a}, color_meta)
end

function _M.hsv(h, s, v, a)
    a = a or 1.
    if s > 0. then
        if h >= 1. then h = 0. end
        local p = math.floor(h)
        local f = h - p
        local m = v*(1.-s)
        local n = v*(1.-s*f)
        local k = v*(1.-s*(1.-f))
        if p == 0. then return setmetatable({v, k, m, a}, color_meta)
        elseif p == 1. then return setmetatable({n, v, m, a}, color_meta)
        elseif p == 2. then return setmetatable({m, v, k, a}, color_meta)
        elseif p == 3. then return setmetatable({m, n, v, a}, color_meta)
        elseif p == 4. then return setmetatable({k, m, v, a}, color_meta)
        elseif p == 5. then return setmetatable({v, m, n, a}, color_meta)
        else return setmetatable({0., 0., 0., a}, color_meta) end
    else
        return setmetatable({v, v, v, a}, color_meta)
    end
end

local rgb8 = _M.rgb8

function _M.is_color(c)
    return getmetatable(c) == color_meta
end

_M.named = {
    aliceblue = rgb8(240,248,255),
    antiquewhite = rgb8(250,235,215),
    aqua = rgb8(0,255,255),
    aquamarine = rgb8(127,255,212),
    azure = rgb8(240,255,255),
    beige = rgb8(245,245,220),
    bisque = rgb8(255,228,196),
    black = rgb8(0,0,0),
    blanchedalmond = rgb8(255,235,205),
    blue = rgb8(0,0,255),
    blueviolet = rgb8(138,43,226),
    brown = rgb8(165,42,42),
    burlywood = rgb8(222,184,135),
    cadetblue = rgb8(95,158,160),
    chartreuse = rgb8(127,255,0),
    chocolate = rgb8(210,105,30),
    coral = rgb8(255,127,80),
    cornflowerblue = rgb8(100,149,237),
    cornsilk = rgb8(255,248,220),
    crimson = rgb8(220,20,60),
    cyan = rgb8(0,255,255),
    darkblue = rgb8(0,0,139),
    darkcyan = rgb8(0,139,139),
    darkgoldenrod = rgb8(184,134,11),
    darkgray = rgb8(169,169,169),
    darkgreen = rgb8(0,100,0),
    darkgrey = rgb8(169,169,169),
    darkkhaki = rgb8(189,183,107),
    darkmagenta = rgb8(139,0,139),
    darkolivegreen = rgb8(85,107,47),
    darkorange = rgb8(255,140,0),
    darkorchid = rgb8(153,50,204),
    darkred = rgb8(139,0,0),
    darksalmon = rgb8(233,150,122),
    darkseagreen = rgb8(143,188,143),
    darkslateblue = rgb8(72,61,139),
    darkslategray = rgb8(47,79,79),
    darkslategrey = rgb8(47,79,79),
    darkturquoise = rgb8(0,206,209),
    darkviolet = rgb8(148,0,211),
    deeppink = rgb8(255,20,147),
    deepskyblue = rgb8(0,191,255),
    dimgray = rgb8(105,105,105),
    dimgrey = rgb8(105,105,105),
    dodgerblue = rgb8(30,144,255),
    firebrick = rgb8(178,34,34),
    floralwhite = rgb8(255,250,240),
    forestgreen = rgb8(34,139,34),
    fuchsia = rgb8(255,0,255),
    gainsboro = rgb8(220,220,220),
    ghostwhite = rgb8(248,248,255),
    gold = rgb8(255,215,0),
    goldenrod = rgb8(218,165,32),
    gray = rgb8(128,128,128),
    green = rgb8(0,128,0),
    greenyellow = rgb8(173,255,47),
    grey = rgb8(128,128,128),
    honeydew = rgb8(240,255,240),
    hotpink = rgb8(255,105,180),
    indianred = rgb8(205,92,92),
    indigo = rgb8(75,0,130),
    ivory = rgb8(255,255,240),
    khaki = rgb8(240,230,140),
    lavender = rgb8(230,230,250),
    lavenderblush = rgb8(255,240,245),
    lawngreen = rgb8(124,252,0),
    lemonchiffon = rgb8(255,250,205),
    lightblue = rgb8(173,216,230),
    lightcoral = rgb8(240,128,128),
    lightcyan = rgb8(224,255,255),
    lightgoldenrodyellow = rgb8(250,250,210),
    lightgray = rgb8(211,211,211),
    lightgreen = rgb8(144,238,144),
    lightgrey = rgb8(211,211,211),
    lightpink = rgb8(255,182,193),
    lightsalmon = rgb8(255,160,122),
    lightseagreen = rgb8(32,178,170),
    lightskyblue = rgb8(135,206,250),
    lightslategray = rgb8(119,136,153),
    lightslategrey = rgb8(119,136,153),
    lightsteelblue = rgb8(176,196,222),
    lightyellow = rgb8(255,255,224),
    lime = rgb8(0,255,0),
    limegreen = rgb8(50,205,50),
    linen = rgb8(250,240,230),
    magenta = rgb8(255,0,255),
    maroon = rgb8(128,0,0),
    mediumaquamarine = rgb8(102,205,170),
    mediumblue = rgb8(0,0,205),
    mediumorchid = rgb8(186,85,211),
    mediumpurple = rgb8(147,112,219),
    mediumseagreen = rgb8(60,179,113),
    mediumslateblue = rgb8(123,104,238),
    mediumspringgreen = rgb8(0,250,154),
    mediumturquoise = rgb8(72,209,204),
    mediumvioletred = rgb8(199,21,133),
    midnightblue = rgb8(25,25,112),
    mintcream = rgb8(245,255,250),
    mistyrose = rgb8(255,228,225),
    moccasin = rgb8(255,228,181),
    navajowhite = rgb8(255,222,173),
    navy = rgb8(0,0,128),
    oldlace = rgb8(253,245,230),
    olive = rgb8(128,128,0),
    olivedrab = rgb8(107,142,35),
    orange = rgb8(255,165,0),
    orangered = rgb8(255,69,0),
    orchid = rgb8(218,112,214),
    palegoldenrod = rgb8(238,232,170),
    palegreen = rgb8(152,251,152),
    paleturquoise = rgb8(175,238,238),
    palevioletred = rgb8(219,112,147),
    papayawhip = rgb8(255,239,213),
    peachpuff = rgb8(255,218,185),
    peru = rgb8(205,133,63),
    pink = rgb8(255,192,203),
    plum = rgb8(221,160,221),
    powderblue = rgb8(176,224,230),
    purple = rgb8(128,0,128),
    red = rgb8(255,0,0),
    rosybrown = rgb8(188,143,143),
    royalblue = rgb8(65,105,225),
    saddlebrown = rgb8(139,69,19),
    salmon = rgb8(250,128,114),
    sandybrown = rgb8(244,164,96),
    seagreen = rgb8(46,139,87),
    seashell = rgb8(255,245,238),
    sienna = rgb8(160,82,45),
    silver = rgb8(192,192,192),
    skyblue = rgb8(135,206,235),
    slateblue = rgb8(106,90,205),
    slategray = rgb8(112,128,144),
    slategrey = rgb8(112,128,144),
    snow = rgb8(255,250,250),
    springgreen = rgb8(0,255,127),
    steelblue = rgb8(70,130,180),
    tan = rgb8(210,180,140),
    teal = rgb8(0,128,128),
    thistle = rgb8(216,191,216),
    tomato = rgb8(255,99,71),
    turquoise = rgb8(64,224,208),
    violet = rgb8(238,130,238),
    wheat = rgb8(245,222,179),
    white = rgb8(255,255,255),
    whitesmoke = rgb8(245,245,245),
    yellow = rgb8(255,255,0),
    yellowgreen = rgb8(154,205,50),
}

local color_to_name = {}

for name, c in pairs(_M.named) do
    color_to_name[tostring(c)] = name
end

function _M.to_name(c)
    return color_to_name[color_to_string(c)]
end

return _M
