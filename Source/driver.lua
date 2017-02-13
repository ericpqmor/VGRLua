local _M = {}

local function merge(source, dest)
    for i,v in pairs(source) do
        dest[i] = v
    end
end


-- this builds a minimal driver from all the modules
-- the idea is to initialize your module table with new()
-- and fill in or replace the functions that you need in a
-- particular new driver
function _M.new()
    local newM = {}
    local type = type
    merge(require"math", newM) -- include math library
    newM.type = type -- remove ovewriten type function
    merge(require"path", newM)
    merge(require"circle", newM)
    merge(require"polygon", newM)
    merge(require"triangle", newM)
    merge(require"rect", newM)
    local command = require"command"
    merge(command.name, newM)
    newM.spread = require"spread"
    local color = require"color"
    merge(color, newM)
    local paint = require"paint"
    merge(paint, newM)
    newM.color = color.named
    merge(require"ramp", newM)
    newM.image = require"image"
    newM.base64 = require"base64"
    merge(require"xform", newM)
    merge(require"viewport", newM)
    merge(require"window", newM)
    merge(require"description", newM)
    local strokestyle = require"strokestyle"
    newM.cap = strokestyle.cap
    newM.join = strokestyle.join
    newM.method = strokestyle.method
    return newM
end

return _M
