local _M = {}

local xform = require"xform"
local cmdtoinstr = require"commandtoinstruction"
local xformable = require"xformable"
local strokable = require"strokable"

local unpack = unpack or table.unpack

local path_meta = {}
path_meta.__index = {}
path_meta.name = "path"

-- Our internal representation of paths consists of an array
-- with instructions, an array of offsets, and an array with
-- data.  Each instruction has a corresponding offset entry
-- pointing into the data array.  There are two interfaces
-- to add information to the path.  The traditional
-- interface is based on move_to, line_to, close_path etc
-- commands. These are converted to our internal
-- representation in a way that guarantees consistency.  The
-- internal representation can be used to directly add
-- instructions and associated data, without much in the way
-- of consistency checks.  The internal and traditional
-- interfaces should not be mixed when adding information to
-- a path, since they depend on some internal state.
--
-- Contours are bracketed by a begin/end pair of
-- instructions.  The pair can be either open or closed.
-- (For example, depending on whether there was a close_path
-- command or not).  Bracketing instructions carry a length
-- data that represents the number to be added to the open
-- instruction index to reach the close instruction index.
--
-- The offset of each instruction points to
-- the start of the instruction's data, so that all
-- instructions can be processed in parallel if need be.
-- Many instructions share common data. In the table below,
-- the data that each instruction needs when being added to
-- a path is marked with '+'. The data to which the
-- instruction's offset points is marked with a '^'
--
-- BOC ^+len +x0 +y0                      ; begin_open_contour
-- EOC ^x0 y0 +len                        ; end_open_contour
-- BCC ^+len +x0 +y0                      ; begin_closed_contour
-- ECC ^x0 y0 +len                        ; end_closed_contour
-- DS  ^x0 y0 +dx0 +dy0 +dx1 +dy1 +x1 +y1 ; degenerate_segment
-- LS  ^x0 y0 +x1 +y1                     ; linear_segment
-- QS  ^x0 y0 +x1 +y1 +x2 +y2             ; quadratic_segment
-- RQS ^x0 y0 +x1 +y1 +w1 +x2 +y2         ; rational_quadratic_segment
-- CS  ^x0 y0 +x1 +y1 +x2 +y2 +x3 +y3     ; cubic_segment
--
-- The degenerate segment represents a segment with zero
-- length. dx0 dy0 represents the tangent direction before
-- the segment. dx1 dy1 the tangent direction after the
-- segment. x1 y1 simply repeats the control point before
-- the segment, for reversibility
--
-- The len in begin/end instructions (when applicable) allows us
-- to find the matching end/begin instruction and is computed
-- automatically.
--
-- The idea is that the representation is reversible in the
-- sense that traversing it forward or backward is equally
-- easy. The datastructure also provide easy random access
-- to the data for each instruction
--
-- For most uses, you do not need to inspect the internal
-- representation. Use the iterate() method instead.

local function push_data(path, ...)
    local data = path.data
    local n = #data
    for i = 1, select("#", ...) do
        data[n+i] = select(i, ...)
    end
end

local function push_instruction(path, type, rewind)
    rewind = rewind or -2
    local instructions_n = #path.instructions+1
    local data_n = #path.data+1
    path.instructions[instructions_n] = type
    path.offsets[instructions_n] = data_n+rewind
end

local bracket = {
    ["begin_open_contour"] = "end_open_contour",
    ["end_open_contour"] = "begin_open_contour",
    ["begin_closed_contour"] = "end_closed_contour",
    ["end_closed_contour"] = "begin_closed_contour"
}

function path_meta.__index.begin_contour(path, type, len, x0, y0)
    assert(not path.ibegin_contour, "nested contour")
    -- add new instruction
    push_instruction(path, type, 0)
    push_data(path, len or 0, x0, y0)
    path.ibegin_contour = #path.instructions
end

function path_meta.__index.end_contour(path, type, x0, y0, len)
    -- index of matching begin instruction
    local bc = assert(path.ibegin_contour, "no contour to end")
    -- clear to signal we are not inside a contour anymore
    path.ibegin_contour = nil
    -- index of new end instruction
    local ec = #path.instructions + 1
    -- length is offset between matching instruction indices
    len = ec - bc
    -- if type is not given, infer from matching begin contour
    type = type or bracket[path.instructions[bc]]
    -- make sure begin instruction matches
    path.instructions[bc] = bracket[type]
    -- update length of matching begin instruction
    path.data[path.offsets[bc]] = len
    -- add end instruction
    push_instruction(path, type)
    push_data(path, len)
end

function path_meta.__index.begin_open_contour(path, len, x0, y0)
    path:begin_contour("begin_open_contour", len, x0, y0)
end

function path_meta.__index.begin_closed_contour(path, len, x0, y0)
    path:begin_contour("begin_closed_contour", len, x0, y0)
end

function path_meta.__index.end_open_contour(path, x0, y0, len)
    path:end_contour("end_open_contour", x0, y0, len)
end

function path_meta.__index.end_closed_contour(path, x0, y0, len)
    path:end_contour("end_closed_contour", x0, y0, len)
end

function path_meta.__index.degenerate_segment(path, x0, y0, dx0, dy0,
    dx1, dy1, x1, y1)
    -- ignore x0, y0: comes from previous instruction
    push_instruction(path, "degenerate_segment")
    push_data(path, dx0, dy0, dx1, dy1, x1, y1)
end

function path_meta.__index.linear_segment(path, x0, y0, x1, y1)
    -- ignore x0, y0: comes from previous instruction
    push_instruction(path, "linear_segment")
    push_data(path, x1, y1)
end

function path_meta.__index.quadratic_segment(path, x0, y0, x1, y1, x2, y2)
    -- ignore x0, y0: comes from previous instruction
    push_instruction(path, "quadratic_segment")
    push_data(path, x1, y1, x2, y2)
end

function path_meta.__index.rational_quadratic_segment(path, x0, y0, x1, y1, w1,
        x2, y2)
    -- ignore x0, y0: comes from previous instruction
    push_instruction(path, "rational_quadratic_segment")
    push_data(path, x1, y1, w1, x2, y2)
end

function path_meta.__index.cubic_segment(path, x0, y0, x1, y1, x2, y2, x3, y3)
    -- ignore x0, y0: comes from previous instruction
    push_instruction(path, "cubic_segment")
    push_data(path, x1, y1, x2, y2, x3, y3)
end

local ndata = { -- if instruction has n data elements, store n-1
    begin_open_contour = 2,
    end_open_contour = 2,
    begin_closed_contour = 2,
    end_closed_contour = 2,
    degenerate_segment = 7,
    linear_segment = 3,
    quadratic_segment = 5,
    rational_quadratic_segment = 6,
    cubic_segment = 7,
}

-- iterate over path contents
function path_meta.__index.iterate(path, forward)
    local data = path.data
    local offsets = path.offsets
    local instructions = path.instructions
    for index = 1, #instructions do
        local instruction = instructions[index]
        local offset = offsets[index]
        -- invoke method with same name as
        -- instruction from forward table, passing all its data
        local callback = forward[instruction]
        if not callback then
            error("unhandled instruction '" .. instruction .. "'")
        end
        callback(forward, unpack(data, offset, offset+ndata[instruction]))
    end
end

-- convert shape to a path, preserving the original xf in the shape
-- in the case of paths, simply returns the path itself
function path_meta.__index.as_path(p, screen_xf)
    return p
end

-- creates and returns a new empty path
function _M.path(cmds)
    -- create empty path
    local p = setmetatable({
        type = "path", -- shape type
        instructions = {},
        offsets = {},
        data = {},
        xf = xform.identity()
    }, path_meta)
    -- fill path from command table
    if cmds then
        cmdtoinstr.append(cmds, p)
    end
    return p
end

-- checkes if object is a path
function _M.is_path(p)
    return getmetatable(p) == path_meta
end

-- make path objects xformable
xformable.setmethods(path_meta.__index)
-- make path objects strokable
strokable.setmethods(path_meta.__index)

return _M
