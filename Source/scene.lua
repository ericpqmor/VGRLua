local _M = {}

local unpack = unpack or table.unpack

local xform = require"xform"
local xformable = require"xformable"

local scene_meta = {}
scene_meta.__index = {}
scene_meta.name = "scene"

-- Our internal representation for scenes consists of an
-- array of shapes, an array of paints, an array of elements,
-- and an array of brackets.
-- Elements can be painted elements, in which case they
-- contain shape and paint indices into the shape and pain
-- arrays.
-- A simple scene would consist of list of such elements to
-- be painted in order, one on top of the next.
-- The brackets are used for operations over groups of elements.
-- For example, a blur bracket specifies a group of elements
-- that will be blurred together with a Gaussian filter of a
-- given standard deviation.
-- A faded bracket specifies a group of elements that will
-- be painted and then have their transparency modulated as
-- a group.
-- A transform bracket applies a transformation to the group.
-- A clip bracket is used to define clipped regions.
-- It specifies two groups.
-- The first group is a set of shapes the "clipper" group.
-- The union of their interiors forms the clipping region.
-- The second group is the "clippee". Their interior is
-- restricted by the clipper group.
-- Clip brackets can be nested, so that clippees can be
-- restricted by multiple groups of clippers, and the
-- clippers themseves can also be restricted by other
-- clippers.
-- A clipper group consists of stencil elements. These
-- contain a single index into the shape array.
-- Brackets are defined by their type, attributes, and by
-- the position where they become active in the element
-- array.
--
-- The scene also contains a xform. This should be applied to all its
-- shapes and paints
--
-- For most uses, you do not need to inspect the internal
-- representation. Use the iterate() method instead.

-- add a new painted element to the scene
function scene_meta.__index.painted_element(self, winding_rule, shape, paint)
    local shape_id = #self.shapes+1
    self.shapes[shape_id] = shape
    local paint_id = #self.paints+1
    self.paints[paint_id] = paint
    local element_id = #self.elements+1
    self.elements[element_id] = {
        type = "painted_element",
        winding_rule = winding_rule,
        shape_id = shape_id,
        paint_id = paint_id
    }
end

-- add a new stencil element to the scene
function scene_meta.__index.stencil_element(self, winding_rule, shape)
    local shape_id = #self.shapes+1
    self.shapes[shape_id] = shape
    local element_id = #self.elements+1
    self.elements[element_id] = {
        type = "stencil_element",
        winding_rule = winding_rule,
        shape_id = shape_id
    }
end

-- begin a clip group
-- following stencil elements, until activate_clip, define the clipper
function scene_meta.__index.begin_clip(self, depth)
    local bracket_id = #self.brackets+1
    self.brackets[bracket_id] = {
        type = "begin_clip",
        element_id = #self.elements+1;
        depth
    }
end

-- activate the clip group
-- stencil elements added between begin_clip and activate_clip form the clipper
-- following elements, until end_clip, define the clippee
-- clippee elements must either all be stencil elements or all painted elements
function scene_meta.__index.activate_clip(self, depth)
    local bracket_id = #self.brackets+1
    self.brackets[bracket_id] = {
        type = "activate_clip",
        element_id = #self.elements+1;
        depth
    }
end

-- finish the clipping group
-- elements added between activate_clip and end_clip form the clippee
-- clippee elements must either all be stencil elements or all painted elements
function scene_meta.__index.end_clip(self, depth)
    local bracket_id = #self.brackets+1
    self.brackets[bracket_id] = {
        type = "end_clip",
        element_id = #self.elements+1;
        depth
    }
end

-- begin a fade group
-- painted elements, until end_fade, have their transparency modulated
-- as a group (not individually, as this is not the same)
function scene_meta.__index.begin_fade(self, depth, opacity)
    local bracket_id = #self.brackets+1
    self.brackets[bracket_id] = {
        type = "begin_fade",
        element_id = #self.elements+1;
        depth,
        opacity
    }
end

-- ends a fade group
-- painted elements added between begin_fade and end_fade have their
-- transparency modulated as a group (not individually, as this is not the same)
function scene_meta.__index.end_fade(self, depth, opacity)
    local bracket_id = #self.brackets+1
    self.brackets[bracket_id] = {
        type = "end_fade",
        element_id = #self.elements+1;
        depth,
        opacity
    }
end

-- begin a blur group
-- painted elements, until end_blur, are blurred by a Gaussian filter
function scene_meta.__index.begin_blur(self, depth, radius)
    local bracket_id = #self.brackets+1
    self.brackets[bracket_id] = {
        type = "begin_blur",
        element_id = #self.elements+1;
        depth,
        radius
    }
end

-- end a blur group
-- painted elements added between begin_blur and end_blur have are
-- blurred by a Gaussian filter
function scene_meta.__index.end_blur(self, depth, radius)
    local bracket_id = #self.brackets+1
    self.brackets[bracket_id] = {
        type = "end_blur",
        element_id = #self.elements+1;
        depth,
        radius
    }
end

-- begin a transformation group
-- elements until end_transform are transformed
function scene_meta.__index.begin_transform(self, depth, xf)
    local bracket_id = #self.brackets+1
    self.brackets[bracket_id] = {
        type = "begin_transform",
        element_id = #self.elements+1;
        depth,
        xf
    }
end

-- end a transformation group
-- elements between begin_transform and end_transform are transformed
function scene_meta.__index.end_transform(self, depth, xf)
    local bracket_id = #self.brackets+1
    self.brackets[bracket_id] = {
        type = "end_transform",
        element_id = #self.elements+1;
        depth,
        xf
    }
end

-- iterate over scene contents
function scene_meta.__index.iterate(self, forward)
    assert(_M.is_scene(self), "not a scene")
    local b, e = 1, 1
    local elements = self.elements
    local last_element = #elements
    local brackets = self.brackets
    local last_bracket = #brackets
    local shapes = self.shapes
    local paints = self.paints
    while e <= last_element or b <= last_bracket do
        local bracket = brackets[b]
        if b <= last_bracket and bracket.element_id <= e then
            local callback = forward[bracket.type]
            if not callback then
                error("missing method '" .. bracket.type .. "'")
            end
            callback(forward, unpack(bracket))
            b = b + 1
        elseif e <= last_element then
            local element = elements[e]
            local callback = forward[element.type]
            if not callback then
                error("missing method '" .. element.type .. "'")
            end
            if element.type == "painted_element" then
                callback(forward, element.winding_rule,
                    shapes[element.shape_id], paints[element.paint_id])
            elseif element.type == "stencil_element" then
                callback(forward, element.winding_rule,
                    shapes[element.shape_id])
            end
            e = e + 1
        end
    end
end

-- creates and returns a new empty scene
function _M.scene()
    return setmetatable({
        shapes = {},
        paints = {},
        brackets = {},
        elements = {},
        xf = xform.identity(),
    }, scene_meta)
end

-- checkes if object is a scene
function _M.is_scene(s)
    return getmetatable(s) == scene_meta
end

-- make scene object xformable
xformable.setmethods(scene_meta.__index)

return _M
