local _M = {}

local window = require"window"
local viewport = require"viewport"
local util = require"util"

local xform_meta = {}
xform_meta.__index = {}
xform_meta.name = "xform"

local abs = math.abs
local rad = math.rad
local cos = math.cos
local sin = math.sin
local is_almost_zero = util.is_almost_zero
local is_almost_equal = util.is_almost_equal
local is_almost_one = util.is_almost_one
local unpack = unpack or table.unpack

function xform_meta.__call(A, i, j)
    return A[(i-1)*3+(j-1)+1]
end

function xform_meta.__index.is_almost_equal(A, B)
    assert(_M.is_xform(A) and _M.is_xform(B), "not a xform")
    for i = 1, 9 do
        if not is_almost_equal(A[i], B[i]) then
            return false
        end
    end
    return true
end

function xform_meta.__index.is_linear(A)
    assert(getmetatable(A) == xform_meta, "not a xform")
    return is_almost_zero(A[3]) and
           is_almost_zero(A[6]) and
           is_almost_zero(A[7]) and
           is_almost_zero(A[8]) and
           is_almost_one(A[9])
end

function xform_meta.__index.is_equal(A, B)
    assert(getmetatable(A) == xform_meta and
           getmetatable(B) == xform_meta, "not a xform")
    for i = 1, 9 do
        if A[i] ~= B[i] then
            return false
        end
    end
    return true
end

function xform_meta.__tostring(A)
    return string.format("xform{%f,%f,%f,%f,%f,%f,%f,%f,%f}",
        A[1], A[2], A[3], A[4], A[5], A[6], A[7], A[8], A[9])
end

function xform_meta.__index.transformed(B, A)
    assert(getmetatable(A) == xform_meta, "not a xform")
    assert(getmetatable(B) == xform_meta, "not a xform")
    local M = {0., 0., 0., 0., 0., 0., 0., 0., 0.}
    for i=0,2 do
        for j=0,2 do
            local s = 0
            for k = 0, 2 do
                s = s + A[i*3+k+1]*B[k*3+j+1]
            end
            M[i*3+j+1] = s
        end
    end
    return setmetatable(M, xform_meta)
end

function xform_meta.__index.apply(A, x, y, w)
    w = w or 1
    local ax = A[0*3+0+1]*x + A[0*3+1+1]*y + A[0*3+2+1]*w
    local ay = A[1*3+0+1]*x + A[1*3+1+1]*y + A[1*3+2+1]*w
    local aw = A[2*3+0+1]*x + A[2*3+1+1]*y + A[2*3+2+1]*w
    return ax, ay, aw
end

function xform_meta.__index.det(A)
    local a, b, c, d, e, f, g, h, i = unpack(A, 1, 9)
    return -c*e*g + b*f*g + c*d*h - a*f*h - b*d*i + a*e*i
end

function xform_meta.__index.adjugate(A)
    local a, b, c, d, e, f, g, h, i = unpack(A, 1, 9)
    return setmetatable({
        -f*h + e*i, c*h - b*i,-c*e + b*f,
         f*g - d*i,-c*g + a*i, c*d - a*f,
        -e*g + d*h, b*g - a*h,-b*d + a*e
    }, xform_meta)
end

function xform_meta.__index.transpose(A)
    local a, b, c, d, e, f, g, h, i = unpack(A, 1, 9)
    return setmetatable({
        a, d, g,
        b, e, h,
        c, f, i
    }, xform_meta)
end

function xform_meta.__index.inverse(A)
    local a, b, c, d, e, f, g, h, i = unpack(A, 1, 9)
    local inv_det = -c*e*g + b*f*g + c*d*h - a*f*h - b*d*i + a*e*i
    assert(not is_almost_zero(inv_det), "singular")
    inv_det = 1./inv_det
    return setmetatable({
        inv_det*(-f*h + e*i),inv_det*( c*h - b*i),inv_det*(-c*e + b*f),
        inv_det*( f*g - d*i),inv_det*(-c*g + a*i),inv_det*( c*d - a*f),
        inv_det*(-e*g + d*h),inv_det*( b*g - a*h),inv_det*(-b*d + a*e)
    }, xform_meta)
end

function xform_meta.__index.det(A)
    local a, b, c, d, e, f, g, h, i = unpack(A, 1, 9)
    return -c*e*g + b*f*g + c*d*h - a*f*h - b*d*i + a*e*i
end

local I = setmetatable({1.,0.,0.,0.,1.,0.,0.,0.,1.}, xform_meta)

function xform_meta.__index.is_identity(A)
    for i = 1, 9 do
        if A[i] ~= I[i] then
            return false
        end
    end
    return true
end

local function rotation_by_angle(angle)
    angle = rad(angle)
    local s = sin(angle)
    local c = cos(angle)
    return setmetatable(
        {c, -s, 0., s, c, 0., 0., 0., 1.},
        xform_meta
    )
end

local function rotation_by_cos_sin(c, s)
    return setmetatable(
        {c, -s, 0., s, c, 0., 0., 0., 1.},
        xform_meta
    )
end

local function centered_rotation_by_angle(angle, cx, cy)
    angle = rad(angle)
    local s = sin(angle)
    local c = cos(angle)
    return setmetatable(
        {c, -s, cx*(1-c)+s*cy,
         s, c,  cy*(1-c)-s*cx,
         0., 0., 1.},
        xform_meta
    )
end

function _M.rotation(a, b, c)
    assert(type(a) == "number")
    assert(not b or type(b) == "number")
    assert(not c or (type(c) == "number" and b))
    if b then
        if c then
            return centered_rotation_by_angle(a, b, c)
        else
            return rotation_by_cos_sin(a, b)
        end
    else
        return rotation_by_angle(a)
    end
end

local function centered_anisotropic_scale(sx, sy, cx, cy)
    return setmetatable(
        {sx, 0., cx*(1-sx),
         0., sy, cy*(1-sy),
         0., 0., 1.},
        xform_meta
    )
end


local function centered_isotropic_scale(s, cx, cy)
    return setmetatable(
        {s, 0., cx*(1-s),
         0., s, cy*(1-s),
         0., 0., 1.},
        xform_meta
    )
end

local function anisotropic_scale(sx, sy)
    return setmetatable(
        {sx, 0., 0., 0., sy, 0., 0., 0., 1.},
        xform_meta
    )
end

local function isotropic_scale(s)
    return setmetatable(
        {s, 0., 0., 0., s, 0., 0., 0., 1.},
        xform_meta
    )
end

function _M.scaling(a, b, c, d)
    assert(type(a) == "number")
    assert(not b or type(b) == "number")
    assert(not c or (type(c) == "number" and b))
    assert(not d or (type(d) == "number" and b and c))
    if b then
        if c then
            if d then
                return centered_anisotropic_scale(a, b, c, d)
            else
                return centered_isotropic_scale(a, b, c)
            end
        else
            return anisotropic_scale(a, b)
        end
    else
        return isotropic_scale(a)
    end
end

function _M.projectivity(a11, a12, a13, a21, a22, a23, a31, a32, a33)
    assert(type(a11) == "number")
    assert(type(a12) == "number")
    assert(type(a13) == "number")
    assert(type(a21) == "number")
    assert(type(a22) == "number")
    assert(type(a23) == "number")
    assert(type(a31) == "number")
    assert(type(a32) == "number")
    assert(type(a33) == "number")
    return setmetatable(
        {a11, a12, a13, a21, a22, a23, a31, a32, a33},
        xform_meta
    )
end

function _M.affinity(a11, a12, a13, a21, a22, a23)
    assert(type(a11) == "number")
    assert(type(a12) == "number")
    assert(type(a13) == "number")
    assert(type(a21) == "number")
    assert(type(a22) == "number")
    assert(type(a23) == "number")
    return setmetatable(
        {a11, a12, a13, a21, a22, a23, 0., 0., 1.},
        xform_meta
    )
end

function _M.linear(a11, a12, a21, a22)
    assert(type(a11) == "number")
    assert(type(a12) == "number")
    assert(type(a21) == "number")
    assert(type(a22) == "number")
    return setmetatable(
        {a11, a12, 0., a21, a22, 0., 0., 0., 1.},
        xform_meta
    )
end

function _M.translation(tx, ty)
    assert(type(tx) == "number")
    ty = ty or 0
    assert(type(ty) == "number")
    return setmetatable(
        {1., 0., tx, 0., 1., ty, 0., 0., 1.},
        xform_meta
    )
end

function _M.windowviewport(w, v)
    assert(window.is_window(w), "expected window")
    assert(viewport.is_viewport(v), "expected viewport")
    local wxmin, wymin, wxmax, wymax = unpack(w, 1, 4)
    local wxmed, wymed = .5*(wxmin+wxmax), .5*(wymin+wymax)
    local wdx, wdy = wxmax-wxmin, wymax-wymin
    local vxmin, vymin, vxmax, vymax = unpack(v, 1, 4)
    local vxmed, vymed = .5*(vxmin+vxmax), .5*(vymin+vymax)
    local vdx, vdy = vxmax-vxmin, vymax-vymin
    return _M.translation(vxmed, vymed) *
           _M.scaling(vdx/wdx, vdy/wdy) *
           _M.translation(-wxmed, -wymed)
end

function _M.identity(...)
    assert(select('#', ...) == 0, "too many arguments")
    return I
end

function xform_meta.__index.translated(A, ...)
    return A:transformed(_M.translation(...))
end

function xform_meta.__index.scaled(A, ...)
    return A:transformed(_M.scaling(...))
end

function xform_meta.__index.rotated(A, ...)
    return A:transformed(_M.rotation(...))
end

function xform_meta.__index.affine(A, ...)
    return A:transformed(_M.affinity(...))
end

function xform_meta.__index.projected(A, ...)
    return A:transformed(_M.projectivity(...))
end

function xform_meta.__index.linear(A, ...)
    return A:transformed(_M.linear(...))
end

function xform_meta.__index.windowviewport(A, ...)
    return A:transformed(_M.windowviewport(...))
end

function xform_meta.__mul(A, B)
    assert(getmetatable(A) == xform_meta and
           getmetatable(B) == xform_meta, "not a xform")
    return B:transformed(A)
end

function _M.is_xform(xf)
    return getmetatable(xf) == xform_meta
end

return _M
