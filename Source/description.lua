local _M = {}

local scene = require"scene"
local color = require"color"
local paint = require"paint"
local strokestyle = require"strokestyle"
local xform = require"xform"
local xformable = require"xformable"
local svd = require"svd"

local shape_types = {
    triangle = true,
    path = true,
    circle = true,
    rect = true,
    stroked = true,
    polygon = true,
}

local function is_shape(s)
    return shape_types[s.type]
end

local painted_meta = { __index = {} }
local stencil_meta = { __index = {} }

function new_primitive_painted(winding_rule)
    return function(s, p)
        assert(is_shape(s), "expected shape")
        if color.is_color(p) then
            p = paint.solid_color(p)
        end
        assert(paint.is_paint(p), "expected paint")
        return setmetatable({
            type = "primitive",
            winding_rule = winding_rule,
            shape = s,
            paint = p,
            xf = xform.identity()
        }, painted_meta)
    end
end

_M.nzfill = new_primitive_painted("non-zero")
_M.fill = _M.nzfill
_M.zfill = new_primitive_painted("zero")
_M.ofill = new_primitive_painted("odd")
_M.eofill = _M.ofill
_M.efill = new_primitive_painted("even")

function new_primitive_stencil(winding_rule)
    return function(s)
        assert(is_shape(s), "expected shape")
        return setmetatable({
            type = "primitive",
            winding_rule = winding_rule,
            shape = s,
            xf = xform.identity()
        }, stencil_meta)
    end
end

_M.nzstencil = new_primitive_stencil("non-zero")
_M.stencil = _M.nzstencil
_M.zstencil = new_primitive_stencil("zero")
_M.ostencil = new_primitive_stencil("odd")
_M.eostencil = _M.ostencil
_M.estencil = new_primitive_stencil("even")

local function is_painted(p)
    return getmetatable(p) == painted_meta
end

local function is_painted_or_list(p)
    if is_painted(p) then
        return true
    else
        if type(p) ~= "table" or #p < 1 then
            return false
        end
        for i,v in ipairs(p) do
            if not is_painted(v) then
                return false
            end
        end
    end
    return true
end

local function is_stencil(s)
    return getmetatable(s) == stencil_meta
end

local function is_stencil_or_list(s)
    if is_stencil(s) then
        return true
    else
        if type(s) ~= "table" or #s < 1 then
            return false
        end
        for i,v in ipairs(s) do
            if not is_stencil(v) then
                return false
            end
        end
    end
    return true
end

function _M.blur(radius, painted)
    if is_painted_or_list(painted) then
        return setmetatable({
            type = "blurred",
            radius = radius,
            painted = painted,
            xf = xform.identity()
        }, painted_meta)
    else
        error("expected painted")
    end
end

function _M.fade(opacity, painted)
    if is_painted_or_list(painted) then
        return setmetatable({
            type = "faded",
            opacity = opacity,
            painted = painted,
            xf = xform.identity()
        }, painted_meta)
    else
        error("expected painted")
    end
end

function _M.transform(xf, p_or_s)
    if is_stencil_or_list(p_or_s) then
        return setmetatable({
            type = "xformed",
            stencil = p_or_s,
            xf = xf
        }, stencil_meta)
    elseif is_painted_or_list(p_or_s) then
        return setmetatable({
            type = "xformed",
            painted = p_or_s,
            xf = xf
        }, painted_meta)
    else
        error("expected painted or stencil")
    end
end

function _M.clip(clipper, clippee)
    if not is_stencil_or_list(clipper) then
        error("expected stencil clipper")
    end
    if is_stencil_or_list(clippee) then
        return setmetatable({
            type = "clipped",
            clipper = clipper,
            clippee = clippee,
            xf = xform.identity()
        }, stencil_meta)
    elseif is_painted_or_list(clippee) then
        return setmetatable({
            type = "clipped",
            clipper = clipper,
            clippee = clippee,
            xf = xform.identity()
        }, painted_meta)
    else
        error("expected painted or stencil clippee")
    end
end

local function build_stencil(stencil_list, depths, forward)
    if is_stencil(stencil_list) then
        stencil_list = { stencil_list }
    end
    for i,st in ipairs(stencil_list) do
        assert(is_stencil(st), "expected stencil")
        if st.type == "primitive" then
            forward:stencil_element(
                st.winding_rule,
                st.shape:transformed(st.xf))
        else
            -- all stencil objects have their own xf
            -- if the xf is not the identity, we bracket
            -- around it with a transform bracket
            local xformed = not st.xf:is_identity()
            if xformed then
                forward:begin_transform(depths.xform, st.xf)
                depths.xform = depths.xform + 1
            end
            if st.type == "xformed" then
                -- already bracketed outside
                build_stencil(st.stencil, depths, forward)
            elseif st.type == "clipped" then
                forward:begin_clip(depths.clip)
                depths.clip = depths.clip + 1
                build_stencil(st.clipper, depths, forward)
                forward:activate_clip(depths.clip-1)
                build_stencil(st.clippee, depths, forward)
                depths.clip = depths.clip - 1
                forward:end_clip(depths.clip)
            end
            if xformed then
                depths.xform = depths.xform - 1
                forward:end_transform(depths.xform, st.xf)
            end
        end
    end
end

local function build_painted(painted_list, depths, forward)
    if is_painted(painted_list) then
        painted_list = { painted_list }
    end
    for i, p in ipairs(painted_list) do
        assert(is_painted(p), "expected painted")
        if p.type == "primitive" then
            forward:painted_element(
                p.winding_rule,
                p.shape:transformed(p.xf),
                p.paint:transformed(p.xf))
        else
            local xformed = not p.xf:is_identity()
            if xformed then
                forward:begin_transform(depths.xform, p.xf)
                depths.xform = depths.xform + 1
            end
            if p.type == "faded" then
                forward:begin_fade(depths.fade, p.opacity)
                depths.fade = depths.fade + 1
                build_painted(p.painted, depths, forward)
                depths.fade = depths.fade - 1
                forward:end_fade(depths.fade, p.opacity)
            elseif p.type == "xformed" then
                -- already brackted outside
                build_painted(p.painted, depths, forward)
            elseif p.type == "blurred" then
                forward:begin_blur(depths.blur, p.radius)
                depths.blur = depths.blur + 1
                build_painted(p.painted, depths, forward)
                depths.blur = depths.blur - 1
                forward:end_blur(depths.blur, p.radius)
            elseif p.type == "clipped" then
                forward:begin_clip(depths.clip)
                depths.clip = depths.clip + 1
                build_stencil(p.clipper, depths, forward)
                forward:activate_clip(depths.clip-1)
                build_painted(p.clippee, depths, forward)
                depths.clip = depths.clip - 1
                forward:end_clip(depths.clip)
            end
            if xformed then
                depths.xform = depths.xform - 1
                forward:end_transform(depths.xform, p.xf)
            end
        end
    end
end

function _M.scene(painted_list)
    local forward = scene.scene()
    local depths = { blur = 1, fade = 1, clip = 1, xform = 1 }
    build_painted(painted_list, depths, forward)
    return forward
end

function _M.stroke(width_or_style)
    return strokestyle.default():stroked(width_or_style)
end

xformable.setmethods(painted_meta.__index)
xformable.setmethods(stencil_meta.__index)

return _M
