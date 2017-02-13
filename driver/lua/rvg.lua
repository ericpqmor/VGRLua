-- ??D we should optionally identify repeated paths
-- (based on some kind of hash) to define them only once
-- and reuse them many times.
local driver = require"driver"
local xform = require"xform"
local indent = require"indent"
local base64 = require"base64"
local image = require"image"
local path = require"path"
local util = require"util"
local unorm_to_uint8_t = util.unorm_to_uint8_t
local color = require"color"
local strokestyle = require"strokestyle"
local svd = require"svd"

local util = require"util"
local is_almost_equal = util.is_almost_equal
local is_almost_zero = util.is_almost_zero
local is_almost_one = util.is_almost_one

local atan2 = math.atan2 or math.atan
local unpack = unpack or table.unpack

local _M = driver.new()

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

local function toxformablestring(xf)
	local tx, ty, ang_U, sx, sy, ang_V = factorxf(xf)
	local t = ""
	if not is_almost_zero(ang_V) then
        t = string.format("rotated(%g)", ang_V)
	end
    if is_almost_equal(sx, sy) then
        if not is_almost_one(sx) then
            t = t ~= "" and t .. ":"  or t
            t = t .. string.format("scaled(%g)", sx)
        end
    else
        t = t ~= "" and t .. ":"  or t
        t = t .. string.format("scaled(%g,%g)", sx, sy)
    end
    if not is_almost_zero(ang_U) then
        t = t ~= "" and t .. ":"  or t
        t = t .. string.format("rotated(%g)", ang_U)
    end
    if not is_almost_zero(tx) or not is_almost_zero(ty) then
        t = t ~= "" and t .. ":"  or t
        t = t .. string.format("translated(%g,%g)", tx, ty)
    end
	return t
end

local function toxformstring(xf)
	local s = toxformablestring(xf)
	s = string.gsub(s, "^rotated", "rotation")
	s = string.gsub(s, "^scaled", "scaling")
	s = string.gsub(s, "^translated", "translation")
	return s
end

local write = {}

-- here is yet an example showcasing how you can write
-- much less code if you learn a bit about Lua. you can also
-- write code that is harder to understand, and perhaps slower. :)
local function newcommandprinter(command, start, stop, file)
    -- returns a new function that receives the callback table
    -- and all arguments passed by the iterator (eg, self, len, x0, y0)
    return function(self, ...)
        -- if the previous was different, output current command
        if self.previous ~= command and (self.previous ~= "M" or
            command ~= "L")  then
            file:write(command, ",")
        end
        -- output arguments start through stop
        for i = start, stop do
            file:write(select(i, ...), ",")
        end
        -- current is new previous
        self.previous = command
    end
end

function write.path(shape, file)
    file:write("path{")
    shape:iterate{
        -- store previous command
        previous = nil,
        -- example: begin_open_contour receives self, len, x0, y0
        -- we should output "M,x0,y0". so we pass "M" as the
        -- command, and select 2 through 3 as the arguments to be output
        begin_open_contour = newcommandprinter("M", 2, 3, file),
        -- rest is similar
        begin_closed_contour = newcommandprinter("M", 2, 3, file),
        linear_segment = newcommandprinter("L", 3, 4, file),
        degenerate_segment = newcommandprinter("L", 7, 8, file),
        quadratic_segment = newcommandprinter("Q", 3, 6, file),
        rational_quadratic_segment = newcommandprinter("R", 3, 7, file),
        cubic_segment = newcommandprinter("C", 3, 8, file),
        end_closed_contour = newcommandprinter("Z", 2, 1, file),
        end_open_contour = function(self, len) self.previous = nil end,
    }
    file:write("}")
end

function write.polygon(shape, file)
    file:write("polygon{", table.concat(shape.data, ","), "}")
end

function write.triangle(shape, file)
    file:write("triangle(",
        shape.x1, ",", shape.y1, ",",
        shape.x2, ",", shape.y2, ",",
        shape.x3, ",", shape.y3, ")")
end

function write.circle(shape, file)
    file:write("circle(",
        shape.cx, ",", shape.cy, ",", shape.r, ")")
end

function write.rect(shape, file)
    file:write("rect(",
        shape.x, ",", shape.y, ",", shape.width, ",", shape.height, ")")
end

local function writecolor(c, file)
    local s = color.to_name(c)
    if not s then
        local r, g, b, a = unpack(c)
        r = unorm_to_uint8_t(r)
        g = unorm_to_uint8_t(g)
        b = unorm_to_uint8_t(b)
        a = unorm_to_uint8_t(a)
        if a ~= 255 then
            file:write("rgba8(", r, ",", g, ",", b, ",", a, ")")
        else
            file:write("rgb8(", r, ",", g, ",", b, ")")
        end
    else
        file:write("color.", s)
    end
end

local function writespread(sp, file)
    if sp == "repeat" then
        file:write('spread["repeat"]')
    else
        file:write('spread.',sp)
    end
end

local function writeramp(ramp, file)
    file:write("ramp(")
    writespread(ramp.spread, file)
    file:write(",{")
    for i, s in ipairs(ramp.stops) do
        file:write("{", s[1], ",")
        writecolor(s[2], file)
        file:write("},")
    end
    file:write('})')
end

function write.radial_gradient(paint, file)
    file:write("radial_gradient(")
    writeramp(paint.ramp, file)
    file:write(",", paint.cx)
    file:write(",", paint.cy)
    file:write(",", paint.fx)
    file:write(",", paint.fy)
    file:write(",", paint.r)
    if paint.opacity ~= 1 then
        file:write(",", paint.opacity)
    end
    file:write(")")
end

function write.linear_gradient(paint, file)
    file:write("linear_gradient(")
    writeramp(paint.ramp, file)
    file:write(",", paint.x1)
    file:write(",", paint.y1)
    file:write(",", paint.x2)
    file:write(",", paint.y2)
    if paint.opacity ~= 1 then
        file:write(",", paint.opacity)
    end
    file:write(")")
end

function write.solid_color(paint, file)
    if paint.opacity ~= 1 then
        file:write("solid_color(")
        writecolor(paint.rgba, file)
        file:write(",", paint.opacity, ")")
    else
        writecolor(paint.rgba, file)
    end
end

local function writeimage(img, file)
    file:write("image.png.load(base64.decode[[\n")
    if img.channel_type == "uint8_t" then
        file:write(base64.encode(image.png.string8(img)), "]])")
    else
        file:write(base64.encode(image.png.string16(img)), "]])")
    end
end

function write.texture(paint, file)
    file:write("texture(")
    writespread(paint.spread, file)
    file:write(",")
    writeimage(paint.image, file)
    if paint.opacity ~= 1 then
        file:write(",", paint.opacity, ")")
    else
        file:write(")")
    end
end

local function writestyle(style, file)
    local s = strokestyle
    if style.width ~= s.default_width then
        file:write(':stroked(', style.width, ')')
    end
    if style.join ~= s.default_join or
        style.miter_limit ~= s.default_miter_limit then
        file:write(':joined(join.', style.join)
        if style.miter_limit ~= s.default_miter_limit then
            file:write(',', style.miter_limit)
        end
        file:write(')')
    end
    if style.cap ~= s.default_cap then
        file:write(':capped(cap.', style.cap, ')')
    end
    if #style.dash_array > 0 then
        if style.initial_phase == s.default_initial_phase and
           style.phase_reset == s.default_phase_reset then
            file:write(':dashed{',table.concat(style.dash_array, ","), '}')
        else
            file:write(':dashed({',table.concat(style.dash_array, ","),
                '},', style.initial_phase, ',', tostring(style.phase_reset), ')')
        end
    end
    if style.method ~= s.default_method then
        file:write(':by(method.', style.method, ')')
    end
end

function write.stroked(stroked, file)
    local s = toxformablestring(stroked.shape.xf)
    write[stroked.shape.type](stroked.shape, file)
    if s ~= "" then file:write(":", s) end
    writestyle(stroked.style, file)
end

--function write.pathindex(pathindex, file)
    --file:write("paths[", pathindex.index, "]")
--end

local winding_rule_prefix = {
    ["non-zero"] = "",
    ["zero"] = "z",
    ["even"] = "e",
    ["odd"] = "eo"
}

local function newsceneprinter(file)
    local sceneprinter = { }
    local ind = indent.indent(1, "  ")

    function sceneprinter.painted_element(self, winding_rule, shape, paint)
        ind:write_inc(file)
        local s = toxformablestring(shape.xf)
        local p = toxformablestring(paint.xf)
        file:write(winding_rule_prefix[winding_rule], "fill(")
        write[shape.type](shape, file)
        if s ~= "" and s ~= p then file:write(":", s) end
        file:write(",")
        ind:write(file)
        write[paint.type](paint, file)
        if p ~= "" and p ~= s then file:write(":", p) end
        if s ~= "" and s == p then file:write("):", s, ",")
        else file:write("),") end
        ind:dec()
    end

    function sceneprinter.stencil_element(self, winding_rule, shape)
        ind:write(file)
        local s = toxformablestring(shape.xf)
        file:write(winding_rule_prefix[winding_rule], "stencil(")
        write[shape.type](shape, file)
        if s ~= "" then file:write(":", s) end
        file:write("),")
    end

    function sceneprinter.begin_transform(self, depth, xf)
        ind:write_inc(file)
        file:write("transform(", toxformstring(xf), ", {")
    end

    function sceneprinter.end_transform(self, depth, xf)
        ind:dec_write(file)
        file:write("}),")
    end

    function sceneprinter.begin_blur(self, depth, radius)
        ind:write_inc(file)
        file:write("blur(", radius, ", {")
    end

    function sceneprinter.end_blur(self, depth, radius)
        ind:dec_write(file)
        file:write("}),")
    end

    function sceneprinter.begin_fade(self, depth, opacity)
        ind:write_inc(file)
        file:write("fade(", opacity, ", {")
    end

    function sceneprinter.end_fade(self, depth, opacity)
        ind:dec_write(file)
        file:write("}),")
    end

    function sceneprinter.begin_clip(self, depth)
        ind:write_inc(file)
        file:write("clip({")
    end

    function sceneprinter.activate_clip(self, depth)
        ind:dec_write(file)
        file:write("},{")
        ind:inc()
    end

    function sceneprinter.end_clip(self, depth)
        ind:dec_write(file)
        file:write("}),")
    end

    return sceneprinter
end

-- define acceleration function
function _M.accelerate(scene, viewport)
    return scene
end

-- define rendering function
function _M.render(scene, viewport, file)
    local vxmin, vymin, vxmax, vymax = unpack(viewport, 1, 4)
    file:write("local rvg = {}\n\n")
    file:write("rvg.window = window(", table.concat(viewport, ","), ")\n\n")
    file:write("rvg.viewport = viewport(", table.concat(viewport, ","), ")\n\n")
    file:write("rvg.scene = scene({")
    scene:iterate(newsceneprinter(file))
    file:write("\n})")
    local xfs = toxformablestring(scene.xf)
    if xfs ~= "" then file:write(":", xfs) end
    file:write("\n\n")
    file:write("return rvg\n")
end

return _M
