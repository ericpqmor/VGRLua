local _M = {}

local xform = require"xform"
local xformable = require"xformable"
local strokestyle = require"strokestyle"

local stroked_meta = {}
stroked_meta.__index = {}
stroked_meta.name = "stroked"

local function newstroked(shape, style)
    return setmetatable({
        type = "stroked",
        shape = shape,
        style = style,
        xf = xform.identity()
    }, stroked_meta)
end

function _M.setmethods(index)
    function index.stroked(shape, width_or_style)
        -- return new stroked object
        return newstroked(shape, strokestyle.default():stroked(width_or_style))
    end

    function index.dashed(shape, dash_array, initial_phase, phase_reset)
        -- if not already stroked, ignore
        if _M.is_stroked(shape) then
            return newstroked(shape.shape, shape.style:dashed(dash_array,
                initial_phase, phase_reset))
        else
            return shape
        end
    end

    function index.capped(shape, cap)
        -- if not already stroked, ignore
        if _M.is_stroked(shape) then
            return newstroked(shape.shape, shape.style:capped(cap))
        else
            return shape
        end
    end

    function index.joined(shape, join, miter_limit)
        -- if not already stroked, ignore
        if _M.is_stroked(shape) then
            return newstroked(shape.shape, shape.style:joined(join,
                miter_limit))
        else
            return shape
        end
    end

    function index.by(shape, method)
        -- if not already stroked, ignore
        if _M.is_stroked(shape) then
            return newstroked(shape.shape, shape.style:by(method))
        else
            return shape
        end
    end
end

-- not implemented yet
function stroked_meta.__index.path(stroked, post_xf)
    io.stderr:write("stroking not implemented yet\n")
    io.stderr:write("  returning input shape\n")
    return stroked.shape:path():transformed(stroked.xf)
end

function _M.is_stroked(s)
    return getmetatable(s) == stroked_meta
end

_M.setmethods(stroked_meta.__index)
xformable.setmethods(stroked_meta.__index)

return _M
