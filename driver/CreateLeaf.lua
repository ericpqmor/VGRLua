local table = require "table"
local scenelib = require "scene"
local path = require "path"

---- AQUI CRIA O SEGMENTO LINEAR NA CENA NOVA
local function NLinearSegment(x0,y0, x1, y1, shape, bb, offsetit)
  --- Aqui entra se o segmento intersecta a lateral direita
  if x0 >= bb.xmax or x1 >= bb.xmax then
    local sentido = (x1-x0)
    if sentido < 0 then
      table.insert(shape.instructions, 'linear_segment') -- cria a instruc
      table.insert(shape.offsets, offsetit) -- define seu offset
      table.insert(shape.data, x0) -- adiciona os dados
      table.insert(shape.data, bb.ymax)
      offsetit = offsetit + 2 -- move o contador do offset
    end
    table.insert(shape.instructions, 'linear_segment')
    table.insert(shape.offsets, offsetit)
    table.insert(shape.data, x0)
    table.insert(shape.data, y0)
    offsetit = offsetit + 2
    if sentido > 0 then
      table.insert(shape.instructions, 'linear_segment') -- cria a instruc
      table.insert(shape.offsets, offsetit) -- define seu offset
      table.insert(shape.data, x1)
      table.insert(shape.data, y1)
      table.insert(shape.data, x1)
      table.insert(shape.data, bb.ymax)
      offsetit = offsetit + 4 -- move o contador do offset
    end
    -- se o segmento intersecta a lateral esquerda
  elseif x0 < bb.xmin or x1 < bb.xmin then
    table.insert(shape.instructions, 'linear_segment') -- cria a instruc
    table.insert(shape.offsets, offsetit) -- define seu offset
    table.insert(shape.data, x0)
    table.insert(shape.data, y0)
    table.insert(shape.data, x1)
    table.insert(shape.data, y1)
    offsetit = offsetit + 4 -- move o contador do offset
    --- se não intersecta nenhuma lateral simplesmente cria o segmento
  else
    table.insert(shape.instructions, 'linear_segment') -- cria a instruc
    table.insert(shape.offsets, offsetit) -- define seu offset
    table.insert(shape.data, x0) -- adiciona os dados
    table.insert(shape.data, y0)
    offsetit = offsetit + 2
  end
  return shape, offsetit
end





----- AQUI COMEÇA A FUNÇÃO
local function CreateLeaf(cena_grande, folha)
-- cena_grande = cena recebida no accelerate
local bb = folha.bb
scene = scenelib.scene() -- cria a nova cena
for k in pairs(folha.data) do -- itera nos shapes contidos pela folha
  local offsetit = 1
  shape = path.path() -- Cria um novo path
  data = cena_grande.shapes[k].data -- pega os dados do shape antigo
  offsets = cena_grande.shapes[k].offsets -- pega os offsets do shape antigo
  instructions = cena_grande.shapes[k].instructions -- pega as instruc do shape antigo
  paint = cena_grande.paints[k]
  element = cena_grande.elements[k]
  element.shape_id = k
  element.paint_id = k
  for j in pairs(folha.data[k]) do -- itera nas instructions contidas pela folha
    instruction = instructions[j] -- recebe a instrução
    offset = offsets[j] -- recebe o offset dessa instruc
    if instruction == 'linear_segment' then
      x0,y0,x1,y1 = unpack(data, offset, offset+3)
      shape, offsetit = NLinearSegment(x0,y0,x1,y1, shape, bb, offsetit)
    elseif instruction == 'end_closed_contour' or instruction == 'end_open_contour' then
      x0, y0, len = unpack(data, offset, offset+2)
      local offset_begin = offsets[j] - len
      x1, y1 = data[offset_begin+ 1], data[offset_begin+ 2]
      shape, offsetit = NLinearSegment(x0,y0,x1,y1, shape, bb, offsetit)
    end
  end
  -- passa as informações do shape para a cena nova
  table.insert(scene.shapes, shape)
  table.insert(scene.elements, element)
  table.insert(scene.paints, paint)
end

return scene

end
