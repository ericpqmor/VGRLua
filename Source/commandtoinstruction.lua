local _M = {}

local command = require"command"
local nargs = command.nargs
local letter = command.letter

local arc = require"arc"

local unpack = unpack or table.unpack

local function init_cursor(path)
    path.current_x = 0
    path.current_y = 0
    path.previous_x = 0
    path.previous_y = 0
    path.start_x = 0
    path.start_y = 0
end

local function deinit_cursor(path)
    path.current_x = nil
    path.current_y = nil
    path.previous_x = nil
    path.previous_y = nil
    path.start_x = nil
    path.start_y = nil
    assert(not path.ibegin_contour, "expected end contour")
end

local function set_previous(path, x, y)
    path.previous_x = x
    path.previous_y = y
end

local function set_current(path, x, y)
    path.current_x = x
    path.current_y = y
end

local function set_start(path, x, y)
    path.start_x = x
    path.start_y = y
end

local function ensure_begun(path)
    if not path.ibegin_contour then
        path:begin_open_contour(nil, path.current_x, path.current_y)
    end
end

local function ensure_ended(path, type)
    if path.ibegin_contour then
        -- check for empty contour and insert a degenerate segment
        if #path.instructions == path.ibegin_contour then
            path:degenerate_segment(nil, nil, 0, 0, 0, 0,
                path.current_x, path.current_y)
        end
        path:end_contour(type, nil, nil, nil, nil)
    end
end

local cmd = {}

function cmd.move_to_abs(path, x0, y0)
    ensure_ended(path, "end_open_contour")
    path:begin_open_contour(nil, x0, y0)
    set_start(path, x0, y0)
    set_current(path, x0, y0)
    set_previous(path, x0, y0)
    return "line_to_abs" -- implicit command that follows is line
end

function cmd.close_path(path)
    ensure_ended(path, "end_closed_contour")
end

function cmd.line_to_abs(path, x1, y1)
    ensure_begun(path)
    local x0, y0 = path.current_x, path.current_y
    if x1 == x0 and y1 == y0 then
        path:degenerate_segment(nil, nil, 0, 0, 0, 0, x1, y1)
    else
        path:linear_segment(nil, nil, x1, y1)
    end
    set_previous(path, x1, y1)
    set_current(path, x1, y1)
end

function cmd.quad_to_abs(path, x1, y1, x2, y2)
    ensure_begun(path)
    local x0, y0 = path.current_x, path.current_y
    if x1 == x0 and y1 == y0 and x2 == x0 and y2 == y0 then
        path:degenerate_segment(nil, nil, 0, 0, 0, 0, x2, y2)
    else
        path:quadratic_segment(nil, nil, x1, y1, x2, y2)
    end
    set_previous(path, x1, y1)
    set_current(path, x2, y2)
end

function cmd.rquad_to_abs(path, x1, y1, w1, x2, y2)
    ensure_begun(path)
    local x0, y0 = path.current_x, path.current_y
    if x1 == x0*w1 and y1 == y0*w1 and x2 == x0 and y2 == y0 then
        path:degenerate_segment(nil, nil, 0, 0, 0, 0, x2, y2)
    else
        path:rational_quadratic_segment(nil, nil, x1, y1, w1, x2, y2)
    end
    set_previous(path, x2, y2)
    set_current(path, x2, y2)
end

function cmd.cubic_to_abs(path, x1, y1, x2, y2, x3, y3)
    ensure_begun(path)
    local x0, y0 = path.current_x, path.current_y
    if x1 == x0 and y1 == y0 and x2 == x0 and y2 == y0 and
       x3 == x0 and y3 == y0 then
        path:degenerate_segment(nil, nil, 0, 0, 0, 0, x3, y3)
    else
        path:cubic_segment(nil, nil, x1, y1, x2, y2, x3, y3)
    end
    set_previous(path, x2, y2)
    set_current(path, x3, y3)
end

function cmd.squad_to_abs(path, x2, y2)
    local x1 = 2.*path.current_x - path.previous_x
    local y1 = 2.*path.current_y - path.previous_y
    cmd.quad_to_abs(path, x1, y1, x2, y2)
end

function cmd.squad_to_rel(path, x2, y2)
    x2 = x2 + path.current_x
    y2 = y2 + path.current_y
    cmd.squad_to_abs(path, x2, y2)
end

function cmd.rquad_to_rel(path, x1, y1, w1, x2, y2)
    x1 = x1 + path.current_x*w1
    y1 = y1 + path.current_y*w1
    x2 = x2 + path.current_x
    y2 = y2 + path.current_y
    cmd.rquad_to_abs(path, x1, y1, w1, x2, y2)
end

-- ??D split wide angled arcs into two
function cmd.svgarc_to_abs(path, rx, ry, rot_ang, fa, fs, x2, y2)
    local x0, y0 = path.current_x, path.current_y
    local x1, y1, w1 = arc.torational(x0, y0, rx, ry, rot_ang, fa, fs, x2, y2)
    cmd.rquad_to_abs(path, x1, y1, w1, x2, y2)
end

function cmd.svgarc_to_rel(path, rx, ry, rot_ang, fa, fs, x2, y2)
    x2 = x2 + path.current_x
    y2 = y2 + path.current_y
    cmd.svgarc_to_abs(path, rx, ry, rot_ang, fa, fs, x2, y2)
end

function cmd.cubic_to_rel(path, x1, y1, x2, y2, x3, y3)
    x1 = x1 + path.current_x
    y1 = y1 + path.current_y
    x2 = x2 + path.current_x
    y2 = y2 + path.current_y
    x3 = x3 + path.current_x
    y3 = y3 + path.current_y
    cmd.cubic_to_abs(path, x1, y1, x2, y2, x3, y3)
end

function cmd.hline_to_abs(path, x1)
    local y1 = path.current_y
    cmd.line_to_abs(path, x1, y1)
end

function cmd.hline_to_rel(path, x1)
    x1 = x1 + path.current_x
    cmd.hline_to_abs(path, x1)
end

function cmd.line_to_rel(path, x1, y1)
    x1 = x1 + path.current_x
    y1 = y1 + path.current_y
    cmd.line_to_abs(path, x1, y1)
end

function cmd.move_to_rel(path, x0, y0)
    x0 = x0 + path.current_x
    y0 = y0 + path.current_y
    cmd.move_to_abs(path, x0, y0)
    return "line_to_rel"
end

function cmd.quad_to_rel(path, x1, y1, x2, y2)
    x1 = x1 + path.current_x
    y1 = y1 + path.current_y
    x2 = x2 + path.current_x
    y2 = y2 + path.current_y
    cmd.quad_to_abs(path, x1, y1, x2, y2)
end

function cmd.scubic_to_abs(path, x1, y1, x2, y2)
    local x0 = 2.*path.current_x - path.previous_x
    local y0 = 2.*path.current_y - path.previous_y
    cmd.cubic_to_abs(path, x0, y0, x1, y1, x2, y2)
end

function cmd.scubic_to_rel(path, x1, y1, x2, y2)
    x1 = x1 + path.current_x
    y1 = y1 + path.current_y
    x2 = x2 + path.current_x
    y2 = y2 + path.current_y
    cmd.scubic_to_abs(path, x1, y1, x2, y2)
end

function cmd.vline_to_abs(path, y1)
    local x1 = path.current_x
    cmd.line_to_abs(path, x1, y1)
end

function cmd.vline_to_rel(path, y1)
    y1 = y1 + path.current_y
    cmd.vline_to_abs(path, y1)
end

local function checkarguments(data, first, last)
    for i = first, last do
        if type(data[i]) ~= "number" then
            error(string.format("entry %d not a number", i))
        end
    end
end

local function stringtotable(cmds)
    -- this is not very fast, but it is very easy
    cmds = string.gsub(cmds, "^%s*", "")
    cmds = string.gsub(cmds, "%s*$", "")
    cmds = string.gsub(cmds, "%s+", ",")
    cmds = string.gsub(cmds, "([TtRrAaCcHhLlMmQqSsVvZz])([%d%.])", "%1,%2")
    cmds = string.gsub(cmds, "([%d%.])([TtRrAaCcHhLlMmQqSsVvZz])", "%1,%2")
    local t = {}
    local i = 1
    string.gsub("," .. cmds, ",([^%,]+)", function(v)
        local c = tonumber(v)
        if not c then c = command.name[v] end
        assert(c, "not a number of command")
        t[i] = c
        i = i + 1
    end)
    return t
end


-- append SVG commands to path
function _M.append(cmds, path)
    if type(cmds) == "string" then
        cmds = stringtotable(cmds)
    end
    local c = cmds[1]
    -- empty path?
    if not c then return path end
    local first = 2
    init_cursor(path)
    -- first command must be move_to_*
    assert(c == "move_to_abs" or c == "move_to_rel",
        "path must start with move_to_abs or move_to_rel")
    while true do
        local count = nargs[c]
        local last = first + count
        -- make sure arguments are numbers
        checkarguments(cmds, first, last-1)
        -- allow command to replace itself (move_to_* becomes line_to_*)
        c = cmd[c](path, unpack(cmds, first, last-1)) or c
        first = last
        if c == "close_path" or type(cmds[first]) ~= "number" then
            c = cmds[first]
            if not c then break end
            if not letter[c] then
                error(string.format("entry %d not a command", first))
            end
            first = first + 1
        end
    end
    -- end last contour in path
    ensure_ended(path)
    deinit_cursor(path)
end

return _M
