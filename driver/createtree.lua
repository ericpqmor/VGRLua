


local function ShapeInsideScene(tree, camada, bb, read)
  local el_id = tree[camada].data


end

local function CreateTreeBranch(tree, camada, read)
local branch = { }
local n_camada = camada .. read
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
--
branch.data = { }
for k,el in pairs(tree[camada].data) do


end


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
  w_i = {},
  -- data = { --[[ element_id--]]},
  {}, -- 1
  {}, -- 2
 {}, -- 3
 {} -- 4
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
    tree = CreateTreeBranch(tree,camada,read+1)


  end
end




return tree

end
