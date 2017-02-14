local table = require"table"

local function inside(x, y, xmin, ymin, xmax, ymax)
if y <= ymax and y >= ymin and x <= xmax  and x >= xmin then
		return true
else
		return false
end

function vertical_test_linear_segment(x1, y1, x2, y2, x, y)
  local valor = (y2 - y1)*x + (x1 - x2)*y - x1*(y2 - y1) - y1*(x1 - x2)
  return (x2 - x1)*valor < 0 
end

function horizontal_test_linear_segment(x1, y1, x2, y2, x, y)
  local valor = (y2 - y1)*x + (x1 - x2)*y - x1*(y2 - y1) - y1*(x1 - x2)
  return (x2 - x1)*valor < 0 
end

function vertical_linear_test(x0,y0,x1,y1,x,y,xmin,ymin,xmax,ymax)
  if xmin <= x and x <= xmax and y >= ymin then 
    if y > ymax then return true
    else return test_linear_segment(x0, y0, x1, y1, x, y) end
  end

  return false
end

function horizontal_linear_test(x0,y0,x1,y1,x,y,xmin,ymin,xmax,ymax)
  if ymin <= y and y <= ymax and x <= xmax then
    if x < xmin then return true
    else return test_linear_segment(x0, y0, x1, y1, x, y) end
  end

  return false
end

function intersects_linear_segment(xmin,ymin,xmax,ymax,x0,y0,x1,y1)
  if vertical_linear_test(x0,y0,xclose,yclose,xmax,ymax,xmin,ymin,xmax,ymax) == true and vertical_linear_test(x0,y0,xclose,yclose,xmax,ymin,xmin,ymin,xmax,ymax) == false then
    return true
  elseif vertical_linear_test(x0,y0,xclose,yclose,xmin,ymax,xmin,ymin,xmax,ymax) == true and vertical_linear_test(x0,y0,xclose,yclose,xmin,ymin,xmin,ymin,xmax,ymax) == false then
    return true
  elseif horizontal_linear_test(x0,y0,xclose,yclose,xmin,ymin,xmin,ymin,xmax,ymax) == true and horizontal_linear_test(x0,y0,xclose,yclose,xmin,ymax,xmin,ymin,xmax,ymax) == false then
    return true
  elseif horizontal_linear_test(x0,y0,xclose,yclose,xmax,ymin,xmin,ymin,xmax,ymax) == true and horizontal_linear_test(x0,y0,xclose,yclose,xmax,ymax,xmin,ymin,xmax,ymax) == false then
    return true
  else
    return false
end

local function ShapeInsideScene(scene,bb,shape)
  local xmax, xmin ,ymax, ymin = unpack(shape.supdata)
  -- Converte não-paths para paths
  if shape.type ~= "path" then shape = shape:as_path(shape, shape.xf) end

  local new_path = path.path()

  --Por enquanto vou percorrer na mão mesmo por aqui. Se quiser um path.iterate(), a gente discute depois
  --Também não vou implementar sua ideia dos bcc e occ
  local begin = false
  local dat = 1
  local xclose, yclose

  for i=1,#path.instructions do
    local instruction = path.instructions[i]

    if instruction == "begin_closed_contour" or instruction == "begin_open_contour" then
      begin = true
      dat = dat + 1
    end
            
    if instruction == "end_open_contour" or instruction == "end_closed_contour" then
      local x0,y0 = unpack(path.data,dat,dat+1)
      local isInside = false

      --Primeiro, testo os endpoints
      if inside(x0,y0,xmin,ymin,xmax,ymax) == true or inside(xclose,yclose,xmin,ymin,xmax,ymax) == true then isInside = true
      --Agora testa as intersecções. Perceba que testo todas as arestas.
      elseif intersects_linear_segment(xmin,ymin,xmax,ymax,x0,y0,xclose,yclose) == true then
          isInside = true
      end

      --
      if isInside == true then
        path.push_data(new_path,x0,y0)
        --Não entendi esse Rewind (que tem na função push_instruction em path.lua). Depois me avisa
        path.push_instruction(new_path, instruction)
      end

      cont = cont + 1
      dat = dat + 3
    end

    if instruction == "linear_segment" then
      local x0,y0,x1,y1 = unpack(path.data,dat,dat+3)
      if begin == true then
        xclose, yclose = x0, y0
        begin = false
      end

      wind_num = wind_num + path.winding[cont](x,y,element.winding_rule,cont)
      cont = cont + 1
      dat = dat + 2
    end

    if instruction == "cubic_segment" then
      local x0,y0,x1,y1,x2,y2,x3,y3 = unpack(path.data,dat,dat+7)
      if begin == true then
        xclose, yclose = x0, y0
        begin = false
      end
      wind_num = wind_num + path.winding[cont](x,y,element.winding_rule,cont,x0,y0,x1,y1,x2,y2,x3,y3)
      cont = cont + 1
      dat = dat + 6
    end
      
    if instruction == "quadratic_segment" then
      local x0,y0,x1,y1,x2,y2 = unpack(path.data,dat,dat+5)
      if begin == true then
        xclose, yclose = x0, y0
        begin = false
      end
      wind_num = wind_num + path.winding[cont](x,y,element.winding_rule,cont)
      cont = cont + 1
      dat = dat + 4   
    end

    if instruction == "rational_quadratic_segment" then
      local x0,y0,x1,y1,w1,x2,y2 = unpack(path.data,dat,dat+6)
      if begin == true then
        xclose, yclose = x0, y0
        begin = false
      end
      wind_num = wind_num + path.winding[cont](x,y,element.winding_rule,cont)
      cont = cont + 1
      dat = dat + 5
    end
  end

  return wind_num
end


  --[[
  -- gambiarra que tou fazendo aqui pra testar se o shape tá dentro da cena
  local x1, y1 = xmin, ymin
  local x2, y2 = xmax, ymin
  local x3, y3 = xmin, ymax
  local x4, y4 = xmax, ymax
  xmax, xmin, ymax, ymin = bb.xmax, bb.xmin, bb.ymax, bb.xmin
  if Inside(x1, y1, xmax, xmin, ymax, ymin) == true then
    return true
  elseif Inside(x2, y2, xmax, xmin, ymax, ymin) == true then
    return true
  elseif Inside(x3, y3, xmax, xmin, ymax, ymin) == true then
    return true
  elseif Inside(x4, y4, xmax, xmin, ymax, ymin) == true then
    return true
  end
  ]]--


  -- outra gambiarra aqui, a cena pode ser contida totalmente pelo shape
  local bbx1, bby1 = xmin, ymin
  local bbx2, bby2 = xmax, ymin
  local bbx3, bby3 = xmin, ymax
  local bbx4, bby4 = xmax, ymax
  xmax, xmin ,ymax, ymin = unpack(shape.supdata)
  if inside(bbx1, bby1, xmin, ymin, xmax, ymax) == true then
    return true
  elseif inside(bbx2, bby2, xmin, ymin, xmax, ymax) == true then
    return true
  elseif inside(bbx3, bby3, xmin, ymin, xmax, ymax) == true then
    return true
  elseif inside(bbx4, bby4, xmin, ymin, xmax, ymax) == true then
    return true
  end
  return false
end

local function CreateTreeBranch(scene, tree, camada, read, maxseg)
local branch = { }
-- Cria o bounding box
local bb = tree[camada].bb
local Dx = (bb.xmax-bb.xmin)/2
local Dy = (bb.ymax-bb.ymin)/2
local xmax, xmin, ymax, ymin
if read == 1 then
  xmax = bb.xmin + Dx
  xmin = bb.xmin
  ymax = bb.ymin + 2*Dy
  ymin = bb.ymin + Dy
elseif read == 2 then
  xmax = bb.xmin + 2*Dx
  xmin = bb.xmin + Dx
  ymax = bb.ymin + 2*Dy
  ymin = bb.ymin + Dy
elseif read == 3 then
  xmax = bb.xmin + Dx
  xmin = bb.xmin
  ymax = bb.ymin + Dy
  ymin = bb.ymin
elseif read == 4 then
  xmax = bb.xmin + 2*Dx
  xmin = bb.xmin + Dx
  ymax = bb.ymin + Dy
  ymin = bb.ymin
end
branch.bb = {xmax = xmax, xmin = xmin, ymax = ymax, ymin = ymin}
--Cria a base de dados do galho
branch.data = { }
branch.numseg = 0
for k,el in pairs(tree[camada].data) do
  if ShapeInsideScene(scene, branch.bb, scene.shapes[k]) == true then
    -- retorna uma table com os seg que pertencem (a implementar)
    branch.data[k] = SegmentInsideScene()
    branch.numseg = branch.numseg + table.getn(branch.data[k])
  end
end
-- Cria a flag leaf
if branch.numseg > maxseg then
  branch.leaf == false
else
  branch.leaf == true
end
-- Cria os w_i
branch.w_i = { } -- tem que pensar nessa parte
-- Cria o depth
branch.depth = tree[camada].depth + 1


return branch

end
--[[Esquema da árvore
Cada folha/galho carrega:
bb = seu boundingbox
leaf = flag que diz se é uma folha ou galho
read = flag que diz que se for galho, qual foi a ultima subdivisão criada. Ex:
se o galho não criou nenhuma subdivisão, read = 0, se criou sua terceira sub-
divisão, read = 3, com no máximo 4.
data = tabela que carrega os element_id que estão contidos neste galho/folha e
cada element_id é uma tabela que contém as instruções dos segmentos que estão
contidos nesta cena, os dados serão chamados utilizando os offsets referentes
depth = profundidade da camada
w_i = incremento no winding number --]]
local function CreateTree(scene, viewport, maxdepth, maxseg)
-- dá unpack no viewport para o bb da camada 0
local vxmin, vymin, vxmax, vymax = unpack(viewport, 1, 4)
local elements = scene.elements
local tree = { }

--[[ Aqui, inicializa a camada 0, que é a imagem inteira, logo, carrega todos
os shapes e seus elementos em data--]]

tree[0] = {
  bb = {xmax = vxmax, xmin = vxmin, ymax = vymax, ymin = vymin},
  leaf = false,
  read = 0,
  depth = 0,
  w_i = {}
  -- data = { --[[ element_id--]]},
}
tree[0].data = {}
local numseg = 0
for i = 1, #elements do
  table.insert(tree[0].data,i)
  table.insert(tree[0].w_i,0)
  local instructions = scene.shapes[i].instructions
  for j = 1, #instructions do
    table.insert(tree[0].data[i], j)
    if instructions[j] == 'begin_open_contour' or instructions[j] == 'begin_closed_contour' then
    else
      numseg = numseg  + 1
    end
  end
end
-- Profundidade atual
local idepth = tree[0].depth -- 0
local read = tree[0].read -- 0
local camada = '0'
tree[camada].numseg = numseg

--[[Aqui, inicia o loop no resto das camadas, o critério de parada é
a profundidade ser zero e todas as suas subdivisões terem sido criadas --]]
while (idepth ~= 0) or (idepth == 0 and read ~= 4) do
  local numseg = tree[camada].numseg
  if numseg > maxseg and tree[camada].read < 4 then
    tree[camada].read = read + 1
    tree[camada].leaf = false
    local n_camada = camada .. tree[camada].read
    -- Cria um galho  
    tree[n_camada] = CreateTreeBranch(scene,tree,camada,read+1, maxseg)
  end
end

  return tree
end
