local _M = { }

local abs = math.abs
local min = math.min
local max = math.max
local sqrt = math.sqrt

local xform = require"xform"

local util = require"util"
local is_almost_zero = util.is_almost_zero

local function sq(s)
    return s*s
end

-- our own implementation of the C hypot function
local function hypot(x, y)
    x, y = abs(x), abs(y)
    if is_almost_zero(x) and is_almost_zero(y) then return 0. end
    local t = min(x,y);
    x = max(x,y)
    t = t/x
    return x*sqrt(1.+t*t)
end

-- build an elementary projector from one of the
-- vectors in the nullspace of symmetric matrix
-- {{r, s}, {s,t}}, which is known to be rank defficient
-- returns the cos and the sin of the rotate
local function projector(r, s, t)
    if abs(r) > abs(t) then
        local h = hypot(r, s)
        if not is_almost_zero(h) then
            local inv_h = 1./h
            return s*inv_h, -r*inv_h
        else
            return 1., 0.
        end
    else
        local h = hypot(t, s)
        if not is_almost_zero(h) then
            local inv_h = 1./h
            return t*inv_h, -s*inv_h
        else
            return 1., 0.
        end
    end
end

-- returns the cos and sin of the rotation angle for U,
-- followed by the sx and sy of the scale S,
-- and omits the orthogonal matrix V
function _M.us(a, b, c, d)
    -- we start computing the two roots of the characteristic
    -- polynomial of AAt as t^2 -m t + p == 0
    local a2, b2, c2, d2 = sq(a), sq(b), sq(c), sq(d)
    local m = a2+b2+c2+d2
    local p = sq(b*c-a*d)
    -- sqrt of discriminant
    local D = hypot(b+c, a-d)*hypot(b-c, a+d)
    if not is_almost_zero(m) then
        -- get largest root
        local el0 = .5*(m+D)
        -- so now we have the largest singular value
        local s0 = sqrt(el0);
        -- get projector from AAt - el0*I
        local cos, sin = projector(a2+b2-el0, a*c+b*d, c2+d2-el0)
        -- get smallest root and singular value
        local el1 = p/el0
        local s1 = sqrt(el1)
        if not is_almost_zero(s1) then -- both singular values are above threshold
            return cos, sin, s0, s1
        else -- only largest is above threshold
            return cos, sin, s0, 0.
        end
    else  -- zero matrix
        return 1., 0., 0., 0.
    end
end

-- returns U, S, and V as matrices
function _M.usv(A)
    assert(A:is_linear())
    local a, b, c, d = A[1], A[2], A[4], A[5]
    -- we start computing the two roots of the characteristic
    -- polynomial of AAt as t^2 -m t + p == 0
    local a2, b2, c2, d2 = sq(a), sq(b), sq(c), sq(d)
    local m = a2+b2+c2+d2
    local p = sq(b*c-a*d)
    -- sqrt of discriminant
    local D = hypot(b+c, a-d)*hypot(b-c, a+d)
    if not is_almost_zero(m) then
        -- get largest root
        local el0 = .5*(m+D)
        -- so now we have the largest singular value
        local s0 = sqrt(el0);
        -- get projector from AAt - el0*I
        local cos, sin = projector(a2+b2-el0, a*c+b*d, c2+d2-el0)
        local U = xform.rotation(cos, sin)
        -- get smallest root and singular value
        local el1 = p/el0
        local s1 = sqrt(el1)
        if not is_almost_zero(s1) then -- both singular values are above threshold
            local S = xform.scaling(s0, s1)
            local V = A:transpose()*U*xform.scaling(1./s0, 1./s1)
            return U, S, V
        else -- only largest is above threshold
            local S = xform.scaling(s0, 0.)
            local x, y = A:transpose():apply(cos, sin)
            local V = xform.linear(x/s0, -y/s0, y/s0, x/s0)
            return U, S, V
        end
    else  -- zero matrix
        return xform.identity(), xform.scaling(0., 0.), xform.identity()
    end
end

return _M
