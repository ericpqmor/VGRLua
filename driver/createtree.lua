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
  return (y2 - y1)*valor < 0 
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

function implicit_vertical_quadratic_test(x0,y0,x1,y1,x2,y2,x,y)
    local valor = 4*(y*x1-y1*x)*(y2*x1-y1*x2)-((2*y1-y2)*x+(x2-2*x1)*y)*((2*y1-y2)*x+(x2-2*x1)*y)
    local sign = 2*x2*(y1*x2 - y2*x1)
    if sign > 0 then valor = -valor end

    return valor > 0
end

function implicit_horizontal_quadratic_test(x0,y0,x1,y1,x2,y2,x,y)
    local valor = 4*(x*y1-x1*y)*(x2*y1-x1*y2)-((2*x1-x2)*y+(y2-2*y1)*x)*((2*x1-x2)*y+(y2-2*y1)*x)
    local sign = 2*y2*(x1*y2 - x2*y1)
    if sign > 0 then valor = -valor end

    return valor < 0
end

function vertical_quadratic_test(x0,y0,x1,y1,x2,y2,x,y,winding_rule,xmin,ymin,xmax,ymax,diagonal)
  local test = false
  if xmin <= x and x < xmax and y > ymin then
    if y >= ymax then test = true
    else
      if (y1-y0)*(x2-x0) == (y2-y0)*(x1-x0) then
        test = vertical_test_linear_segment(x0,y0,x2,y2,x,y)
      elseif vertical_test_linear_segment(x0,y0,x2,y2,x1,y1) == true then
        if vertical_test_linear_segment(x0,y0,x2,y2,x,y) == true then
          test = implicit_vertical_quadratic_test(0,0,x1-x0,y1-y0,x2-x0,y2-y0,x-x0,y-y0)
        else
          test = false
        end
      else
        if vertical_test_linear_segment(x0,y0,x2,y2,x,y) == true then
          test = true
        else
          test = implicit_vertical_quadratic_test(0,0,x1-x0,y1-y0,x2-x0,y2-y0,x-x0,y-y0)
        end
      end 
    end
  end

  return test
end

function horizontal_quadratic_test(x0,y0,x1,y1,x2,y2,x,y,winding_rule,xmin,ymin,xmax,ymax,diagonal)
  local test = false
  if ymin <= y and y < ymax and x <= xmax then
    if x <= xmin then test = true
    else
      if horizontal_test_linear_segment(x0,y0,x2,y2,x1,y1) == true then
        if horizontal_test_linear_segment(x0,y0,x2,y2,x,y) == true then
          --Degeneration
          if (y1-y0)*(x2-x0) == (y2-y0)*(x1-x0) then
            test = horizontal_test_linear_segment(x0,y0,x2,y2,x,y)
          else
            test = implicit_horizontal_quadratic_test(0,0,x1-x0,y1-y0,x2-x0,y2-y0,x-x0,y-y0)
          end
        else
          test = false
        end
      else
        if horizontal_test_linear_segment(x0,y0,x2,y2,x,y) == true then
          test = true
        else
          if (y1-y0)*(x2-x0) == (y2-y0)*(x1-x0) then
            test = horizontal_test_linear_segment(x0,y0,x2,y2,x,y)
          else
            test = implicit_horizontal_quadratic_test(0,0,x1-x0,y1-y0,x2-x0,y2-y0,x-x0,y-y0)
          end
        end
      end 
    end
  end

  return test
end

function intersects_linear_segment(xmin,ymin,xmax,ymax,x0,y0,x1,y1)
  if vertical_linear_test(x0,y0,x1,y1,xmax,ymax,xmin,ymin,xmax,ymax) == true and vertical_linear_test(x0,y0,x1,y1,xmax,ymin,xmin,ymin,xmax,ymax) == false then
    return true
  elseif vertical_linear_test(x0,y0,x1,y1,xmin,ymax,xmin,ymin,xmax,ymax) == true and vertical_linear_test(x0,y0,x1,y1,xmin,ymin,xmin,ymin,xmax,ymax) == false then
    return true
  elseif horizontal_linear_test(x0,y0,x1,y1,xmin,ymin,xmin,ymin,xmax,ymax) == true and horizontal_linear_test(x0,y0,x1,y1,xmin,ymax,xmin,ymin,xmax,ymax) == false then
    return true
  elseif horizontal_linear_test(x0,y0,x1,y1,xmax,ymin,xmin,ymin,xmax,ymax) == true and horizontal_linear_test(x0,y0,x1,y1,xmax,ymax,xmin,ymin,xmax,ymax) == false then
    return true
  else
    return false
end

function intersects_quadratic_segment(xmin,ymin,xmax,ymax,x0,y0,x1,y1,x2,y2)
  if vertical_quadratic_test(x0,y0,x1,y1,x2,y2,xmax,ymax,xmin,ymin,xmax,ymax) == true and vertical_quadratic_test(x0,y0,x1,y1,x2,y2,xmax,ymin,xmin,ymin,xmax,ymax) == false then
    return true
  elseif vertical_quadratic_test(x0,y0,x1,y1,x2,y2,xmin,ymax,xmin,ymin,xmax,ymax) == true and vertical_quadratic_test(x0,y0,x1,y1,x2,y2,xmin,ymin,xmin,ymin,xmax,ymax) == false then
    return true
  elseif horizontal_quadratic_test(x0,y0,x1,y1,x2,y2,xmin,ymin,xmin,ymin,xmax,ymax) == true and horizontal_quadratic_test(x0,y0,x1,y1,x2,y2,xmin,ymax,xmin,ymin,xmax,ymax) == false then
    return true
  elseif horizontal_quadratic_test(x0,y0,x1,y1,x2,y2,xmax,ymin,xmin,ymin,xmax,ymax) == true and horizontal_quadratic_test(x0,y0,x1,y1,x2,y2,xmax,ymax,xmin,ymin,xmax,ymax) == false then
    return true
  else
    return false
end

--[[

]]--

function ShapeInsideScene(new_scene, shape)
end

function subdivide(scene, maxdepth, maxseg)
  local boundingbox = scene.boundingBox
  scene.child = {}
  for i=1,4 do
    new_scene = scene.scene()

    new_scene.id = scene.id .. i
    scene.child[new_scene.id] = new_scene

    local xmin,ymin,xmax,ymax = createBoundingBox(boundingBox, i)
    new_scene.boundingBox = {xmin,ymin,xmax,ymax}

    new_scene.depth = scene.depth + 1
    new_scene.segments = 0

    for i=1, #scene.shapes do
      local shape = scene.shapes[i]
      if ShapeInsideScene(new_scene, shape) == true then
        new_scene.shapes[#new_scene.shapes+1] = shape
        new_scene.segments = new_scene.segments + 1
      end
    end

    if isLeaf(new_scene) == true then
      return 
    else
      subdivide(new_scene, maxdepth, maxseg)
    end
  end
end



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

local function createBoundingBox(bb, read)
  local xmin, xmax, ymin, ymax
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

  return xmin,xmax,ymin,ymax
end

local function CreateTreeBranch(scene, tree, camada, read, maxseg)
  local branch = { }
  -- Cria o bounding box
  local boundingBox = tree[camada].bb
  local Dx = (bb.xmax-bb.xmin)/2
  local Dy = (bb.ymax-bb.ymin)/2
  local xmax, xmin, ymax, ymin = createBoundingBox(boundingBox, read)
  branch.bb = {xmax = xmax, xmin = xmin, ymax = ymax, ymin = ymin}
  --Cria a base de dados do galho
  branch.data = { }
  branch.numseg = 0
  for k,el in pairs(tree[camada].data) do
    if ShapeInsideScene(scene, branch.bb, scene.shapes[k]) == true then
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
w_i = incremento no winding number ]]--

--[[
local function subdivide(branch)
  local xmin, ymin, xmax, ymax = unpack(branch.boundingbox, 1, 4)

  for int i=1,4,1 do
    local subBranch
    subBranch.boundingBox = createBoundingBox(branch.boundingbox, i)
    subBranch.leaf = false
    subBranch.shapes = {}
    for int i=1,#branch.shapes do
      if 
  end
end
]]--
local function CreateTree(scene, viewport, maxdepth, maxseg)
  -- dá unpack no viewport para o bb da camada 0
  local vxmin, vymin, vxmax, vymax = unpack(viewport, 1, 4)
  local elements = scene.elements
  local tree = { }

  --[[ Aqui, inicializa a camada 0, que é a imagem inteira, logo, carrega todos
  os shapes e seus elementos em data--]]

  tree['0'] = {
    bb = {xmax = vxmax, xmin = vxmin, ymax = vymax, ymin = vymin},
    leaf = false,
    read = 0,
    depth = 0,
    w_i = {}
    -- data = { --[[ element_id--]]},
  }
  tree['0'].data = {}
  local numseg = 0
  for i = 1, #elements do
    table.insert(tree['0'].data,i)
    table.insert(tree['0'].w_i,0)
    local instructions = scene.shapes[i].instructions
    for j = 1, #instructions do
      table.insert(tree['0'].data[i], j)
      if instructions[j] == 'begin_open_contour' or instructions[j] == 'begin_closed_contour' then
      else
        numseg = numseg  + 1
      end
    end
  end
  -- Profundidade atual
  local idepth = tree['0'].depth -- 0
  local read = tree['0'].read -- 0
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
