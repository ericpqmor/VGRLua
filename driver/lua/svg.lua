local driver = require"driver"
local xform = require"xform"
local arc = require"arc"
local filter = require"filter"
local util = require"util"
local color = require"color"
local image = require"image"
local svd = require"svd"
local indent = require"indent"
local base64 = require"base64"
local unorm_to_uint8_t = util.unorm_to_uint8_t
local is_almost_zero = util.is_almost_zero
local is_almost_equal = util.is_almost_equal
local is_almost_one = util.is_almost_one
local path = require"path"
local EMPTY_PATH = path.path()

local function stderr(...)
    io.stderr:write(string.format(...))
end

local _M = driver.new()

local unpack = unpack or table.unpack
local abs = math.abs
local min = math.min
local max = math.max
local atan2 = math.atan2 or math.atan

local function chop(v)
    if is_almost_zero(v) then return 0
    else return v end
end

-- translate RVG instructions to SVG commands
local function newsvgpathwriter(file)
    local args = {}
    local lastcmd = nil

    local function writecmd(cmd)
        if cmd ~= lastcmd then
            file:write(cmd, " ")
            lastcmd = cmd
        end
    end

    function args:begin_closed_contour(len, x0, y0)
        writecmd("M")
        file:write(chop(x0), " ", chop(y0), " ")
        -- lastcmd = "L" -- omit L after M
    end

    args.begin_open_contour = args.begin_closed_contour

    function args:end_closed_contour(x0, y0, len)
        writecmd("Z")
    end

    function args:end_open_contour(x0, y0, len)
    end

    function args:linear_segment(x0, y0, x1, y1)
        writecmd("L")
        file:write(chop(x1), " ", chop(y1), " ")
    end

    function args:quadratic_segment(x0, y0, x1, y1, x2, y2)
        writecmd("Q")
        file:write(chop(x1), " ", chop(y1), " ", chop(x2), " ", chop(y2), " ")
    end

    function args:rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2)
        local rx, ry, rot_deg, fa, fs = arc.tosvg(x0, y0, x1, y1, w1, x2, y2)
        if rx then
            writecmd("A")
            file:write(chop(rx), " ", chop(ry), " ", chop(rot_deg), " ",
                fa, " ", fs, " ")
            file:write(chop(x2), " ", chop(y2), " ")
        else -- degenerate to parabola
            writecmd("Q")
            file:write(chop(x1), " ", chop(y1), " ",
                chop(x2), " ", chop(y2), " ")
        end
    end

    function args:cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3)
        writecmd("C")
        file:write(chop(x1), " ", chop(y1), " ", chop(x2), " ", chop(y2), " ",
            chop(x3), " ", chop(y3), " ")
    end

    function args:degenerate_segment(x0, y0, dx0, dy0, dx1, dy1, x1, y1)
        writecmd("L")
        file:write(chop(x1), " ", chop(y1), " ")
    end

    return args
end

local function print_path_data(p, file, opt_pre_xf)
    local previous = ""
    file:write(' d="')
    if not opt_pre_xf or opt_pre_xf:is_identity() then
        p:iterate(newsvgpathwriter(file))
    else
        p:iterate(filter.xform(opt_pre_xf, newsvgpathwriter(file)))
    end
    file:write('"')
end

local function factorxf(xf)
    local a, b, tx, c, d, ty = unpack(xf, 1, 6)
    local L = xform.linear(a,b,c,d)
    local U,S,V = svd.usv(L)
    local cos_U = U[1]
    local sin_U = U[4]
    local ang_U = math.deg(atan2(sin_U, cos_U))
    local sx = S[1]
    local sy = S[5]
    if V:det() < 0 then
        sy = -sy
    end
    local cos_V = V[1]
    local sin_V = V[4]
    local ang_V = -math.deg(atan2(sin_V, cos_V))
    return tx, ty, ang_U, sx, sy, ang_V
end

local function print_xform(xf, name, file)
    local tx, ty, ang_U, sx, sy, ang_V = factorxf(xf)
    local t = ""
    if not is_almost_zero(ang_V) then
        t = string.format("rotate(%g)", ang_V)
    end
    if is_almost_equal(sx, sy) then
        if not is_almost_one(sx) then
            t = string.format("scale(%g)", sx) .. t
        end
    else
        t = string.format("scale(%g,%g)", sx, sy) .. t
    end
    if not is_almost_zero(ang_U) then
        t = string.format("rotate(%g)", ang_U) .. t
    end
    if not is_almost_zero(tx) or not is_almost_zero(ty) then
        if is_almost_zero(ty) then
            t = string.format("translate(%g)", tx) .. t
        else
            t = string.format("translate(%g,%g)", tx, ty) .. t
        end
    end
    if t ~= "" then
        file:write(name, '="', t, '"')
    end
end

local function print_stroke_style(s, file)
    if s.width ~= 0. then
        file:write(string.format(' stroke-width="%g"', s.width))
    end -- default is 1
    if s.cap and s.cap ~= _M.cap.butt then
        file:write(' stroke-linecap="', s.cap, '"')
    end
    if s.join and s.join ~= _M.join.miter then
        file:write(' stroke-linejoin="', s.join, '"')
    end
    if s.miter_limit and s.miter_limit ~= 4 then
        file:write(string.format(' stroke-miterlimit="%g"', s.miter_limit))
    end -- default is 4
    if #s.dash_array > 0 then
        if s.initial_phase and s.initial_phase ~= 0 then
            file:write(string.format(' stroke-dashoffset="%g"',
                s.initial_phase))
        end -- default is 0
        file:write(' stroke-dasharray="',
            table.concat(s.dash_array, " "), '"')
    end
end

local function print_ramp(ramp, ind, file)
    for i, stop in ipairs(ramp.stops) do
        local r, g, b, a = unpack(stop[2], 1, 4)
        local s = color.to_name(stop[2])
        if not s then
            s = string.format("rgb(%d,%d,%d)",
                unorm_to_uint8_t(r), unorm_to_uint8_t(g), unorm_to_uint8_t(b))
        end
        ind:write(file)
        file:write(string.format('<stop offset="%g" stop-color="%s"',
            stop[1], s))
        if a < 1 then
            file:write(string.format(' stop-opacity="%g"', a))
        end
        file:write('/>')
    end
end

local accepted_ramp_spread = {
    ["repeat"] = true,
    ["reflect"] = true,
    ["pad"] = true
}

local accepted_texture_spread = {
    ["repeat"] = true
}

local svg_winding_rule_name = {
    ["non-zero"] = "nonzero",
    ["zero"] = "zero",
    ["odd"] = "evenodd",
    ["even"] = "even",
}

local function newpaintstencilprinter(ind, map, screen_xf, file)
    local sceneprinter = { }
    local blur_id = 0
    local stencil_id = 0
    local gradient_id = 0
    local texture_id = 0
    local clippath_id = 0
    local active_clips = {}
    local not_yet_active_clips = {}
    local stencil_xf_stack = {}
    local stencil_xf = xform.identity()

    function sceneprinter.print_linear_gradient(self, shape, paint)
        if not map[paint] then
            ind:write_inc(file)
            file:write('<linearGradient id="gradient', gradient_id,
                '" gradientUnits="userSpaceOnUse"')
            file:write(string.format(' x1="%g" y1="%g" x2="%g" y2="%g"',
                paint.x1, paint.y1, paint.x2, paint.y2))
            if not accepted_ramp_spread[paint.ramp.spread] then
                stderr("ramp spread %s is not supported by SVG\n",
                    tostring(paint.ramp.spread))
            end
            file:write(string.format(' spreadMethod="%s"', paint.ramp.spread))
            print_xform(paint.xf:transformed(shape.xf:inverse()),
                ' gradientTransform', file)
            file:write('>')
            print_ramp(paint.ramp, ind, file)
            ind:dec_write(file)
            file:write('</linearGradient>')
            map[paint] = gradient_id
            gradient_id = gradient_id + 1
        end
    end

    function sceneprinter.print_radial_gradient(self, shape, paint)
        if not map[paint] then
            ind:write_inc(file)
            file:write('<radialGradient id="gradient', gradient_id,
                '" gradientUnits="userSpaceOnUse"')
            file:write(string.format(' cx="%g" cy="%g" fx="%g" fy="%g" r="%g"',
                paint.cx, paint.cy, paint.fx, paint.fy, paint.r))
            if not accepted_ramp_spread[paint.ramp.spread] then
                stderr("ramp spread %s is not supported by SVG\n",
                    tostring(paint.ramp.spread))
            end
            file:write(string.format(' spreadMethod="%s"', paint.ramp.spread))
            print_xform(paint.xf:transformed(shape.xf:inverse()),
                ' gradientTransform', file)
            file:write('>')
            print_ramp(paint.ramp, ind, file)
            ind:dec_write(file)
            file:write('</radialGradient>')
            map[paint] = gradient_id
            gradient_id = gradient_id + 1
        end
    end

    function sceneprinter.print_texture(self, shape, paint)
        if not map[paint] then
            ind:write_inc(file)
            if not accepted_texture_spread[paint.spread] then
                stderr("texture spread %s is not supported by SVG\n",
                    tostring(paint.spread))
            end
            file:write('<pattern id="texture', texture_id, '" patternUnits="userSpaceOnUse" width="1" height="1" preserveAspectRatio="none"')
            print_xform(paint.xf:transformed(shape.xf:inverse()),
                " patternTransform", file)
            file:write('>')
            ind:write(file)
            file:write('<image id="image', texture_id, '" width="1" height="1" preserveAspectRatio="none" transform="scale(1,-1) translate(0,-1)" xlink:href="data:image/png;base64,\n')
            if paint.image.channel_type == "uint8_t" then
                file:write(base64.encode(image.png.string8(paint.image)))
            else
                file:write(base64.encode(image.png.string16(paint.image)))
            end
            file:write('"/>')
            ind:dec_write(file)
            file:write('</pattern>')
            map[paint] = texture_id
            texture_id = texture_id + 1
        end
    end

    function sceneprinter.painted_element(self, winding_rule, shape, paint)
        if paint.type == "linear_gradient" then
            self:print_linear_gradient(shape, paint)
        elseif paint.type == "radial_gradient" then
            self:print_radial_gradient(shape, paint)
        elseif paint.type == "texture" then
            self:print_texture(shape, paint)
        end
    end

    function sceneprinter.stencil_element(self, winding_rule, shape)
        ind:write(file)
        file:write('<path id="stencil', stencil_id, '" clip-rule="',
            svg_winding_rule_name[winding_rule] or "unknown", '"')
        -- convert to path preserving xf
        local path_shape = shape:as_path(screen_xf)
        print_path_data(path_shape, file)
        print_xform(shape.xf:transformed(stencil_xf), ' transform', file)
        file:write('/>')
        stencil_id = stencil_id+1
    end

    function sceneprinter.begin_clip(self, depth)
        not_yet_active_clips[#not_yet_active_clips+1] = clippath_id
        clippath_id = clippath_id + 1
    end

    function sceneprinter.activate_clip(self, depth)
        active_clips[#active_clips+1] =
            not_yet_active_clips[#not_yet_active_clips]
        not_yet_active_clips[#not_yet_active_clips] = nil
    end

    function sceneprinter.end_clip(self, depth)
        active_clips[#active_clips] = nil
    end

    function sceneprinter.begin_blur(self, depth, radius)
        if not is_almost_zero(radius) and not map[radius] then
            ind:write_inc(file)
            file:write('<filter id="blur', blur_id,
                '">')
                --'" filterUnits="userSpaceOnUse">')
            ind:write(file)
            file:write('<feGaussianBlur stdDeviation="', radius, '"/>')
            ind:dec_write(file)
            file:write('</filter>')
            map[radius] = blur_id
            blur_id = blur_id + 1
        end
    end

    function sceneprinter.end_blur(self, depth, radius)
    end

    function sceneprinter.begin_fade(self, depth, opacity)
    end

    function sceneprinter.end_fade(self, depth, opacity)
    end

    function sceneprinter.begin_transform(self, depth, xf)
        -- if the transformation happened within a stencil
        -- definition, we accumulate it
        if #not_yet_active_clips > 0 then
            stencil_xf_stack[#stencil_xf_stack+1] = stencil_xf
            stencil_xf = xf:transformed(stencil_xf)
        end
    end

    function sceneprinter.end_transform(self, depth, xf)
        if #not_yet_active_clips > 0 then
            stencil_xf = stencil_xf_stack[#stencil_xf_stack]
            stencil_xf_stack[#stencil_xf_stack] = nil
        end
    end

    return sceneprinter
end

local function newclipprinter(ind, map, screen_xf, file)
    local sceneprinter = { }
    local stencil_id = 0
    local clippath_id = 0
    local active_clips = {}
    local not_yet_active_clips = {}
    local nested_clips = {}

    function sceneprinter.painted_element(self, winding_rule, shape, paint)
    end

    function sceneprinter.stencil_element(self, winding_rule, shape)
        ind:write(file)
        file:write('<use xlink:href="#stencil', stencil_id, '"')
        if #nested_clips > 0 then
            file:write(' clip-path="url(#clip',
                nested_clips[#nested_clips], ')"')
        end
        file:write('/>')
        stencil_id = stencil_id+1
    end

    function sceneprinter.begin_clip(self, depth)
        ind:write_inc(file)
        file:write('<clipPath id="clip', clippath_id, '">')
        not_yet_active_clips[#not_yet_active_clips+1] = clippath_id
        clippath_id = clippath_id + 1
    end

    function sceneprinter.activate_clip(self, depth)
        ind:dec_write(file)
        file:write('</clipPath>')
        active_clips[#active_clips+1] =
            not_yet_active_clips[#not_yet_active_clips]
        not_yet_active_clips[#not_yet_active_clips] = nil
        if #not_yet_active_clips > 0 then
            nested_clips[#nested_clips+1] = active_clips[#active_clips]
        end
    end

    function sceneprinter.end_clip(self, depth)
        if #nested_clips > 0 and
            nested_clips[#nested_clips] == active_clips[#active_clips] then
            nested_clips[#nested_clips] = nil
        end
        active_clips[#active_clips] = nil
    end

    function sceneprinter.begin_blur(self, depth, radius)
    end

    function sceneprinter.end_blur(self, depth, radius)
    end

    function sceneprinter.begin_fade(self, depth, opacity)
    end

    function sceneprinter.end_fade(self, depth, opacity)
    end

    function sceneprinter.begin_transform(self, depth, xf)
    end

    function sceneprinter.end_transform(self, depth, xf)
    end

    return sceneprinter
end

local function newsceneprinter(ind, map, screen_xf, file)
    local sceneprinter = { }
    local shape_id = 0
    local clippath_id = 0
    local active_clips = {}
    local not_yet_active_clips = {}

    function sceneprinter.print_paint(self, paint, mode)
        if paint.type == "solid_color" then
            local r, g, b, a = unpack(paint.rgba, 1, 4)
            a = a*paint.opacity
            local s = color.to_name(paint.rgba)
            if not s then
                s = string.format("rgb(%d,%d,%d)",
                    unorm_to_uint8_t(r), unorm_to_uint8_t(g), unorm_to_uint8_t(b))
            end
            file:write(string.format(' %s="%s"', mode, s))
            if a < 1 then
                file:write(string.format(' %s-opacity="%g"', mode, a))
            end
        elseif paint.type == "linear_gradient" or
               paint.type == "radial_gradient" then
            file:write(string.format(' %s="url(#gradient%d)"', mode, map[paint]))
            if paint.opacity < 1 then
                file:write(string.format(' %s-opacity="%g"', mode, paint.opacity))
            end
        elseif paint.type == "texture" then
            file:write(string.format(' %s="url(#texture%d)"', mode, map[paint]))
            if paint.opacity < 1 then
                file:write(string.format(' %s-opacity="%g"', mode, paint.opacity))
            end
        else
            error("invalid paint type " .. tostring(paint.type))
        end
    end

    function sceneprinter.painted_element(self, winding_rule, shape, paint)
        local mode, stroke_style
        -- In SVG, there is no way to specify that a shape
        -- should be transformed *before* it is stroked
        -- We can only ask that it be transformed *after* it is stroked.
        -- So we have to do our own pre-transformation by hand
        local post_xf = shape.xf
        local pre_xf = nil
        local path_shape = nil

        if shape.type == "stroked" and shape.style.method == "driver" then
            mode = "stroke"
            stroke_style = shape.style
            pre_xf = shape.shape.xf
            -- convert to path the shape that is to be stroked
            -- preserving its original xf
            path_shape = shape.shape:as_path(shape.xf:transformed(screen_xf))
        else
            mode = "fill"
            -- convert to path preserving xf
            path_shape = shape:as_path(screen_xf)
        end
        ind:write(file)
        file:write('<path id="shape', shape_id, '" fill-rule="',
            svg_winding_rule_name[winding_rule] or "unknown", '"')
        if stroke_style then
            file:write(' fill="none"')
            print_stroke_style(stroke_style, file)
        end
        print_xform(post_xf, ' transform', file)
        self:print_paint(paint, mode)
        print_path_data(path_shape, file, pre_xf)
        file:write('/>')
        shape_id = shape_id+1

    end

    function sceneprinter.stencil_element(self, winding_rule, shape)
        -- all stencil elements are in inside the <defs> section
    end

    function sceneprinter.begin_clip(self, depth)
        not_yet_active_clips[#not_yet_active_clips+1] = clippath_id
        clippath_id = clippath_id + 1
    end

    function sceneprinter.activate_clip(self, depth)
        active_clips[#active_clips+1] =
            not_yet_active_clips[#not_yet_active_clips]
        not_yet_active_clips[#not_yet_active_clips] = nil
        if #not_yet_active_clips == 0 then
            ind:write_inc(file)
            file:write('<g clip-path="url(#clip',
                active_clips[#active_clips], ')">')
        end
    end

    function sceneprinter.end_clip(self, depth)
        active_clips[#active_clips] = nil
        if #not_yet_active_clips == 0 then
            ind:dec_write(file)
            file:write('</g>')
        end
    end

    function sceneprinter.begin_blur(self, depth, radius)
        if not is_almost_zero(radius) then
            ind:write_inc(file)
            file:write('<g filter="url(#blur', map[radius], ')">')
        end
    end

    function sceneprinter.end_blur(self, depth, radius)
        if not is_almost_zero(radius) then
            ind:dec_write(file)
            file:write('</g>')
        end
    end

    function sceneprinter.begin_fade(self, depth, opacity)
        if opacity < 1 then
            ind:write_inc(file)
            file:write('<g opacity="', opacity, '">')
        end
    end

    function sceneprinter.end_fade(self, depth, opacity)
        if opacity < 1 then
            ind:dec_write(file)
            file:write('</g>')
        end
    end

    function sceneprinter.begin_transform(self, depth, xf)
        if #not_yet_active_clips == 0 then
            ind:write_inc(file)
            file:write('<g')
            print_xform(xf, ' transform', file)
            file:write('>')
        end
    end

    function sceneprinter.end_transform(self, depth, xf)
        if #not_yet_active_clips == 0 then
            ind:dec_write(file)
            file:write('</g>')
        end
    end

    return sceneprinter
end

-- define acceleration function does nothing.
-- returns scene unchanged
function _M.accelerate(scene, viewport)
    return scene
end

-- define rendering function
function _M.render(scene, viewport, file)
    local vxmin, vymin, vxmax, vymax = unpack(viewport, 1, 4)
    local width, height = abs(vxmax-vxmin), abs(vymax-vymin)
    local xmin, ymin = min(vxmin, vxmax), min(vymin, vymax)
    file:write(string.format(
[[<?xml version="1.0" standalone="no"?>
<svg
  xmlns:xlink="http://www.w3.org/1999/xlink"
  xmlns:dc="http://purl.org/dc/elements/1.1/"
  xmlns:cc="http://creativecommons.org/ns#"
  xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
  xmlns:svg="http://www.w3.org/2000/svg"
  xmlns="http://www.w3.org/2000/svg"
  version="1.1"
  width="%d" height="%d"
  viewBox="%d %d %d %d">]], width, height, xmin, ymin, width, height))
    local flip = xform.translation(0,-vymin):scaled(1,-1):translated(0,vymax)
    local screen_xf = scene.xf:transformed(flip)
    local ind = indent.indent(1, "  ")
    local map = {}
    ind:write_inc(file)
    file:write('<defs>')
    -- write stencil shapes, gradient paints, and textures
    scene:iterate(newpaintstencilprinter(ind, map, screen_xf, file))
    -- write clip-paths
    scene:iterate(newclipprinter(ind, map, screen_xf, file))
    ind:dec_write(file)
    file:write('</defs>')
    ind:write_inc(file)
    file:write('<g')
    print_xform(flip, " transform", file)
    file:write('> <!-- invert y -->')
    ind:write_inc(file)
    file:write('<g')
    print_xform(scene.xf, " transform", file)
    file:write('> <!-- window-viewport -->')
    -- write painted shapes
    scene:iterate(newsceneprinter(ind, map, screen_xf, file))
    ind:dec_write(file)
    file:write("</g>")
    ind:dec_write(file)
    file:write("</g>")
    file:write("\n</svg>\n")
end

return _M
