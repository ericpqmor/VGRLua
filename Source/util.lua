local _M = {}

local abs = math.abs
local max = math.max
local sqrt = math.sqrt
local min = math.min
local floor = math.floor
local FLT_EPS = 1.19209290E-07
local MAX_REL = 8*FLT_EPS
local MAX_ABS = 8*FLT_EPS

function _M.stderr(...)
    io.stderr:write(string.format(...))
end

function _M.is_almost_equal(a, b)
    local diff = abs(a-b);
    if (diff < MAX_ABS) then return true end
    a, b = abs(a), abs(b)
    return diff <= max(a, b) * MAX_REL
end

function _M.is_almost_zero(a)
    return abs(a) < MAX_ABS
end

function _M.is_almost_one(f)
    local diff = abs(f-1.)
    if (diff < MAX_ABS) then return true end
    f = abs(f)
    return diff <= max(f,1.) * MAX_REL
end

function _M.det2(a, b, c, d)
    return a*d-b*c
end

-- adjugate matrix
function _M.adjugate2(a, b, c, d)
    return d, -b, -c, a
end

function _M.apply2(a, b, c, d, x, y)
    return a*x+b*y, c*x+d*y
end

function _M.dot2(x0, y0, x1, y1)
    return x0*x1 + y0*y1
end

function _M.norm2(x, y)
    return x*x + y*y
end

function _M.perp(x, y, w)
    return -y, x, w
end

function _M.sign(v)
    if v < 0. then return -1.
    elseif v > 0. then return 1.
    else return 0. end
end

function _M.unorm_to_uint8_t(f)
    return floor(min(255, max(0, f*256)))
end

local a = 1./254.
local b = -.5/254.
function _M.uint8_t_to_unorm(i)
    return min(1., max(0., a*i+b))
end


return _M
