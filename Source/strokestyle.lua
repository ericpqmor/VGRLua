local _M = {}

local strokestyle_meta = {}
strokestyle_meta.__index = {}
strokestyle_meta.name = "strokestyle"

_M.cap = {
    butt = "butt",
    round = "round",
    square = "square",
    triangle = "triangle",
    fletching = "fletching"
}

_M.join = {
    arcs = "arcs",
    miter = "miter",
    miterclip = "miterclip",
    round = "round",
    bevel = "bevel"
}

_M.method = {
   driver = "driver",
   curves = "curves",
   lines = "lines"
}

_M.default_cap = _M.cap.butt
_M.default_join = _M.join.miter
_M.default_width = 0.
_M.default_miter_limit = 4.
_M.default_phase_reset = false
_M.default_initial_phase = 0.
_M.default_method = "driver"
_M.default_dash_array = {}

local _default = setmetatable({
    type = "strokestyle",
    join = _M.default_join,
    cap = _M.default_cap,
    width = _M.default_width,
    phase_reset = _M.default_phase_reset,
    dash_array = _M.default_dash_array,
    miter_limit = _M.default_miter_limit,
    method = _M.default_method,
    initial_phase = _M.default_initial_phase,
}, strokestyle_meta)

function _M.default()
    return _default
end

local function copystyle(st)
    local copy = {}
    for i,v in pairs(st) do
        copy[i] = v
    end
    return setmetatable(copy, strokestyle_meta)
end

function strokestyle_meta.__index.stroked(style, width_or_style)
    assert(type(width_or_style) == "number" or
        _M.is_stroke_style(width_or_style))
    if _M.is_stroke_style(width_or_style) then
        return width_or_style
    end
    local copy = copystyle(style)
    copy.width = width_or_style
    return copy
end

function strokestyle_meta.__index.capped(style, cap)
    local copy = copystyle(style)
    copy.cap = assert(_M.cap[cap], "invalid cap")
    return copy
end

function strokestyle_meta.__index.joined(style, join, miter_limit)
    local copy = copystyle(style)
    copy.join = assert(_M.join[join], "invalid join")
    assert(miter_limit == nil or type(miter_limit) == "number", "invalid miter_limit")
    copy.miter_limit = miter_limit or _M.default_miter_limit
    return copy
end

function strokestyle_meta.__index.dashed(style, dash_array, initial_phase, phase_reset)
    local copy = copystyle(style)
    copy.dash_array = dash_array
    assert(initial_phase == nil or type(initial_phase) == "number", "invalid initial_phase")
    copy.initial_phase = initial_phase or _M.default_initial_phase
    assert(phase_reset == nil or type(phase_reset) == "boolean", "invalid phase_reset")
    if phase_reset ~= nil then
        copy.phase_reset = phase_reset
    end
    return copy
end

function strokestyle_meta.__index.by(style, method)
    local copy = copystyle(style)
    copy.method = method
    return copy
end

function _M.is_stroke_style(s)
    return getmetatable(s) == strokestyle_meta
end

return _M
