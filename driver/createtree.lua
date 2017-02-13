


local function InsideScene()



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
  w_i = 0,
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
local idepth = tree[0].depth
local read = tree[0].read
tree[0].numseg = numseg
--[[Aqui, inicia o loop no resto das camadas, o critério de parada é
a profundidade ser zero e todas as suas subdivisões terem sido criadas --]]
while (idepth ~= 0) or (idepth == 0 and read ~= 4) do




end




return tree

end
