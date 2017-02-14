local table = require "table"
local scenelib = require "scene"
local path = require "path"
local png = require "png"

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

---- AQUI CRIA O SEGMENTO QUADRATICO DA NOVA cena
local function NQuadraticSegment(x0,y0, x1, y1,x2,y2, shape, bb, offsetit)
  --- Aqui entra se o segmento intersecta a lateral direita
  if x0 >= bb.xmax or x2 >= bb.xmax then
    local sentido = (x2-x0)
    if sentido < 0 then
      table.insert(shape.instructions, 'linear_segment') -- cria a instruc
      table.insert(shape.offsets, offsetit) -- define seu offset
      table.insert(shape.data, x0) -- adiciona os dados
      table.insert(shape.data, bb.ymax)
      offsetit = offsetit + 2 -- move o contador do offset
    end
    table.insert(shape.instructions, 'quadratic_segment')
    table.insert(shape.offsets, offsetit)
    table.insert(shape.data, x0)
    table.insert(shape.data, y0)
    table.insert(shape.data, x1)
    table.insert(shape.data, y1)
    offsetit = offsetit + 4
    if sentido > 0 then
      table.insert(shape.instructions, 'linear_segment') -- cria a instruc
      table.insert(shape.offsets, offsetit) -- define seu offset
      table.insert(shape.data, x2)
      table.insert(shape.data, y2)
      table.insert(shape.data, x2)
      table.insert(shape.data, bb.ymax)
      offsetit = offsetit + 4 -- move o contador do offset
    end
    -- se o segmento intersecta a lateral esquerda
  elseif x0 < bb.xmin or x2 < bb.xmin then
    table.insert(shape.instructions, 'quadratic_segment') -- cria a instruc
    table.insert(shape.offsets, offsetit) -- define seu offset
    table.insert(shape.data, x0)
    table.insert(shape.data, y0)
    table.insert(shape.data, x1)
    table.insert(shape.data, y1)
    table.insert(shape.data, x2)
    table.insert(shape.data, y2)
    offsetit = offsetit + 6 -- move o contador do offset
    --- se não intersecta nenhuma lateral simplesmente cria o segmento
  else
    table.insert(shape.instructions, 'quadratic_segment') -- cria a instruc
    table.insert(shape.offsets, offsetit) -- define seu offset
    table.insert(shape.data, x0) -- adiciona os dados
    table.insert(shape.data, y0)
    table.insert(shape.data, x1)
    table.insert(shape.data, y1)
    offsetit = offsetit + 4
  end
  return shape, offsetit
end


---- AQUI CRIA O SEGMENTO RACIONAL QUADRATICO DA NOVA cena
local function NRatQuadraticSegment(x0,y0, x1, y1, w,x2,y2, shape, bb, offsetit)
  --- Aqui entra se o segmento intersecta a lateral direita
  if x0 >= bb.xmax or x2 >= bb.xmax then
    local sentido = (x2-x0)
    if sentido < 0 then
      table.insert(shape.instructions, 'linear_segment') -- cria a instruc
      table.insert(shape.offsets, offsetit) -- define seu offset
      table.insert(shape.data, x0) -- adiciona os dados
      table.insert(shape.data, bb.ymax)
      offsetit = offsetit + 2 -- move o contador do offset
    end
    table.insert(shape.instructions, 'rational_quadratic_segment')
    table.insert(shape.offsets, offsetit)
    table.insert(shape.data, x0)
    table.insert(shape.data, y0)
    table.insert(shape.data, x1)
    table.insert(shape.data, y1)
    table.insert(shape.data, w)
    offsetit = offsetit + 5
    if sentido > 0 then
      table.insert(shape.instructions, 'linear_segment') -- cria a instruc
      table.insert(shape.offsets, offsetit) -- define seu offset
      table.insert(shape.data, x2)
      table.insert(shape.data, y2)
      table.insert(shape.data, x2)
      table.insert(shape.data, bb.ymax)
      offsetit = offsetit + 4 -- move o contador do offset
    end
    -- se o segmento intersecta a lateral esquerda
  elseif x0 < bb.xmin or x2 < bb.xmin then
    table.insert(shape.instructions, 'rational_quadratic_segment') -- cria a instruc
    table.insert(shape.offsets, offsetit) -- define seu offset
    table.insert(shape.data, x0)
    table.insert(shape.data, y0)
    table.insert(shape.data, x1)
    table.insert(shape.data, y1)
    table.insert(shape.data, w)
    table.insert(shape.data, x2)
    table.insert(shape.data, y2)
    offsetit = offsetit + 7 -- move o contador do offset
    --- se não intersecta nenhuma lateral simplesmente cria o segmento
  else
    table.insert(shape.instructions, 'rational_quadratic_segment') -- cria a instruc
    table.insert(shape.offsets, offsetit) -- define seu offset
    table.insert(shape.data, x0) -- adiciona os dados
    table.insert(shape.data, y0)
    table.insert(shape.data, x1)
    table.insert(shape.data, y1)
    table.insert(shape.data, w)
    offsetit = offsetit + 5
  end
  return shape, offsetit
end

---- AQUI CRIA O SEGMENTO RACIONAL QUADRATICO DA NOVA cena
local function NCubicSegment(x0,y0, x1, y1, x2,y2,x3,y3, shape, bb, offsetit)
  --- Aqui entra se o segmento intersecta a lateral direita
  if x0 >= bb.xmax or x3 >= bb.xmax then
    local sentido = (x3-x0)
    if sentido < 0 then
      table.insert(shape.instructions, 'linear_segment') -- cria a instruc
      table.insert(shape.offsets, offsetit) -- define seu offset
      table.insert(shape.data, x0) -- adiciona os dados
      table.insert(shape.data, bb.ymax)
      offsetit = offsetit + 2 -- move o contador do offset
    end
    table.insert(shape.instructions, 'cubic_segment')
    table.insert(shape.offsets, offsetit)
    table.insert(shape.data, x0)
    table.insert(shape.data, y0)
    table.insert(shape.data, x1)
    table.insert(shape.data, y1)
    table.insert(shape.data, x2)
    table.insert(shape.data, y2)
    offsetit = offsetit + 6
    if sentido > 0 then
      table.insert(shape.instructions, 'linear_segment') -- cria a instruc
      table.insert(shape.offsets, offsetit) -- define seu offset
      table.insert(shape.data, x3)
      table.insert(shape.data, y3)
      table.insert(shape.data, x3)
      table.insert(shape.data, bb.ymax)
      offsetit = offsetit + 4 -- move o contador do offset
    end
    -- se o segmento intersecta a lateral esquerda
  elseif x0 < bb.xmin or x3 < bb.xmin then
    table.insert(shape.instructions, 'cubic_segment') -- cria a instruc
    table.insert(shape.offsets, offsetit) -- define seu offset
    table.insert(shape.data, x0)
    table.insert(shape.data, y0)
    table.insert(shape.data, x1)
    table.insert(shape.data, y1)
    table.insert(shape.data, x2)
    table.insert(shape.data, y2)
    table.insert(shape.data, x3)
    table.insert(shape.data, y3)
    offsetit = offsetit + 8 -- move o contador do offset
    --- se não intersecta nenhuma lateral simplesmente cria o segmento
  else
    table.insert(shape.instructions, 'cubic_segment') -- cria a instruc
    table.insert(shape.offsets, offsetit) -- define seu offset
    table.insert(shape.data, x0) -- adiciona os dados
    table.insert(shape.data, y0)
    table.insert(shape.data, x1)
    table.insert(shape.data, y1)
    table.insert(shape.data, x2)
    table.insert(shape.data, y2)
    offsetit = offsetit + 6
  end
  return shape, offsetit
end


----- AQUI COMEÇA A FUNÇÃO
local function CreateLeaf(cena_grande, folha)
-- cena_grande = cena recebida no accelerate
local bb = folha.bb
local shapeit = 0
scene = scenelib.scene() -- cria a nova cena
for k in pairs(folha.data) do -- itera nos shapes contidos pela folha
  local offsetit = 1
  shapeit = shapeit + 1
  local shape = path.path() -- Cria um novo path
  local data = cena_grande.shapes[k].data -- pega os dados do shape antigo
  local offsets = cena_grande.shapes[k].offsets -- pega os offsets do shape antigo
  local instructions = cena_grande.shapes[k].instructions -- pega as instruc do shape antigo
  local paint = cena_grande.paints[k]
  local element = cena_grande.elements[k]
  element.shape_id = shapeit
  element.paint_id = shapeit
  for j in pairs(folha.data[k]) do -- itera nas instructions contidas pela folha
    instruction = instructions[j] -- recebe a instrução
    offset = offsets[j] -- recebe o offset dessa instruc
    if instruction == 'linear_segment' then
      local x0,y0,x1,y1 = unpack(data, offset, offset+3)
      shape, offsetit = NLinearSegment(x0,y0,x1,y1, shape, bb, offsetit)
    elseif instruction == 'quadratic_segment' then
      local x0,y0,x1,y1,x2, y2 = unpack(data, offset, offset+5)
      shape, offsetit = NQuadraticSegment(x0,y0,x1,y1,x2, y2, shape, bb, offsetit)
    elseif instruction == 'rational_quadratic_segment' then
      local x0,y0,x1,y1,w,x2,y2 = unpack(data, offset, offset+6)
      shape, offsetit = NRatQuadraticSegment(x0,y0,x1,y1,w,x2,y2,shape,bb,offsetit)
    elseif instruction == 'cubic_segment' then
      local x0,y0,x1,y1,x2,y2, x3, y3 = unpack(data, offset, offset+7)
      shape, offsetit = NCubicSegment(x0,y0,x1,y1,x2,y2,x3,y3,shape,bb,offsetit)
    elseif instruction == 'end_closed_contour' or instruction == 'end_open_contour' then
      x0, y0, len = unpack(data, offset, offset+2)
      local offset_begin = offsets[j] - len
      x1, y1 = data[offset_begin+ 1], data[offset_begin+ 2]
      shape, offsetit = NLinearSegment(x0,y0,x1,y1, shape, bb, offsetit)
    end
  end
  local shape.w_i = png.wind(cena_grande, k, bb.xmax, bb.ymin)
  -- passa as informações do shape para a cena nova
  table.insert(scene.shapes, shape)
  table.insert(scene.elements, element)
  table.insert(scene.paints, paint)
end

return scene

end
