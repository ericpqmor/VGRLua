local _M = {}

_M.bnot = function(a)
    return ~a & 0xFFFFFFFF
end

_M.band = function(a, b)
    return a & b
end

_M.bor = function(a, b)
    return (a|b) & 0xFFFFFFFF
end

_M.bxor = function(a, b)
    return (a~b) & 0xFFFFFFFF
end

_M.lshift = function(a, b)
    return (a<<b) & 0xFFFFFFFF
end

_M.rshift = function(a, b)
    return a >> b
end

return _M
