local _M = { }

local xform = require"xform"

-- shallow copy object (including metatable) but replace xf
local function newxform(obj, xf)
    local copy = {}
    for i,v in pairs(obj) do
        copy[i] = v
    end
    copy.xf = xf
    return setmetatable(copy, getmetatable(obj))
end

function _M.setmethods(index)

    function index.transformed(obj, xf)
        return newxform(obj, obj.xf:transformed(xf))
    end

    function index.translated(obj, ...)
        return newxform(obj, obj.xf:transformed(xform.translation(...)))
    end

    function index.scaled(obj, ...)
        return newxform(obj, obj.xf:transformed(xform.scaling(...)))
    end

    function index.rotated(obj, ...)
        return newxform(obj, obj.xf:transformed(xform.rotation(...)))
    end

    function index.affine(obj, ...)
        return newxform(obj, obj.xf:transformed(xform.affinity(...)))
    end

    function index.linear(obj, ...)
        return newxform(obj, obj.xf:transformed(xform.linear(...)))
    end

    function index.projected(obj, ...)
        return newxform(obj, obj.xf:transformed(xform.projectivity(...)))
    end

    function index.windowviewport(obj, ...)
        return newxform(obj, obj.xf:transformed(xform.windowviewport(...)))
    end
end

return _M
