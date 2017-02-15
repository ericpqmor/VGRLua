local table = require 'table'

function vertical_test_linear_segment(x1, y1, x2, y2, x, y)
  local valor = (y2 - y1)*x + (x1 - x2)*y - x1*(y2 - y1) - y1*(x1 - x2)
  return (x2 - x1)*valor < 0 and ((x1<=x and x<x2) or (x2<=x and x<x1))
end

local function LinearIntersection(x0,y0,x1,y1, xmin, ymin, xmax, ymax)
  if vertical_test_linear_segment(x0,y0,x1,y1,xmax, ymax) == true and vertical_test_linear_segment(x0,y0,x1,y1,xmax, ymin) == false then
    return true
  else
    return false
  end
end


local function CreateShortcuts(scene,data, bb)
local xmin, ymin, xmax, ymax = bb[1], bb[2], bb[3], bb[4]
local shortcuts = {}
for i = 1, #data do
  shortcuts[i] = {}
  if data[i] ~= nil then
    for j = 1, #data[i] then
      local instruction = data[i][j]
      local offset = scene.shapes[i].offsets[instruction]
      if scene.shapes[i].instructions[instruction] == 'linear_segment' then
        local x0, y0, x1, y1 = unpack(scene.shapes[i].data, offset, offset+3)
        if LinearIntersection (x0,y0,x1,y1,xmin,ymin, xmax, ymax) then
          if (x1-x0) > 0 then
            x0s, y0s, x1s, y1s =  x1, y1, x1, ymax
          else
            x0s, y0s, x1s, y1s = x0, ymax, x0, y0
          end
          shortcuts[i] = table.pack(unpack(shortcuts[i],x0s, y0s, x1s, y1s)
        end
      elseif scene.shapes[i].instructions[instruction] == 'end_open_contour' or if scene.shapes[i].instructions[instruction] == 'end_closed_contour' then
        local x0, y0, len = unpack(scene.shapes[i].data, offset, offset+2)
        local begin_off = scene.shapes[i].offsets[instruction-len]
        local x1, y1 = unpack(scene.shapes[i].data, begin_off+1, begin_off+2)
        if LinearIntersection (x0,y0,x1,y1,xmin,ymin, xmax, ymax) then
          if (x1-x0) > 0 then
            x0s, y0s, x1s, y1s =  x1, y1, x1, ymax
          else
            x0s, y0s, x1s, y1s = x0, ymax, x0, y0
          end
          shortcuts[i] = table.pack(unpack(shortcuts[i],x0s, y0s, x1s, y1s)
        end
      end
    end
  end
end
  return shortcuts --- tabela formato {shape1{shortcuts},shape2{}}
end


--
--
-- local function UpdateWinding(scene,son, father)
--
-- for i = 1, #father.data do
--   son.winding[i] = father.winding[i]
--   local notin = {}
--   for j = 1, #father.data
--
--   end
-- end
