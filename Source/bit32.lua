if _VERSION == "Lua 5.3" then
    return require"bit.53"
elseif _VERSION == "Lua 5.2" then
    return bit32
else
    return require"bit.numberlua".bit32
end
