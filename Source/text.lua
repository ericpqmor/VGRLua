local _M = {}

local freetype = require"freetype"
local harfbuzz = require"harfbuzz"

local path = require"path"
local xform = require"xform"
local filter = require"filter"
local fonts = require"fonts"

local glyphs = {}

local function loadglyph(freetypeface, glyphs, codepoint)
    local g = glyphs[codepoint] -- load glyph from cache
    if g then return g, g.m end
    local c = assert(freetypeface:glyphoutline(codepoint),
        "no glyph for codepoint")
    local m = assert(freetypeface:glyphmetrics(codepoint),
        "no metrics for codepoint")
    g = path.path(c)
    g.m = m
    glyphs[codepoint] = g
    return g, g.m
end

-- glyph cache
local glyphs = {}

-- returns a path containing the string of text
function _M.text(text, facename_or_filename, features)
    -- get face from cache or load from file
    local f = fonts.loadface(facename_or_filename or fonts.files[1])
    glyphs[f] = glyphs[f] or {}
    local xcursor, ycursor = 0, 0
    local ymin = math.huge
    local ymax = -math.huge
    local xmin = math.huge
    local xmax = -math.huge
    -- path starts empty
    local p = path.path()
    local buffer = harfbuzz.buffer()
    --buffer:setdirection("ttb")
    buffer:add(text)
    buffer:shape(f.harfbuzzfont, features and harfbuzz.features(features))
    --buffer:normalizeglyphs()
    local infos, n = buffer:glyphinfos()
    local positions = buffer:glyphpositions()
    -- append each codepoint to path
    for i = 1, n do
        local g, m = loadglyph(f.freetypeface, glyphs[f], infos[i].codepoint)
        -- update bounding box
        xmin = math.min(xmin, xcursor+m.metrics.horiBearingX)
        xmax = math.max(xmax, xcursor+m.metrics.horiBearingX+m.metrics.width)
        ymin = math.min(ymin, ycursor+m.metrics.horiBearingY-m.metrics.height)
        ymax = math.max(ymax, ycursor+m.metrics.horiBearingY)
        -- add glyph to path, translated by cursor
        g:iterate(
          filter.xform(
            xform.translation(
                xcursor + positions[i].x_offset,
                ycursor + positions[i].y_offset
            ), p))
        xcursor = xcursor + positions[i].x_advance
        ycursor = ycursor + positions[i].y_advance
    end
    return p, f.freetypeface, xmin, ymin, xmax, ymax
end

return _M
