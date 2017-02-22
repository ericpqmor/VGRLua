local driver = require"driver"
local image = require"image"
local chronos = require"chronos"
local bezier = require"bezier"
local filter = require"filter"
local xform = require"xform"
local xformable = require"xformable"
local scene = require"scene"
local path = require"path"
local quadratic = require"quadratic"
local solvequadratic = quadratic.quadratic
local util = require"util"
local bezier = require"bezier"
local blue = require"blue"
-- Output formatted string to stderr
local function stderr(...)
    io.stderr:write(string.format(...))
end

local unpack = unpack or table.unpack
local floor = math.floor

-- Create driver with all Lua functions needed to build
-- the scene description
local _M = driver.new()


-- This is one of the functions you must implement. It
-- receives a scene and a viewport. It returns an
-- acceleration datastructure that contains all scene
-- information in a form that enables fast sampling.
-- For now, it simply returns the scene itself.

function accel_circle(scene, shape)

    local transformed = scene.xf*shape.xf
    shape.transf = transformed:inverse()
    shape.boundingBox = {shape.cx-shape.r,shape.cy-shape.r,shape.cx+shape.r,shape.cy+shape.r}

    return viewport
end

function insideBoundingBox(xmin, ymin, xmax, ymax, x, y)
	return xmin <= x and x < xmax and ymin <= y and y < ymax
end

----------------------------------------
--[[ 	LINEAR FUNCTIONS 			]]--
----------------------------------------
function horizontal_test_linear_segment(x1, y1, x2, y2, x, y)
	local valor = (y2 - y1)*x + (x1 - x2)*y - x1*(y2 - y1) - y1*(x1 - x2)
	return (y2 - y1)*valor < 0
end

function horizontal_linear_test(x0,y0,x1,y1,x,y,xmin,ymin,xmax,ymax)
	local test = false
	local wind_num = 0

	if ymin <= y and y < ymax and x < xmax then
		if x <= xmin then test = true
		else test = horizontal_test_linear_segment(x0, y0, x1, y1, x, y) end
	end

	return test
end

function vertical_test_linear_segment(x1, y1, x2, y2, x, y)
  local valor = (y2 - y1)*x + (x1 - x2)*y - x1*(y2 - y1) - y1*(x1 - x2)
  return (x2 - x1)*valor < 0
end

function vertical_linear_test(x0,y0,x1,y1,x,y,xmin,ymin,xmax,ymax)
  local test = false
  local wind_num = 0

  if xmin <= x and x < xmax and y >= ymin then
    if y >= ymax then test = true
    else test = vertical_test_linear_segment(x0, y0, x1, y1, x, y) end
  end

  return test
end

function intersects_linear_segment(xmin,ymin,xmax,ymax,x0,y0,x1,y1)
  if vertical_linear_test(x0,y0,x1,y1,xmax,ymax,math.min(x0,x1),math.min(y0,y1),math.max(x0,x1),math.max(y0,y1)) == true and
  	vertical_linear_test(x0,y0,x1,y1,xmax,ymin,math.min(x0,x1),math.min(y0,y1),math.max(x0,x1),math.max(y0,y1)) == false then
    return true
  elseif vertical_linear_test(x0,y0,x1,y1,xmin,ymax,math.min(x0,x1),math.min(y0,y1),math.max(x0,x1),math.max(y0,y1)) == true and
  	vertical_linear_test(x0,y0,x1,y1,xmin,ymin,math.min(x0,x1),math.min(y0,y1),math.max(x0,x1),math.max(y0,y1)) == false then
    return true
  elseif horizontal_linear_test(x0,y0,x1,y1,xmin,ymin,math.min(x0,x1),math.min(y0,y1),math.max(x0,x1),math.max(y0,y1)) == true and
  	horizontal_linear_test(x0,y0,x1,y1,xmax,ymin,math.min(x0,x1),math.min(y0,y1),math.max(x0,x1),math.max(y0,y1)) == false then
    return true
  elseif horizontal_linear_test(x0,y0,x1,y1,xmax,ymin,math.min(x0,x1),math.min(y0,y1),math.max(x0,x1),math.max(y0,y1)) == true and
  	horizontal_linear_test(x0,y0,x1,y1,xmax,ymax,math.min(x0,x1),math.min(y0,y1),math.max(x0,x1),math.max(y0,y1)) == false then
    return true
  else
    return false
  end
end

----------------------------------------
--[[ 	QUADRATICS FUNCTIONS 		]]--
----------------------------------------
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

function vertical_quadratic_test(x0,y0,x1,y1,x2,y2,x,y,xmin,ymin,xmax,ymax,diagonal)
  local test = false
  local rec = false
  if x == 145.5 and y == 75.5 then rec = true end
  if xmin <= x and x < xmax and y >= ymin then
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

function horizontal_quadratic_test(x0,y0,x1,y1,x2,y2,x,y,xmin,ymin,xmax,ymax,diagonal)
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

function intersects_quadratic_segment(xmin,ymin,xmax,ymax,x0,y0,x1,y1,x2,y2)
  if vertical_quadratic_test(x0,y0,x1,y1,x2,y2xmax,ymax,math.min(x0,x1),math.min(y0,y1),math.max(x0,x1),math.max(y0,y1)) == true and
    vertical_quadratic_test(x0,y0,x1,y1,x2,y2xmax,ymin,math.min(x0,x1),math.min(y0,y1),math.max(x0,x1),math.max(y0,y1)) == false then
    return true
  elseif vertical_quadratic_test(x0,y0,x1,y1,x2,y2xmin,ymax,math.min(x0,x1),math.min(y0,y1),math.max(x0,x1),math.max(y0,y1)) == true and
    vertical_quadratic_test(x0,y0,x1,y1,x2,y2xmin,ymin,math.min(x0,x1),math.min(y0,y1),math.max(x0,x1),math.max(y0,y1)) == false then
    return true
  elseif horizontal_quadratic_test(x0,y0,x1,y1,x2,y2xmin,ymin,math.min(x0,x1),math.min(y0,y1),math.max(x0,x1),math.max(y0,y1)) == true and
    horizontal_quadratic_test(x0,y0,x1,y1,x2,y2xmax,ymin,math.min(x0,x1),math.min(y0,y1),math.max(x0,x1),math.max(y0,y1)) == false then
    return true
  elseif horizontal_quadratic_test(x0,y0,x1,y1,x2,y2xmax,ymin,math.min(x0,x1),math.min(y0,y1),math.max(x0,x1),math.max(y0,y1)) == true and
    horizontal_quadratic_test(x0,y0,x1,y1,x2,y2xmax,ymax,math.min(x0,x1),math.min(y0,y1),math.max(x0,x1),math.max(y0,y1)) == false then
    return true
  else
    return false
  end
end

----------------------------------------
--[[ RATIONAL QUADRATICS FUNCTIONS	]]--
----------------------------------------
function calculate_rational_quadratic_coefs(x0,y0,x1,y1,w1,x2,y2)
  local a = (4*x1^2 - 4*w1*x1*x2 + x2^2)
  local b = 4*x1*x2*y1 - 4*x1^2*y2
  local c = -4*x2*y1^2 + 4*x1*y1*y2
  local d = -8*x1*y1 + 4*w1*x2*y1 + 4*w1*x1*y2 - 2*x2*y2
  local e = (4*y1^2 - 4*w1*y1*y2 + y2^2)
  local sign = 2*y2*(-x2*y1 + x1*y2)

  return a,b,c,d,e,sign
end

function vertical_implicit_rational_quadratic_test(x0,y0,x1,y1,w1,x2,y2,x,y)
  -- Observe que o teste vertical das quadráticas racionais não pré-calcula os coeficientes.
  -- Fiz isso para simplificar, já que essa função deve ser chamada apenas em pré-processamento.
  local a = (4*y1^2 - 4*w1*y1*y2 + y2^2)
  local b = 4*y1*y2*x1 - 4*y1^2*x2
  local c = -4*y2*x1^2 + 4*y1*x1*x2
  local d = -8*y1*x1 + 4*w1*y2*x1 + 4*w1*y1*x2 - 2*y2*x2
  local e = (4*x1^2 - 4*w1*x1*x2 + x2^2)
  local sign = 2*x2*(-y2*x1 + y1*x2)
  local valor = x*(a*x + b) + y*(c + x*d + y*e)
  return valor*sign > 0
end

function vertical_rational_quadratic_test(x0,y0,x1,y1,w1,x2,y2,x,y,coefs,xmin,ymin,xmax,ymax,diagonal)
  local test = false
  local a,b,c,d,e,sign = unpack(coefs,1,6)

  if xmin <= x and x < xmax and y > ymin then
      if y >= ymax then test = true
    else
    --Inside the bounding box. Diagonal test
      if diagonal == true then
        if vertical_test_linear_segment(x0,y0,x2,y2,x,y) == true then
          test = vertical_implicit_rational_quadratic_test(0,0,x1/w1-x0,y1/w1-y0,1,x2-x0,y2-y0,x-x0,y-y0)
        else
          test = false
        end
      else
        if vertical_test_linear_segment(x0,y0,x2,y2,x,y) == true then
          test = true
        else
          test = vertical_implicit_rational_quadratic_test(0,0,x1/w1-x0,y1/w1-y0,1,x2-x0,y2-y0,x-x0,y-y0)
        end
      end
    end
  end

  return test
end

function horizontal_implicit_rational_quadratic_test(a,b,c,d,e,sign,x,y)
	local valor =y*(a*y + b) + x*(c + y*d + x*e)
	return valor*sign < 0
end

function horizontal_rational_quadratic_test(x0,y0,x1,y1,w1,x2,y2,x,y,coefs,xmin,ymin,xmax,ymax,diagonal)
	local test = false
	local rec = false
	--if x==45.5 and y==132.5 then rec = true end
	local a,b,c,d,e,sign = unpack(coefs,1,6)

	if ymin <= y and y < ymax and x <= xmax then
		if x <= xmin then test = true
		else
		--Inside the bounding box. Diagonal test
			if diagonal == true then
				if horizontal_test_linear_segment(x0,y0,x2,y2,x,y) == true then
					test = horizontal_implicit_rational_quadratic_test(a,b,c,d,e,sign,x-x0,y-y0)
				else
					test = false
				end
			else
				if horizontal_test_linear_segment(x0,y0,x2,y2,x,y) == true then
					test = true
				else
					test = horizontal_implicit_rational_quadratic_test(a,b,c,d,e,sign,x-x0,y-y0)
				end
			end
		end
	end

	return test
end

----------------------------------------
--[[	CUBIC FUNCTIONS				]]--
----------------------------------------
function calculate_cubic_coefs(x1,y1,x2,y2,x3,y3)
	local a = -27*x1*x3^2*y1^2 + 81*x1*x2*x3*y1*y2 - 81*x1^2*x3*y2^2 - 81*x1*x2^2*y1*y3 + 54*x1^2*x3*y1*y3 + 81*x1^2*x2*y2*y3 - 27*x1^3*y3^2
	local b = -27*x1^3 + 81*x1^2*x2 - 81*x1*x2^2 + 27*x2^3 - 27*x1^2*x3 + 54*x1*x2*x3 - 27*x2^2*x3 - 9*x1*x3^2 + 9*x2*x3^2 - x3^3
	local c = 81*x1*x2^2*y1 - 54*x1^2*x3*y1 - 81*x1*x2*x3*y1 + 54*x1*x3^2*y1 - 9*x2*x3^2*y1 - 81*x1^2*x2*y2 + 162*x1^2*x3*y2 - 81*x1*x2*x3*y2 + 27*x2^2*x3*y2 - 18*x1*x3^2*y2 + 54*x1^3*y3 - 81*x1^2*x2*y3 + 81*x1*x2^2*y3 - 27*x2^3*y3 - 54*x1^2*x3*y3 + 27*x1*x2*x3*y3
	local d = 27*x3^2*y1^3 - 81*x2*x3*y1^2*y2 + 81*x1*x3*y1*y2^2 + 81*x2^2*y1^2*y3 - 54*x1*x3*y1^2*y3 - 81*x1*x2*y1*y2*y3 + 27*x1^2*y1*y3^2
	local e = -81*x2^2*y1^2 + 108*x1*x3*y1^2 + 81*x2*x3*y1^2 - 54*x3^2*y1^2 - 243*x1*x3*y1*y2 + 81*x2*x3*y1*y2 + 27*x3^2*y1*y2 + 81*x1^2*y2^2 + 81*x1*x3*y2^2 - 54*x2*x3*y2^2 - 108*x1^2*y1*y3 + 243*x1*x2*y1*y3 - 81*x2^2*y1*y3 - 9*x2*x3*y1*y3 - 81*x1^2*y2*y3 - 81*x1*x2*y2*y3 + 54*x2^2*y2*y3 + 9*x1*x3*y2*y3 + 54*x1^2*y3^2 - 27*x1*x2*y3^2
	local f = 81*x1^2*y1 - 162*x1*x2*y1 + 81*x2^2*y1 + 54*x1*x3*y1 - 54*x2*x3*y1 + 9*x3^2*y1 - 81*x1^2*y2 + 162*x1*x2*y2 - 81*x2^2*y2 - 54*x1*x3*y2 + 54*x2*x3*y2 - 9*x3^2*y2 + 27*x1^2*y3 - 54*x1*x2*y3 + 27*x2^2*y3 + 18*x1*x3*y3 - 18*x2*x3*y3 + 3*x3^2*y3
	local g = -54*x3*y1^3 + 81*x2*y1^2*y2 + 81*x3*y1^2*y2 - 81*x1*y1*y2^2 - 81*x3*y1*y2^2 + 27*x3*y2^3 + 54*x1*y1^2*y3 - 162*x2*y1^2*y3 + 54*x3*y1^2*y3 + 81*x1*y1*y2*y3 + 81*x2*y1*y2*y3 - 27*x3*y1*y2*y3 - 27*x2*y2^2*y3 - 54*x1*y1*y3^2 + 18*x2*y1*y3^2 + 9*x1*y2*y3^2
	local h = -81*x1*y1^2 + 81*x2*y1^2 - 27*x3*y1^2 + 162*x1*y1*y2 - 162*x2*y1*y2 + 54*x3*y1*y2 - 81*x1*y2^2 + 81*x2*y2^2 - 27*x3*y2^2 - 54*x1*y1*y3 + 54*x2*y1*y3 - 18*x3*y1*y3 + 54*x1*y2*y3 - 54*x2*y2*y3 + 18*x3*y2*y3 - 9*x1*y3^2 + 9*x2*y3^2 - 3*x3*y3^2
	local i = 27*y1^3 - 81*y1^2*y2 + 81*y1*y2^2 - 27*y2^3 + 27*y1^2*y3 - 54*y1*y2*y3 + 27*y2^2*y3 + 9*y1*y3^2 - 9*y2*y3^2 + y3^3
	local sign = (y1-y2-y3)*(-x3^2*(4*y1^2 - 2*y1*y2 + y2^2) + x1^2*(9*y2^2 - 6*y2*y3 - 4*y3^2) + x2^2*(9*y1^2 - 12*y1*y3 - y3^2) + 2*x1*x3*(-y2*(6*y2 + y3) + y1*(3*y2 + 4*y3)) - 2*x2*(x3*(3*y1^2 - y2*y3 + y1*(-6*y2 + y3)) + x1*(y1*(9*y2 - 3*y3) - y3*(6*y2 + y3))))
	return a,b,c,d,e,f,g,h,i,sign
end

function horizontal_implicit_cubic_test(a,b,c,d,e,f,g,h,i,sign,x,y)
	local valor = y*(a + y*(b*y + c)) + x*(d + y*(e + y*f) + x*(g + y*h + x*i))
	return sign*valor <	 0
end

function horizontalInsideTriangle(x0,y0,x1,y1,x2,y2,x3,y3,x,y)
	local den = (x3-x2)*y1-x1*(y3-y2)
	local xi, yi = x1*(x3*y2-x2*y3)/den, y1*(x3*y2-x2*y3)/den

	local wind_num = 0
	wind_num = wind_num + applyWindingNumber(horizontal_linear_test(x0,y0,x3,y3,x,y,math.min(x0,x3),math.min(y0,y3),math.max(x0,x3),math.max(y0,y3)),"odd",y0,y3)
	wind_num = wind_num + applyWindingNumber(horizontal_linear_test(x3,y3,xi,yi,x,y,math.min(x3,xi),math.min(y3,yi),math.max(x3,xi),math.max(y3,yi)),"odd",y3,yi)
	wind_num = wind_num + applyWindingNumber(horizontal_linear_test(x0,y0,xi,yi,x,y,math.min(x0,xi),math.min(y0,yi),math.max(x0,xi),math.max(y0,yi)),"odd",y0,yi)

	return wind_num == 1
end

function horizontal_cubic_test(x0,y0,x1,y1,x2,y2,x3,y3,x,y,coefs,xmin,ymin,xmax,ymax,diagonal)
	local test = false
	local a,b,c,d,e,f,g,h,i,sign = unpack(coefs,1,10)

	if x0 == x1 and y0 == y1 then
		return horizontal_quadratic_test(x0,y0,x2,y2,x3,y3,x,y,xmin,ymin,xmax,ymax,diagonal)
	elseif x2 == x3 and y2 == y3 then
		return horizontal_quadratic_test(x0,y0,x1,y1,x2,y2,x,y,xmin,ymin,xmax,ymax,diagonal)
	end

	if bezier.classify3(x0,y0,x1,y1,x2,y2,x3,y3) == "line or point" then
		return horizontal_linear_test(x0,y0,x3,y3,x,y,xmin,ymin,xmax,ymax)
	elseif bezier.classify3(x0,y0,x1,y1,x2,y2,x3,y3) == "quadratic" then
		return horizontal_quadratic_test(x0,y0,x1,y1,x3,y3,x,y,xmin,ymin,xmax,ymax,diagonal)

	end

	if ymin <= y and y < ymax and x <= xmax then
		if x < xmin then test = true
		else
		--Inside the bounding box. Diagonal test
			if diagonal == true then
				if horizontal_test_linear_segment(x0,y0,x3,y3,x,y) == true then
					if horizontalInsideTriangle(0,0,x1-x0,y1-y0,x2-x0,y2-y0,x3-x0,y3-y0,x-x0,y-y0) == true then
						test = horizontal_implicit_cubic_test(a,b,c,d,e,f,g,h,i,sign,x-x0,y-y0)
					else
						test = true
					end
				else
					test = false
				end
			else
				if horizontal_test_linear_segment(x0,y0,x3,y3,x,y) == true then
					test = true
				else
					if horizontalInsideTriangle(0,0,x1-x0,y1-y0,x2-x0,y2-y0,x3-x0,y3-y0,x-x0,y-y0) == true then
						test = horizontal_implicit_cubic_test(a,b,c,d,e,f,g,h,i,sign,x-x0,y-y0)
					else
						test = false
					end
				end
			end
		end
	end

	return test
end

function vertical_implicit_cubic_test(a,b,c,d,e,f,g,h,i,sign,x,y)
  local valor = x*(a + x*(b*x + c)) + y*(d + x*(e + x*f) + y*(g + x*h + y*i))
  return sign*valor > 0
end

function verticalInsideTriangle(x0,y0,x1,y1,x2,y2,x3,y3,x,y)
  local den = (x3-x2)*y1-x1*(y3-y2)
  local xi, yi = x1*(x3*y2-x2*y3)/den, y1*(x3*y2-x2*y3)/den

  local wind_num = 0
  wind_num = wind_num + applyWindingNumber(vertical_linear_test(x0,y0,x3,y3,x,y,math.min(x0,x3),math.min(y0,y3),math.max(x0,x3),math.max(y0,y3)),"odd",x0,x3)
  wind_num = wind_num + applyWindingNumber(vertical_linear_test(x3,y3,xi,yi,x,y,math.min(x3,xi),math.min(y3,yi),math.max(x3,xi),math.max(y3,yi)),"odd",x3,xi)
  wind_num = wind_num + applyWindingNumber(vertical_linear_test(x0,y0,xi,yi,x,y,math.min(x0,xi),math.min(y0,yi),math.max(x0,xi),math.max(y0,yi)),"odd",x0,xi)
  return wind_num == 1
end

function vertical_cubic_test(x0,y0,x1,y1,x2,y2,x3,y3,x,y,coefs,xmin,ymin,xmax,ymax,diagonal)
  local test = false
  local a,b,c,d,e,f,g,h,i,sign = calculate_cubic_coefs(y1-y0,x1-x0,y2-y0,x2-x0,y3-y0,x3-x0)
  if x0 == x1 and y0 == y1 then
    return vertical_quadratic_test(x0,y0,x2,y2,x3,y3,x,y,xmin,ymin,xmax,ymax,diagonal)
  elseif x2 == x3 and y2 == y3 then
    return vertical_quadratic_test(x0,y0,x1,y1,x2,y2,x,y,xmin,ymin,xmax,ymax,diagonal)
  end

  if bezier.classify3(x0,y0,x1,y1,x2,y2,x3,y3) == "line or point" then
    return vertical_linear_test(x0,y0,x3,y3,x,y,xmin,ymin,xmax,ymax)
  elseif bezier.classify3(x0,y0,x1,y1,x2,y2,x3,y3) == "quadratic" then
    return vertical_quadratic_test(x0,y0,x1,y1,x3,y3,x,y,xmin,ymin,xmax,ymax,diagonal)

  end

  if xmin <= x and x < xmax and y >= ymin then
    if y >= ymax then test = true
    else
    --Inside the bounding box. Diagonal test
      if diagonal == true then
        if vertical_test_linear_segment(x0,y0,x3,y3,x,y) == true then
          if verticalInsideTriangle(0,0,x1-x0,y1-y0,x2-x0,y2-y0,x3-x0,y3-y0,x-x0,y-y0) == true then
            test = vertical_implicit_cubic_test(a,b,c,d,e,f,g,h,i,sign,x-x0,y-y0)
          else
            test = true
          end
        else
          test = false
        end
      else
        if vertical_test_linear_segment(x0,y0,x3,y3,x,y) == true then
          test = true
        else
          if verticalInsideTriangle(0,0,x1-x0,y1-y0,x2-x0,y2-y0,x3-x0,y3-y0,x-x0,y-y0) == true then
            test = vertical_implicit_cubic_test(a,b,c,d,e,f,g,h,i,sign,x-x0,y-y0)
          else
            test = false
          end
        end
      end
    end
  end

  return test
end

-----------------------------------------
--[[      SHORTCUTS          ]] --
----------------------------------------
local function LinearIntersection(x0,y0,x1,y1, xmin, ymin, xmax, ymax)
  if vertical_test_linear_segment(x0,y0,x1,y1,xmax, ymax-0.05) == true and vertical_test_linear_segment(x0,y0,x1,y1,xmax, ymin+0.05) == false then
    return true
  else
    return false
  end
  return false
end

local function QuadraticIntersection(x0,y0,x1,y1,x2,y2,xmin,ymin,xmax,ymax)
  if implicit_vertical_quadratic_test(x0,y0,x1,y1,x2,y2,xmax,ymax-0.05) == true and implicit_vertical_quadratic_test(x0,y0,x1,y1,x2,y2,xmax,ymin+0.05) == false then
    return true
  else
    return false
  end
  return false
end

local function CubicIntersection(x0,y0,x1,y1,x2,y2,x3,y3,xmin,ymin,xmax,ymax)
  local a,b,c,d,e,f,g,h,i,sign = calculate_cubic_coefs(y1-y0,x1-x0,y2-y0,x2-x0,y3-y0,x3-x0)
  if vertical_implicit_cubic_test(a,b,c,d,e,f,g,h,i,sign,xmax-x0,ymax-y0-0.05) == true and vertical_implicit_cubic_test(a,b,c,d,e,f,g,h,i,sign,xmax-x0,ymin-y0+0.05) == false then
    return true
  else
    return false
  end
end

local function RationalQuadraticIntersection(x0,y0,x1,y1,w1,x2,y2,xmin,ymin,xmax,ymax)
  local a,b,c,d,e,f,g,h,i,sign = calculate_rational_quadratic_coefs(0,0,y1/w1-y0,x1/w1-x0,1,y2-y0,x2-x0)
  if vertical_implicit_rational_quadratic_test(0,0,x1/w1-x0,y1/w1-y0,1,x2-x0,y2-y0,xmax-x0,ymax-y0-0.05) == true and
    vertical_implicit_rational_quadratic_test(0,0,x1/w1-x0,y1/w1-y0,1,x2-x0,y2-y0,xmax-x0,ymin-y0+0.05) == false then
    return true
  else
    return false
  end
end

local function CreateShortcuts(scene,data, bb, ind)
  local xmin, ymin, xmax, ymax = unpack(bb)
  local shortcuts = {}
  local rec = false
  local number = 0
  for i in pairs(data) do -- For each path
    shortcuts[i] = {}
    if data[i] ~= nil then
      for j = 1, #data[i] do
        local instruction = data[i][j]
        local offset = scene.shapes[i].offsets[instruction]
        if scene.shapes[i].instructions[instruction] == 'linear_segment' then
          local x0, y0, x1, y1 = unpack(scene.shapes[i].data, offset, offset+3)
          if x1 ~= x0 then
            if (xmax-x0)*(xmax-x1) <= 0 then
              if LinearIntersection(x0,y0,x1,y1,xmin,ymin,xmax,ymax) then
                if util.sign(x1-x0) > 0 then
                  x0s, y0s =  x1, y1
                  sobe = true
                else
                  x0s, y0s = x0, y0
                  sobe = false
                end
                table.insert(shortcuts[i],x0s)
                table.insert(shortcuts[i],y0s)
                table.insert(shortcuts[i],sobe)
                table.insert(shortcuts[i],instruction)
                number = number + 1
              end
            end
          end
        elseif scene.shapes[i].instructions[instruction] == 'end_open_contour' or scene.shapes[i].instructions[instruction] == 'end_closed_contour' then
          local x0, y0, len = unpack(scene.shapes[i].data, offset, offset+2)
          local begin_off = scene.shapes[i].offsets[instruction-len]
          local x1, y1 = unpack(scene.shapes[i].data, begin_off+1, begin_off+2)
          if x1 ~= x0 then
            if (xmax-x0)*(xmax-x1) <= 0  then
              if LinearIntersection (x0,y0,x1,y1,xmin,ymin,xmax,ymax) then
                local x0s,y0s
                if (x1-x0) > 0 then
                  x0s, y0s =  x1, y1
                  sobe = true
                else
                  x0s, y0s = x0, y0
                  sobe = false
                end
                table.insert(shortcuts[i],x0s)
                table.insert(shortcuts[i],y0s)
                table.insert(shortcuts[i],sobe)
                table.insert(shortcuts[i],instruction)
              end
            end
          end
        elseif scene.shapes[i].instructions[instruction] == "quadratic_segment" then
          local x0,y0,x1,y1,x2,y2 = unpack(scene.shapes[i].data,offset,offset+5)
          if x2 ~= x0 then
            if (xmax-x0)*(xmax-x2) <= 0 then
              if QuadraticIntersection(x0,y0,x1,y1,x2,y2,xmin,ymin,xmax,ymax) then
                if x2-x0 > 0 then
                  x0s, y0s =  x2, y2
                  sobe = true
                else
                  x0s, y0s = x0, y0
                  sobe = false
                end
                table.insert(shortcuts[i],x0s)
                table.insert(shortcuts[i],y0s)
                table.insert(shortcuts[i],sobe)
                table.insert(shortcuts[i],instruction)
                number = number + 1
              end
            end
          end
        elseif scene.shapes[i].instructions[instruction] == "cubic_segment" then
          local x0,y0,x1,y1,x2,y2,x3,y3 = unpack(scene.shapes[i].data,offset,offset+7)
          if x3 ~= x0 then
            if x0 > xmax or x3 > xmax then
              if CubicIntersection(x0,y0,x1,y1,x2,y2,x3,y3,xmin,ymin,xmax,ymax) then
                if x3-x0 > 0 then
                  x0s, y0s =  x3, y3
                  sobe = true
                else
                  x0s, y0s = x0, y0
                  sobe = false
                end
                table.insert(shortcuts[i],x0s)
                table.insert(shortcuts[i],y0s)
                table.insert(shortcuts[i],sobe)
                table.insert(shortcuts[i],instruction)
                number = number + 1
              end
            end
          end
        elseif scene.shapes[i].instructions[instruction] == "rational_quadratic_segment" then
          local x0,y0,x1,y1,w1,x2,y2 = unpack(scene.shapes[i].data,offset,offset+6)
          if x2 ~= x0 then
            if x0 > xmax or x2 > xmax then
              if RationalQuadraticIntersection(x0,y0,x1,y1,w1,x2,y2,xmin,ymin,xmax,ymax) then
                if x2-x0 > 0 then
                  x0s, y0s =  x2, y2
                  sobe = true
                else
                  x0s, y0s = x0, y0
                  sobe = false
                end
                table.insert(shortcuts[i],x0s)
                table.insert(shortcuts[i], y0s)
                table.insert(shortcuts[i], sobe)
                table.insert(shortcuts[i], instruction)
                number = number + 1
              end
            end
          end
        end
      end
    end
  end

  return shortcuts, number --- tabela formato {shape1{shortcuts},shape2{}}
end
-----------------------------------------
--[[   WINDING INCREMENTS    ]]--
-----------------------------------------
local function WindingIncrement(tree, ind, shape, segment_num, k)
  local bb = tree[ind].boundingBox
  local offset = shape.offsets[segment_num]
  local instruction = shape.instructions[segment_num]
  local bxmin, bymin, bxmax, bymax = unpack(bb,1,4)
  if instruction == 'linear_segment' then
    local x0,y0,x1,y1 = unpack(shape.data, offset, offset + 3)
    local ymax = math.max(y0,y1)
    local ymin = math.min(y0,y1)
    local xmin = math.min(x0,x1)
    local xmax = math.max(x0,x1)
    if y1 == y0 then return 0 end
    if bymin < ymax and bymin >= ymin then
      if horizontal_test_linear_segment(x0,y0, x1, y1, bxmax, bymin) then return util.sign(y1-y0) end
    end
  elseif instruction == 'end_open_contour' or instruction == 'end_closed_contour' then
    local x0,y0,len = unpack(shape.data, offset, offset+2)
    local offset_begin = shape.offsets[segment_num - len]
    local x1,y1 = unpack(shape.data, offset_begin+1, offset_begin+2)
    local ymax = math.max(y0,y1)
    local ymin = math.min(y0,y1)
    if y1 == y0 then return 0 end
    if bymin < ymax and bymin >= ymin then
      if horizontal_test_linear_segment(x0,y0, x1, y1, bxmax, bymin) then return util.sign(y1-y0) end
    end
  elseif instruction == 'quadratic_segment' then
    local x0,y0,x1,y1,x2,y2 = unpack(shape.data,offset,offset+5)
    local ymax = math.max(y0,y2)
    local ymin = math.min(y0,y2)
    if y2 == y0 then return 0 end
    if bymin < ymax and bymin >= ymin then
      if implicit_horizontal_quadratic_test(x0,y0,x1,y1,x2,y2,bxmax,bymin+0.15) then return util.sign(y2-y0) end
    end
  elseif instruction == 'cubic_segment' then
    local x0,y0,x1,y1,x2,y2,x3,y3 = unpack(shape.data,offset,offset+7)
    local ymax = math.max(y0,y3)
    local ymin = math.min(y0,y3)
    if y3 == y0 then return 0 end
    if bymin < ymax and bymin >= ymin then
      local a,b,c,d,e,f,g,h,i,sign = calculate_cubic_coefs(x1-x0,y1-y0,x2-x0,y2-y0,x3-x0,y3-y0)
      if horizontal_implicit_cubic_test(a,b,c,d,e,f,g,h,i,sign,bxmax-x0,bymin-y0) then return util.sign(y3-y0) end
    end
  elseif instruction == 'rational_quadratic_segment' then
    local x0,y0,x1,y1,w1,x2,y2 = unpack(shape.data,offset,offset+6)
    local ymax = math.max(y0,y2)
    local ymin = math.min(y0,y2)
    if y2 == y0 then return 0 end
    if bymin < ymax and bymin >= ymin then
      local a,b,c,d,e,sign = calculate_rational_quadratic_coefs(0,0,x1/w1-x0,y1/w1-y0,1,x2-x0,y2-y0)
      if horizontal_implicit_rational_quadratic_test(a,b,c,d,e,sign,bxmax-x0,bymin-y0) then return util.sign(y2-y0) end
    end
  end
  return 0
end

local function WindingShortcuts(tree,ind,fatherind,segment_num,shape)
  local winding = 0
  local shortcuts = tree[fatherind].shortcuts[shape]
  local data = tree[ind].data[shape]
  local xmin, ymin, xmax, ymax = unpack(tree[ind].boundingBox)
  local ymax_shortcuts = tree[fatherind].boundingBox[4]
  if shortcuts ~= nil then
    local n = #shortcuts
    for k =1, n, 4 do
      local x0,y0,sobe,father_segment = unpack(shortcuts, k, k+3)
      if father_segment == segment_num then
        if sobe == true then
          if (ymin - y0)*(ymin - ymax_shortcuts)<0 and horizontal_test_linear_segment(x0,y0, x0, ymax_shortcuts, xmax, ymin + 0.05) then
            winding = winding + 1
          end
        elseif sobe == false then
          if (ymin - y0)*(ymin - ymax_shortcuts)<0 and horizontal_test_linear_segment(x0,ymax_shortcuts, x0, y0, xmax, ymin + 0.05) then
            winding = winding - 1
          end
        end
      end
    end
  end
  return winding
end
-----------------------------------------
--[[    DEBUGGING GRID       ]]--
-----------------------------------------
local function InsideGrid(tree, ind, x, y)
  local xmin, ymin, xmax, ymax = unpack(tree[ind].boundingBox)
  if insideBoundingBox((xmax+xmin)/2 - 0.5, ymin, (xmax+xmin)/2 + 0.5, ymax, x, y) or insideBoundingBox(xmin, (ymax+ymin)/2 - 0.5, xmax, (ymax+ymin)/2 +0.5 , x, y) then
    return true
  else
    return false
  end
  return false
end
-----------------------------------------
--[[		SHORTCUT TREE 			 ]]--
-----------------------------------------
local function createBoundingBox(bb, read)
  local xmin, xmax, ymin, ymax
  local vxmin, vymin, vxmax, vymax = unpack(bb,1,4)
  local Dx = (vxmax - vxmin)/2
  local Dy = (vymax - vymin)/2

  if read == 1 then
    xmax = vxmin + Dx
    xmin = vxmin
    ymax = vymin + 2*Dy
    ymin = vymin + Dy
  elseif read == 2 then
    xmax = vxmin + 2*Dx
    xmin = vxmin + Dx
    ymax = vymin + 2*Dy
    ymin = vymin + Dy
  elseif read == 3 then
    xmax = vxmin + Dx
    xmin = vxmin
    ymax = vymin + Dy
    ymin = vymin
  elseif read == 4 then
    xmax = vxmin + 2*Dx
    xmin = vxmin + Dx
    ymax = vymin + Dy
    ymin = vymin
  end

  return xmin,ymin,xmax,ymax
end

-- Função que diferencia scenes como branches ou leafs
-- LEMBRAR DE USÁ-LA NO SAMPLE
function isLeaf(tree, ind, maxdepth, maxseg)
  if maxseg <= tree[ind].segments and tree[ind].depth <= maxdepth then return false
  else return true end
end

function insidetest_linear(x0,y0,x1,y1,xmin,ymin,xmax,ymax)
  local xm,ym = (x0+x1)/2,(y0+y1)/2
  -- if y0 == y1 and (x1 == xmax or x1 == xmin) then return false end
  -- if x0 == x1 and (x1 <= xmax) and y0 ~= y1 then return true end
  if (xmin<x0 and x0<xmax and ymin<y0 and y0<ymax) or (xmin<x1 and x1<xmax and ymin<y1 and y1<ymax) or
    (xmin<xm and xm<xmax and ymin<ym and ym<ymax) then
    return true
  else
   local sxmin, symin, sxmax, symax = math.min(x0,x1),math.min(y0,y1),math.max(x0,x1),math.max(y0,y1)
    if vertical_test_linear_segment(x0,y0,x1,y1,xmax-0.05,ymax-0.05) == true and
       vertical_test_linear_segment(x0,y0,x1,y1,xmax-0.05,ymin+0.05) == false and
       sxmin <= xmax and xmax <= symax then
          return true
    end
    if vertical_test_linear_segment(x0,y0,x1,y1,xmin-0.05,ymax-0.05) == true and
      vertical_test_linear_segment(x0,y0,x1,y1,xmin-0.05,ymin+0.05) == false and
      sxmin <= xmin and xmin <= sxmax then
          return true
    end
    if horizontal_test_linear_segment(x0,y0,x1,y1,xmin-0.05,ymin) == true and
      horizontal_test_linear_segment(x0,y0,x1,y1,xmax-0.05,ymin) == false and
      symin <= ymin and ymin <= symax then
          return true
    end
    if horizontal_test_linear_segment(x0,y0,x1,y1,xmin+0.05,ymax) == true and
      horizontal_test_linear_segment(x0,y0,x1,y1,xmax-0.05,ymax) == false and
      symin <= ymax and ymax <= symax then
          return true
    end
  end
end

function insidetest_quadratic(x0,y0,x1,y1,x2,y2,xmin,ymin,xmax,ymax,ind)
  local xm,ym = bezier.at2(1/2,x0,y0,x1,y1,x2,y2)
  if x0 == x2 and (x2 == xmax or x2 == xmin) then return false end
  if (xmin<x0 and x0<xmax and ymin<y0 and y0<ymax) or (xmin<x2 and x2<xmax and ymin<y2 and y2<ymax) or
    (xmin<xm and xm<xmax and ymin<ym and ym<ymax) then
    return true
  else
   local sxmin, symin, sxmax, symax = math.min(x0,x2),math.min(y0,y2),math.max(x0,x2),math.max(y0,y2)
    if implicit_vertical_quadratic_test(x0,y0,x1,y1,x2,y2,xmax,ymax-0.05) == true and
       implicit_vertical_quadratic_test(x0,y0,x1,y1,x2,y2,xmax,ymin+0.05) == false and
       sxmin <= xmax and xmax <= symax then
          return true
    end
    if implicit_vertical_quadratic_test(x0,y0,x1,y1,x2,y2,xmin,ymax-0.05) == true and
      implicit_vertical_quadratic_test(x0,y0,x1,y1,x2,y2,xmin,ymin+0.05) == false and
      sxmin <= xmin and xmin <= sxmax then
          return true
    end
    if implicit_horizontal_quadratic_test(x0,y0,x1,y1,x2,y2,xmin-0.05,ymin) == true and
      implicit_horizontal_quadratic_test(x0,y0,x1,y1,x2,y2,xmax-0.05,ymin) == false and
      symin <= ymin and ymin <= symax then
          return true
    end
    if implicit_horizontal_quadratic_test(x0,y0,x1,y1,x2,y2,xmin+0.05,ymax) == true and
      implicit_horizontal_quadratic_test(x0,y0,x1,y1,x2,y2,xmax-0.05,ymax) == false and
      symin <= ymax and ymax <= symax then
          return true
    end
  end
end

function insidetest_cubic(x0,y0,x1,y1,x2,y2,x3,y3,xmin,ymin,xmax,ymax)
  local xm,ym = bezier.at3(1/2,x0,y0,x1,y1,x2,y2,x3,y3)
  local rec = false
  if xmin == 0 and xmax == 100 and ymin == 100 and ymax == 200 then rec = true end
  if x0 == x3 and (x3 == xmax or x3 == xmin) then return false end
  if (xmin<x0 and x0<xmax and ymin<y0 and y0<ymax) or (xmin<x3 and x3<xmax and ymin<y3 and y3<ymax) or
    (xmin<xm and xm<xmax and ymin<ym and ym<ymax) then
    return true
  else
   local sxmin, symin, sxmax, symax = math.min(x0,x3),math.min(y0,y3),math.max(x0,x3),math.max(y0,y3)
   local ah,bh,ch,dh,eh,fh,gh,hh,ih,signh = calculate_cubic_coefs(x1-x0,y1-y0,x2-x0,y2-y0,x3-x0,y3-y0)
   local av,bv,cv,dv,ev,fv,gv,hv,iv,signv = calculate_cubic_coefs(y1-y0,x1-x0,y2-y0,x2-x0,y3-y0,x3-x0)
    if vertical_implicit_cubic_test(av,bv,cv,dv,ev,fv,gv,hv,iv,signv,xmax-x0,ymax-y0-0.05) == true and
       vertical_implicit_cubic_test(av,bv,cv,dv,ev,fv,gv,hv,iv,signv,xmax-x0,ymin-y0+0.05) == false and
       sxmin <= xmax and xmax <= symax then
          return true
    end
    if vertical_implicit_cubic_test(av,bv,cv,dv,ev,fv,gv,hv,iv,signv,xmin-x0,ymax-y0-0.05) == true and
      vertical_implicit_cubic_test(av,bv,cv,dv,ev,fv,gv,hv,iv,signv,xmin-x0,ymin-y0+0.05) == false and
      sxmin <= xmin and xmin <= sxmax then
          return true
    end
    if horizontal_implicit_cubic_test(ah,bh,ch,dh,eh,fh,gh,hh,ih,signh,xmin-x0-0.05,ymin-y0) == true and
      horizontal_implicit_cubic_test(ah,bh,ch,dh,eh,fh,gh,hh,ih,signh,xmax-x0-0.05,ymin-y0) == false and
      symin <= ymin and ymin <= symax then
          return true
    end
    if horizontal_implicit_cubic_test(ah,bh,ch,dh,eh,fh,gh,hh,ih,signh,xmin-x0+0.05,ymax-y0) == true and
      horizontal_implicit_cubic_test(ah,bh,ch,dh,eh,fh,gh,hh,ih,signh,xmax-x0-0.05,ymax-y0) == false and
      symin <= ymax and ymax <= symax then
          return true
    end
  end
end

function insidetest_rationalquadratic(x0,y0,x1,y1,w1,x2,y2,xmin,ymin,xmax,ymax)
  local xm,ym = bezier.at2r(1/2,x0,y0,1,x1,y1,w1,x2,y2,1)
  local rec = false
  if xmin == 0 and xmax == 100 and ymin == 100 and ymax == 200 then rec = true end
  if x0 == x2 and (x2 == xmax or x2 == xmin) then return false end
  if (xmin<x0 and x0<xmax and ymin<y0 and y0<ymax) or (xmin<x2 and x2<xmax and ymin<y2 and y2<ymax) or
    (xmin<xm and xm<xmax and ymin<ym and ym<ymax) then
    return true
  else
   local sxmin, symin, sxmax, symax = math.min(x0,x2),math.min(y0,y2),math.max(x0,x2),math.max(y0,y2)
   local ah,bh,ch,dh,eh,signh = calculate_rational_quadratic_coefs(0,0,x1/w1-x0,y1/w1-y0,1,x2-x0,y2-y0)
   -- local av,bv,cv,dv,ev,signv = calculate_rational_quadratic_coefs(0,0,y1/w1-y0,x1/w1-x0,1,y2-y0,x2-x0)
    if vertical_implicit_rational_quadratic_test(0,0,x1/w1-x0,y1/w1-y0,1,x2-x0,y2-y0,xmax-x0,ymax-y0-0.05) == true and
       vertical_implicit_rational_quadratic_test(0,0,x1/w1-x0,y1/w1-y0,1,x2-x0,y2-y0,xmax-x0,ymin-y0+0.05) == false and
       sxmin <= xmax and xmax <= symax then
          return true
    end
    if vertical_implicit_rational_quadratic_test(0,0,x1/w1-x0,y1/w1-y0,1,x2-x0,y2-y0,xmin-x0,ymax-y0-0.05) == true and
      vertical_implicit_rational_quadratic_test(0,0,x1/w1-x0,y1/w1-y0,1,x2-x0,y2-y0,xmin-x0,ymin-y0+0.05) == false and
      sxmin <= xmin and xmin <= sxmax then
          return true
    end
    if horizontal_implicit_rational_quadratic_test(ah,bh,ch,dh,eh,signh,xmin-x0-0.05,ymin-y0) == true and
      horizontal_implicit_rational_quadratic_test(ah,bh,ch,dh,eh,signh,xmax-x0-0.05,ymin-y0) == false and
      symin <= ymin and ymin <= symax then
          return true
    end
    if horizontal_implicit_rational_quadratic_test(ah,bh,ch,dh,eh,signh,xmin-x0+0.05,ymax-y0) == true and
      horizontal_implicit_rational_quadratic_test(ah,bh,ch,dh,eh,signh,xmax-x0-0.05,ymax-y0) == false and
      symin <= ymax and ymax <= symax then
          return true
    end
  end
end

function LineIntersect(k, segment_num, shape, xmin, ymin, xmax, ymax, ind)
    local instruction = shape.instructions[segment_num]
    local offset = shape.offsets[segment_num]

    if instruction == "begin_closed_contour" or instruction == "begin_open_contour" then
      return 0
    elseif instruction == "end_open_contour" or instruction == "end_closed_contour" then
      local x0,y0,len = unpack(shape.data, offset, offset+2)
      local offset_begin = shape.offsets[segment_num - len]
      local x1, y1 = unpack(shape.data, offset_begin+1, offset_begin+2)
      if horizontal_test_linear_segment(x0,y0,x1,y1,xmin,ymin) == true and (ymin - y0)*(ymin - y1) < 0 and horizontal_test_linear_segment(x0,y0,x1,y1,xmax,ymin) == false then
        return util.sign(y1-y0)
      else
        return 0
      end
    elseif instruction == "linear_segment" then
      local x0,y0,x1,y1 = unpack(shape.data,offset,offset+3)
      if horizontal_test_linear_segment(x0,y0,x1,y1,xmin,ymin) == true and (ymin - y0)*(ymin - y1) < 0 and horizontal_test_linear_segment(x0,y0,x1,y1,xmax,ymin) == false then
        return util.sign(y1-y0)
      else
        return 0
      end
    elseif instruction == "cubic_segment" then
      local offset = shape.offsets[segment_num]
      local x0,y0,x1,y1,x2,y2,x3,y3 = unpack(shape.data,offset,offset+7)
      return insidetest_cubic(x0,y0,x1,y1,x2,y2,x3,y3,xmin,ymin,xmax,ymax)
    elseif instruction == "quadratic_segment" then
      local offset = shape.offsets[segment_num]
      local x0,y0,x1,y1,x2,y2 = unpack(shape.data,offset,offset+5)
      if insidetest_quadratic(x0,y0,x1,y1,x2,y2,xmin,ymin,xmax,ymax,ind) then return util.sign(y2-y0) end
    elseif instruction == "rational_quadratic_segment" then
      local offset = shape.offsets[segment_num]
      local x0,y0,x1,y1,w1,x2,y2 = unpack(shape.data,offset,offset+6)
      return insidetest_rationalquadratic(x0,y0,x1,y1,w1,x2,y2,xmin,ymin,xmax,ymax)
    end
    return 0
end

function fillData(scene, tree, fatherInd, ind)
  local number = 0

  for k in pairs(tree[fatherInd].data) do
    local shape = scene.shapes[k]
    tree[ind].data[k] = {}
    if ind == fatherInd .. "1" then
      tree[ind].winding[k] = tree[fatherInd .. "2"].winding[k]
      for index,segment_num in pairs(tree[fatherInd].data[k]) do
        if testSegment(tree, ind, shape, segment_num,k) == true then
          tree[ind].data[k][#tree[ind].data[k] + 1] = segment_num
          number = number + 1
        end
        local xmin,ymin,xmax,ymax = unpack(tree[fatherInd.."2"].boundingBox)
        tree[ind].winding[k] = tree[ind].winding[k] +  LineIntersect(k, segment_num, shape, xmin, ymin, xmax, ymax, ind)
      end
    elseif ind == fatherInd .. "2" then
      tree[ind].winding[k] = tree[fatherInd .. "4"].winding[k]
      for index,segment_num in pairs(tree[fatherInd].data[k]) do
        if testSegment(tree, ind, shape, segment_num,k) == true then
          tree[ind].data[k][#tree[ind].data[k] + 1] = segment_num
          number = number + 1
        end
        tree[ind].winding[k] = tree[ind].winding[k] + WindingIncrement(tree, ind, shape, segment_num, k)
        tree[ind].winding[k] = tree[ind].winding[k] + WindingShortcuts(tree, ind, fatherInd, segment_num, k)
      end

    elseif ind == fatherInd .. "3" then
      tree[ind].winding[k] = tree[fatherInd .. "4"].winding[k]
      for index,segment_num in pairs(tree[fatherInd].data[k]) do
        if testSegment(tree, ind, shape, segment_num,k) == true then
          tree[ind].data[k][#tree[ind].data[k] + 1] = segment_num
          number = number + 1
        end
        local xmin,ymin,xmax,ymax = unpack(tree[fatherInd.."4"].boundingBox)
        tree[ind].winding[k] = tree[ind].winding[k] +  LineIntersect(k, segment_num, shape, xmin, ymin, xmax, ymax, ind)
      end
    elseif ind == fatherInd .. "4" then
      tree[ind].winding[k] = tree[fatherInd].winding[k]
      for index,segment_num in pairs(tree[fatherInd].data[k]) do
        if testSegment(tree, ind, shape, segment_num,k) == true then
          tree[ind].data[k][#tree[ind].data[k] + 1] = segment_num
          number = number + 1
        end
      end
    end
  end
  tree[ind].segments = number
end

function testSegment(tree, ind, shape, segment_num, k)

  local xmin,ymin,xmax,ymax = unpack(tree[ind].boundingBox,1,4)

    --Olha a scene original
    local instruction = shape.instructions[segment_num]
    local offset = shape.offsets[segment_num]

    if instruction == "begin_closed_contour" or instruction == "begin_open_contour" then
      return false
    elseif instruction == "end_open_contour" or instruction == "end_closed_contour" then
      local x0,y0,len = unpack(shape.data, offset, offset+2)
      local offset_begin = shape.offsets[segment_num - len ]
      local xclose, yclose = unpack(shape.data, offset_begin+1, offset_begin+2)
      return insidetest_linear(x0,y0,xclose,yclose,xmin,ymin,xmax,ymax)
    elseif instruction == "linear_segment" then
      local x0,y0,x1,y1 = unpack(shape.data,offset,offset+3)
      return insidetest_linear(x0,y0,x1,y1,xmin,ymin,xmax,ymax)
    elseif instruction == "cubic_segment" then
      local offset = shape.offsets[segment_num]
      local x0,y0,x1,y1,x2,y2,x3,y3 = unpack(shape.data,offset,offset+7)
      return insidetest_cubic(x0,y0,x1,y1,x2,y2,x3,y3,xmin,ymin,xmax,ymax)
    elseif instruction == "quadratic_segment" then
      local offset = shape.offsets[segment_num]
      local x0,y0,x1,y1,x2,y2 = unpack(shape.data,offset,offset+5)
      return insidetest_quadratic(x0,y0,x1,y1,x2,y2,xmin,ymin,xmax,ymax,ind)
    elseif instruction == "rational_quadratic_segment" then
      local offset = shape.offsets[segment_num]
      local x0,y0,x1,y1,w1,x2,y2 = unpack(shape.data,offset,offset+6)
      return insidetest_rationalquadratic(x0,y0,x1,y1,w1,x2,y2,xmin,ymin,xmax,ymax)
    end

  return hasSegment
end

--[[
----------------------------------
|	ESTRUTURA DA SHORTCUT TREE	 |
----------------------------------
A tree terá uma estrutura própria associada, que será explicada logo abaixo. Cada galho/folha é chamado de tree[ind]. Diferente da versão anterior, teremos apenas uma scene: a original.
	ind -> "0", "01", "02", ..., é o índice associado a uma certa bounding box. Cada número representa a escolha de uma bounding box antecessora na subdivisão:
	 1 para o canto superior esquerdo, 2 para o canto superior direito, 3 para o inferior esquerdo e 4 para o inferior direito. Cada dígito é uma subdivisão.
	tree[ind].boundingBox -> Auto-explicativo, trata-se do bounding box associado à cada subdivisão. É uma tabela cheia no formato {xmin, ymin, xmax, ymax}
	tree[ind].data -> Representa os paths e os segmentos que serão levados em consideração pelos pontos pertencentes àquela região.
	Formato de tabela cheia: {numero_do_path: {numero_do_segmento, numero_do_segmento, ...} ...}
	tree[ind].winding -> Outra tabela cheia, guarda o winding_increment relativo ao path representado pelo índice (O path 1 possui o winding number da posição [1] da table).
	O winding_increment é calculado de acordo com o artigo "Massively Parallel Vector Graphics", 2014.
	tree[ind].seg -> O número de segmentos armazenados.
	tree[ind].depth -> A "profundidade" da árvore, armazena quantas subdivisões fizemos para chegar na região.
	tree[ind].leaf -> Flag que verifica se a região delimitada pelo bounding box é ou não uma folha.
	tree[ind].shortcuts -> Tabela cheia que armazena os shortcuts segments relativos ao path do índice.
]]--

function LoadShortcuts(scene, tree, fatherInd, ind)
  for k in pairs(tree[fatherInd].data) do
    tree[ind].shortcuts[k] = {}
    for index,segment_num in pairs(tree[ind].data[k]) do
      local shortcuts = tree[fatherInd].shortcuts[k]
      if shortcuts ~= nil then
        local n = #shortcuts
        for j =1, n, 4 do
          local x0,y0,sobe,father_segment = unpack(shortcuts, j, j+3)
          if father_segment == segment_num then
            table.insert(tree[ind].shortcuts[k],x0)
            table.insert(tree[ind].shortcuts[k],y0)
            table.insert(tree[ind].shortcuts[k],sobe)
            table.insert(tree[ind].shortcuts[k],father_segment)
          end
        end
      end
    end
  end
end
local function CleanFather(tree, fatherInd)
  -- for k = 1 , #tree[fatherInd].data do
  tree[fatherInd].data = {}
  tree[fatherInd].shortcuts = {}
  tree[fatherInd].segments = 0
-- end
end

function subdivide(scene, tree, fatherInd, maxdepth, maxseg)
  for i=4,1,-1 do
    local ind = fatherInd .. i

    --Inicializa as tables. As duas últimas serão vazias para todos os ind.
    tree[ind] = {}
    tree[ind].data = {}
    tree[ind].winding = {}
    tree[ind].shortcuts = {}

    --Criação de bounding boxes
    local xmin,ymin,xmax,ymax = createBoundingBox(tree[fatherInd].boundingBox, i)
    tree[ind].boundingBox = {xmin,ymin,xmax,ymax}
    tree[ind].depth = tree[fatherInd].depth + 1
    tree[ind].segments = 0

    fillData(scene, tree, fatherInd, ind) -- FUTURE OPTIMIZATION: SAME LOOP
    tree[ind].shortcuts, number = CreateShortcuts(scene, tree[ind].data, tree[ind].boundingBox, ind)
    tree[ind].segments = tree[ind].segments + number
    if i == 1 then CleanFather(tree, fatherInd) end
    if isLeaf(tree, ind, maxdepth, maxseg) == true then
      tree[ind].leaf = true
    else
      tree[ind].leaf = false

      subdivide(scene, tree, ind, maxdepth, maxseg)
    end
  end
end

function initializeTree(new_scene, viewport)
  local tree = {}
  local oxmin, oymin, oxmax, oymax = unpack(viewport,1,4)
    local ind = "0"
    tree[ind] = {}
    tree[ind].boundingBox = {oxmin,oymin,oxmax,oymax}
    tree[ind].depth = 0
    tree[ind].segments = 0
    -- tree[ind].leaf = isLeaf(tree, ind, 3, 100)
    tree[ind].leaf = false
    tree[ind].winding = {}
    tree[ind].shortcuts ={}
    tree[ind].data = {}
    for i=1,#new_scene.shapes do
      tree[ind].data[i] = {}
      tree[ind].winding[i] = 0
      for j=1,#new_scene.shapes[i].instructions do
        table.insert(tree[ind].data[i],j)
      end
    end
    return tree
end

-----------------------------------------
--[[		ACCELERATE FUNCTION 	 ]]--
-----------------------------------------
function applyWindingNumber(test, winding_rule, y0, y1)
	if test then
		if winding_rule == "non-zero" then
			if y0 > y1 then return -1 end
			if y0 < y1 then return  1 end
		else
			return 1
		end
	end

	return 0
end

function _M.accelerate(scene, viewport)

	local new_scene = scene

	local transf = xform.identity()
	local element = 1

	function bgc(self)
		-- body
	end

	function atc(self)
		-- body
	end

	function enc(self)
		-- body
	end

	function bgf(self)
		-- body
	end

	function enf(self)
		-- body
	end

	function bgb(self)
		-- body
	end

	function enb(self)
		-- body
	end

	function bgt(self, depth, xf)
		transf = transf*xf
	end

	function ent(self, depth, xf)
		local inverted = xf:inverse()
		transf = transf*inverted
	end

	function pe(self, winding_rule, shape, paint)
			if shape.type == "circle" then
				accel_circle(scene, shape)
			elseif shape.type ~= "path" then shape = shape:as_path(shape, shape.xf) end

			local pxmin = 10000
			local pymin = 10000
			local pxmax = 0
			local pymax = 0

			function updatePathBoundingBox(x0,y0,x1,y1)
				pxmin = math.min(pxmin, x0, x1)
				pxmax = math.max(pxmax, x0, x1)
				pymin = math.min(pymin, y0, y1)
				pymax = math.max(pymax, y0, y1)
			end

		    local begin = false
		    local xclose, yclose
		    local vxmin, vymin, vxmax, vymax

		    function bcc(self)
		    	scene.segments = scene.segments + 1
          self.coef[#self.coef+1] = {}
          self.bound[#self.bound+1] = {}
          self.diagonal[#self.diagonal+1] = {}
          self.winding[#self.winding + 1] = {}
				  begin = true
			end

			function boc(self)
				  scene.segments = scene.segments + 1
          self.coef[#self.coef+1] = {}
          self.bound[#self.bound+1] = {}
          self.diagonal[#self.diagonal+1] = {}
          self.winding[#self.winding + 1] = {}
				  begin = true
			end

			function ecc(self, x0, y0)
				scene.segments = scene.segments + 1
				if begin == true then
					xclose, yclose = x0, y0
					begin = false
				end
				updatePathBoundingBox(x0,y0,xclose,yclose)
				self.coef[#self.coef+1] = {}

				local vxmin = math.min(x0, xclose)
				local vymin = math.min(y0, yclose)
				local vxmax = math.max(x0, xclose)
				local vymax = math.max(y0, yclose)
				self.bound[#self.bound+1] = {vxmin, vymin, vxmax, vymax}
				self.diagonal[#self.diagonal+1] = {}

				self.winding[#self.winding + 1] =
					function(x0,y0,x1,y1,x,y,winding_rule,count)
						local xmin, ymin, xmax, ymax = unpack(self.bound[count],1,4)
						return applyWindingNumber(horizontal_linear_test(x0,y0,x1,y1,x,y,xmin,ymin,xmax,ymax),winding_rule,y0,yclose)
					end
			end

			function eoc(self, x0, y0)
				scene.segments = scene.segments + 1
				if begin == true then
					xclose, yclose = x0, y0
					begin = false
				end
				updatePathBoundingBox(x0,y0,xclose,yclose)
				self.coef[#self.coef+1] = {}

				local vxmin = math.min(x0, xclose)
				local vymin = math.min(y0, yclose)
				local vxmax = math.max(x0, xclose)
				local vymax = math.max(y0, yclose)
				self.bound[#self.bound+1] = {vxmin, vymin, vxmax, vymax}

				self.diagonal[#self.diagonal+1] = {}
				-- Winding test
				self.winding[#self.winding + 1] =
					function(x0,y0,x1,y1,x,y,winding_rule,count)
						local xmin, ymin, xmax, ymax = unpack(self.bound[count],1,4)
						return applyWindingNumber(horizontal_linear_test(x0,y0,x1,y1,x,y,xmin,ymin,xmax,ymax),winding_rule,y0,yclose)
					end
			end

			function ls(self, x0, y0, x1, y1)
				scene.segments = scene.segments + 1
				if begin == true then
					xclose, yclose = x0, y0
					begin = false
				end

				local vxmin = math.min(x0, x1)
				local vymin = math.min(y0, y1)
				local vxmax = math.max(x0, x1)
				local vymax = math.max(y0, y1)
				self.bound[#self.bound+1] = {vxmin, vymin, vxmax, vymax}

				updatePathBoundingBox(x0,y0,x1,y1)
				self.coef[#self.coef+1] = {}

				self.diagonal[#self.diagonal+1] = {}

				self.winding[#self.winding + 1] =
					function(x0,y0,x1,y1,x,y,winding_rule,count)
						local xmin, ymin, xmax, ymax = unpack(self.bound[count],1,4)
						return applyWindingNumber(horizontal_linear_test(x0,y0,x1,y1,x,y,xmin,ymin,xmax,ymax),winding_rule,y0,y1)
					end
			end

			function cs(self, x0, y0, x1, y1, x2, y2, x3, y3)
				scene.segments = scene.segments + 1
				if begin == true then
					xclose, yclose = x0, y0
					begin = false
				end

				vxmin = math.min(x0, x3)
				vymin = math.min(y0, y3)
				vxmax = math.max(x0, x3)
				vymax = math.max(y0, y3)
				self.bound[#self.bound+1] = {vxmin, vymin, vxmax, vymax}
				local a,b,c,d,e,f,g,h,i,sign = calculate_cubic_coefs(x1-x0,y1-y0,x2-x0,y2-y0,x3-x0,y3-y0)
				self.coef[#self.coef+1] = {a,b,c,d,e,f,g,h,i,sign}
				self.diagonal[#self.diagonal+1] = horizontal_test_linear_segment(x0,y0,x3,y3,x2,y2)
				--Update path bounding box
				updatePathBoundingBox(x0,y0,x3,y3)

				-- Winding rule
				self.winding[#self.winding+1] =
				function(x0,y0,x1,y1,x2,y2,x3,y3,x,y,winding_rule,count)
					local xmin, ymin, xmax, ymax = unpack(self.bound[count],1,4)
					local coefs = self.coef[count]
					local diagonal = self.diagonal[count]
					return applyWindingNumber(horizontal_cubic_test(x0,y0,x1,y1,x2,y2,x3,y3,x,y,coefs,xmin,ymin,xmax,ymax,diagonal),winding_rule,y0,y3)
				end
			end

			function ds(self, x0, y0, dx0, dy0, dx1, dy1, x1, y1)
			end

			function qs(self, x0, y0, x1, y1, x2, y2)
				scene.segments = scene.segments + 1
				if begin == true then
					xclose, yclose = x0, y0
					begin = false
				end

				vxmin = math.min(x0, x2)
				vymin = math.min(y0, y2)
				vxmax = math.max(x0, x2)
				vymax = math.max(y0, y2)
				self.bound[#self.bound+1] = {vxmin, vymin, vxmax, vymax}

				updatePathBoundingBox(x0,y0,x2,y2)
				self.coef[#self.coef+1] = {}
				self.diagonal[#self.diagonal+1] = horizontal_test_linear_segment(x0,y0,x2,y2,x1,y1)
				--Implicitization
				self.winding[#self.winding+1] =
				function(x0,y0,x1,y1,x2,y2,x,y,winding_rule,count)
					local xmin, ymin, xmax, ymax = unpack(self.bound[count],1,4)
					local diagonal = self.diagonal[count]
					return applyWindingNumber(horizontal_quadratic_test(x0,y0,x1,y1,x2,y2,x,y,xmin,ymin,xmax,ymax,diagonal),winding_rule,y0,y2)
				end
			end

			function rqs(self, x0, y0, x1, y1, w1, x2, y2)
				scene.segments = scene.segments + 1
				if begin == true then
					xclose, yclose = x0, y0
					begin = false
				end

				vxmin = math.min(x0, x2)
				vymin = math.min(y0, y2)
				vxmax = math.max(x0, x2)
				vymax = math.max(y0, y2)
				self.bound[#self.bound+1] = {vxmin, vymin, vxmax, vymax}
				self.diagonal[#self.diagonal+1] = horizontal_test_linear_segment(x0,y0,x2,y2,x1/w1,y1/w1)

				updatePathBoundingBox(x0,y0,x2,y2)
				local a,b,c,d,e,sign = calculate_rational_quadratic_coefs(0,0,x1/w1-x0,y1/w1-y0,1,x2-x0,y2-y0)
				self.coef[#self.coef+1] = {a,b,c,d,e,sign}

				self.winding[#self.winding+1] =
				function(x0,y0,x1,y1,w1,x2,y2,x,y,winding_rule,count)
					local xmin, ymin, xmax, ymax = unpack(self.bound[count],1,4)
					return applyWindingNumber(horizontal_rational_quadratic_test(x0,y0,x1,y1,w1,x2,y2,x,y,self.coef[count],xmin,ymin,xmax,ymax,self.diagonal[count]),winding_rule,y0,y2)
				end
			end

			local forward = {
				  diagonal={},
				  bound={},
				  coef={},
				  winding={},
				  begin_closed_contour=bcc,
				  begin_open_contour=boc,
				  end_closed_contour=ecc,
				  end_open_contour=eoc,
				  linear_segment=ls,
				  quadratic_segment=qs,
				  cubic_segment = cs,
				  degenerate_segment=ds,
				  rational_quadratic_segment = rqs
			}

			---DOUBT: Unify the iterates
			if shape.type == "path" then
				local new_path = path.path()
				shape:iterate(filter.monotonize(filter.xform(new_scene.xf*shape.xf*transf, new_path)))
				shape = new_path
				shape:iterate(forward)

				shape.boundingBox = {pxmin, pymin, pxmax, pymax}
				shape.winding = forward.winding
				shape.coef = forward.coef
				shape.bound = forward.bound
			end

			local index = new_scene.elements[element].shape_id
			new_scene.shapes[index] = shape
			element = element + 1
	end

	function se(self, winding_rule, shape)
		element = element + 1
	end

	local forward_scene = {
		data={},
		-- Bracket types
		begin_clip=bgc,
		activate_clip=atc,
		end_clip=enc,
		begin_fade=bgf,
		end_fade=enf,
		begin_blur=bgb,
		end_blur=enb,
		begin_transform=bgt,
		end_transform=ent,
		-- Element types
		painted_element=pe,
		stencil_element=se,
	}

	new_scene.segments = 0
	new_scene:iterate(forward_scene)
    local vxmin, vymin, vxmax, vymax = unpack(viewport, 1, 4)
    local width, height = vxmax-vxmin, vymax-vymin
    new_scene.dimension = {width, height}
    for i=1,#new_scene.paints,1 do
    	local paint = new_scene.paints[i]
    	if paint.type == "linear_gradient" then
    		local x1, x2 = paint.x1, paint.x2
    		local y1,y2 =paint.y1, paint.y2
    		local den = (x2-x1)^2 + (y2-y1)^2
    		local a,b,c = x2-x1,y2-y1,-(x1*(x2-x1)+y1*(y2-y1))
    		paint.a1 = a/den
    		paint.a2 = b/den
    		paint.a3 = c/den
    	end
    end

    local tree = initializeTree(new_scene, viewport)
    subdivide(new_scene,tree,"0",7,20)

   	-- UNIT TEST - TREE[IND].DATA FILLING:
    -- print("Test subdivision: ")
    -- for i=1,#tree["03"].data do
      -- io.write(i,": ")
      -- for j in ipairs(tree["03"].data[i]) do
        -- io.write(tree["03"].data[i][j], " ")
      -- end
      -- io.write("\n")
    -- end

    new_scene.tree = tree
	  return new_scene
end

--------------------------------------------------
--[[	TRANSPARENCY AND GRADIENT FUNCTIONS 	]]--
----------------------------------------------------

local function blend(r2,g2,b2,a2,r1,g1,b1,a1)
	local alpha = a1+(1-a1)*a2
	local r,g,b
	if util.is_almost_zero(alpha) then
		r,g,b = 0,0,0
	else
		r = (a1*r1 + (1-a1)*a2*r2)/alpha
		g = (a1*g1 + (1-a1)*a2*g2)/alpha
		b = (a1*b1 + (1-a1)*a2*b2)/alpha
	end

	return r,g,b,alpha
end

local function rampf(rx,gx,bx,ax,rk,gk,bk,ak, pk, p, px)

	local r = ((pk-p)*rx + (p-px)*rk)/(pk-px)
	local g = ((pk-p)*gx + (p-px)*gk)/(pk-px)
	local b = ((pk-p)*bx + (p-px)*bk)/(pk-px)
	local a = ((pk-p)*ax + (p-px)*ak)/(pk-px)
	return r,g,b,a
end

function wrap_function(t, spread)
	local p

	if spread == "pad" then
		p = math.min(1, math.max(0,t))
	elseif spread == "repeat" then
		p = t - math.floor(t)
	elseif spread == "reflect" then
		p = 2*(t/2 - math.floor(t/2 + 1/2))
		if p < 0 then p = -p end
	elseif spread == "transparent" then
		-- '-10' is just an arbitrary number I use as a flag for transparent spread
		if t<=0 or t>1 then p = -10 end
		if 0 < t and t <= 1 then p = t end
	end

	return p
end

local function LocalizeStop(t, stops)
	local tmax,tmin = -100000, 1000000
	local i_max, i_min = 0,0


	for i = 1, #stops-1 do
	  local t_1 = stops[i][1]
	  local t_2 = stops[i+1][1]
	  if t >= t_1 and t <= t_2 then
	    return t_1, t_2, i,i+1
	  end
	end
	for i = 1, #stops do
	  if stops[i][1] >= tmax then
	    tmax = stops[i][1]
	    i_max = i
	  end
	  if stops[i][1] == tmin and i > i_min then
	  elseif stops[i][1] < tmin then
	    tmin = stops[i][1]
	    i_min = i
	  end
	end
	if t < tmin then
	  return tmin, tmin, i_min, i_min
	elseif t >  tmax then
	  return tmax, tmax, i_max, i_max
	end
end

function findColor(t, ramp, paint)
	local p = wrap_function(t, ramp.spread)
	local opacity = paint.opacity
	if p == -10 then return 0,0,0,0 end

	local stops = ramp.stops
	local px, pk, i1, i2 = LocalizeStop(p, stops)
	local rx, gx, bx, ax = unpack(stops[i1][2], 1, 4)
	local rk, gk, bk, ak = unpack(stops[i2][2], 1, 4)
	local r, g, b, a
	if 0 <= p and p <= stops[1][1] then
		r,g,b,a = unpack(stops[1][2],1,4)
	elseif stops[#stops][1] <= p and p <= 1 then
		r,g,b,a = unpack(stops[#stops][2],1,4)
	else
		r,g,b,a = rampf(rx,gx,bx,ax,rk,gk,bk,ak,pk,p,px, opacity)
	end

	return r,g,b,a
end

function linear_gradient(accel, x, y, paint)
	local a1 = paint.a1
	local a2 = paint.a2
	local a3 = paint.a3
	local ramp = paint.ramp

	local G = accel.xf*paint.xf
	G = G:inverse()
	x,y = G:apply(x,y)

	local t = a1*x + a2*y + a3
	return findColor(t, ramp, paint)
end

function focus_transform(fx, fy, r)
	local a = fx*fx + fy*fy
	local b = 0
	local c = -r*r

	local n, t1, s1, t2, s2 = solvequadratic(a, b, c)
	local r1 = t1/s1
	local r2 = t2/s2
	local t = math.max(r1, r2)

	fx = fx*t + 1.5
	fy = fy*t + 1.5

	return fx,fy
end

function radial_gradient_mapping(paint, px, py)
	local cx, cy, fx, fy, r = paint.cx, paint.cy, paint.fx, paint.fy, paint.r
	px, py = _M.translation(-cx,-cy):apply(px,py)
	fx, fy = _M.translation(-cx,-cy):apply(fx,fy)
	cx, cy = _M.translation(-cx,-cy):apply(cx,cy)
	local m = math.sqrt((px-fx)^2 + (py-fy)^2)
	local c = (px-fx)/m
	local s = (py-fy)/m
	if px-fx>0 then
		M = _M.affinity(c,s,0,-s,c,0,0,0,1)
	else
		M = _M.affinity(-c,-s,0,s,-c,0,0,0,1)
	end

	--M = xform.affinity(1/r, 0, 0, 0, 1/r, 0)
	px,py = M:apply(px,py)
	fx,fy = M:apply(fx,fy)


	if fx*fx + fy*fy > r*r then fx,fy = focus_transform(fx,fy,r) end

	local a = (px-fx)*(px-fx)
	local b = 2*(px-fx)*fx
	local c = fx*fx + fy*fy - r*r
	local n, t1, s1, t2, s2 = solvequadratic(a, b, c)

	local r1 = t1/s1
	local r2 = t2/s2
	local t = math.max(r1,r2)

	local qx = fx + (px-fx)*t
	local qy = fy + (py-fy)*t
	local den = ((qx-fx)*(qx-fx) + (qy-fy)*(qy-fy))

	local ans = math.sqrt(((px-fx)*(px-fx) + (py-fy)*(py-fy))/den)
	return ans
end

function radial_gradient(accel, x, y, paint)
	local ramp = paint.ramp
	local G = accel.xf*paint.xf
	G = G:inverse()
	x,y = G:apply(x,y)

	local t = radial_gradient_mapping(paint, x, y)
	local p = wrap_function(t, ramp.spread)
	local opacity = paint.opacity
	return findColor(t, ramp, paint)
end

local function LinearMapping(x, y, xp1, yp1, xp2, yp2)
	local l = util.dot2(x-xp1,y-yp1,xp2-xp1,yp2-yp1)
	  l = l/(util.dot2(xp2-xp1,yp2-yp1,xp2-xp1,yp2-yp1))
	return l
end

local function BilinearInterpolation(x, y, img)
	local x1 = math.floor(x)
	local y1 = math.floor(y)
	local x2 = math.ceil(x)
	local y2 = math.ceil(y)
	-----edges-----
	if (x1 == 0) and (y1 == 0) then return img:get_pixel(x2,y2)
	elseif (x1 == 0) then
		local r1,g1,b1,a1 = img:get_pixel(x2, y1)
		local r2,g2,b2,a2 = img:get_pixel(x2, y2)
		local r = (y2 - y)*r1 + (y - y1)*r2
		local g = (y2 - y)*g1 + (y - y1)*g2
		local b = (y2 - y)*b1 + (y - y1)*b2
		if a1~=nil and a2~=nil then
			a = (y2 - y)*r1 + (y - y1)*r2 end
		return r,g,b,a
	elseif (y1 == 0) then
		local r1,g1,b1,a1 = img:get_pixel(x1, y2)
		local r2,g2,b2,a2 = img:get_pixel(x2, y2)
		local r = (x2 - x)*r1 + (x - x1)*r2
		local g = (x2 - x)*g1 + (x - x1)*g2
		local b = (x2 - x)*b1 + (x - x1)*b2
		if a1~=nil and a2~=nil then
			a = (y2 - y)*r1 + (y - y1)*r2 end
		return r,g,b,a
	end
	---------------
	local fx1y1,fx1y2,fx2y1,fx2y2,fxy1,fxy2,fxy = {},{},{},{},{},{},{}
	fx1y1[1],fx1y1[2],fx1y1[3],fx1y1[4] = img:get_pixel(x1, y1)
	fx1y2[1],fx1y2[2],fx1y2[3],fx1y2[4] = img:get_pixel(x1, y2)
	fx2y1[1],fx2y1[2],fx2y1[3],fx2y1[4] = img:get_pixel(x2, y1)
	fx2y2[1],fx2y2[2],fx2y2[3],fx2y2[4] = img:get_pixel(x2, y2)
	for i = 1, 4 do
		if (fx1y1[i] ~= nil) and (fx1y2[i] ~= nil) and (fx2y1[i] ~= nil) and (fx2y2[i] ~= nil) then
			fxy1[i] = fx1y1[i]*(x2 - x) + fx2y1[i]*(x - x1)
			fxy2[i] = fx1y2[i]*(x2 - x) + fx2y2[i]*(x - x1)
		end
	end
	for i = 1, 4 do
		if (fxy1[i] ~= nil) and (fxy2[i] ~= nil) then
			fxy[i] = fxy1[i]*(y2 - y) + fxy2[i]*(y - y1)
		end
	end
	return fxy[1],fxy[2],fxy[3],fxy[4]
end


function texture(accel, x, y, paint)
	local spread = paint.spread
	local G = accel.xf*paint.xf
	G = G:inverse()
	x,y = G:apply(x,y)
	local img = paint.image
	local width, height = img.width, img.height
	local tx = LinearMapping(x,0,0,0,width,0)
	local ty = LinearMapping(0,y,0,0,0,height)
	tx = wrap_function(tx*width,spread)
	ty = wrap_function(ty*height,spread)
     	tx = tx*width
	ty = ty*height
	local r,g,b,a = BilinearInterpolation(tx, ty, img)
	if a == nil then a = 1 end
	return r,g,b,a
end

function test_circle(scene, shape, x, y)
        local xt, yt = shape.transf:apply(x, y, 1)

        local xt = xt - shape.cx
        local yt = yt - shape.cy

        if xt*xt + yt*yt < (shape.r)*(shape.r) then return 1 else return 0 end
end

function wind(accel, i, ind, x, y)
  local data = accel.tree[ind].data
 	local element = accel.elements[i]
  local path = accel.shapes[i]
  local paint = accel.paints[i]
  local rec = false
	if path.type == "circle" then
		return test_circle(accel,path,x,y)
	end
  local wind_num = 0
  for i_seg, seg in pairs(data[i]) do
    local instruction = path.instructions[seg]
    local offset = path.offsets[seg]

  	if instruction == "end_open_contour" or instruction == "end_closed_contour" then
  		local x0,y0,len = unpack(path.data,offset,offset+2)
      local offbegin = path.offsets[seg - len]
      local x1, y1 = unpack(path.data, offbegin+1, offbegin+2)
      if path.winding[seg] ~= nil then wind_num = wind_num + path.winding[seg](x0,y0,x1,y1,x,y,element.winding_rule,seg)
        wind_num = wind_num + windshortcuts(accel,ind,i,seg,x,y)
      end

    elseif instruction == "linear_segment" then
  		local x0,y0,x1,y1 = unpack(path.data,offset,offset+3)
      if path.winding[seg] ~= nil then
        wind_num = wind_num + path.winding[seg](x0,y0,x1,y1,x,y,element.winding_rule,seg)
        wind_num = wind_num + windshortcuts(accel,ind,i,seg,x,y)
      end

  	elseif instruction == "quadratic_segment" then
      local x0,y0,x1,y1,x2,y2 = unpack(path.data,offset,offset+5)
      if path.winding[seg] ~= nil then
        wind_num = wind_num + path.winding[seg](x0,y0,x1,y1,x2,y2,x,y,element.winding_rule,seg)
        wind_num = wind_num + windshortcuts(accel,ind,i,seg,x,y)
      end

    elseif instruction == "cubic_segment" then
      local x0,y0,x1,y1,x2,y2,x3,y3 = unpack(path.data,offset,offset+7)
      if path.winding[seg] ~= nil then wind_num = wind_num + path.winding[seg](x0,y0,x1,y1,x2,y2,x3,y3,x,y,element.winding_rule,seg) end

    elseif instruction == "rational_quadratic_segment" then
      local x0,y0,x1,y1,w1,x2,y2 = unpack(path.data,offset,offset+6)
      if path.winding[seg] ~= nil then wind_num = wind_num + path.winding[seg](x0,y0,x1,y1,w1,x2,y2,x,y,element.winding_rule,seg) end
    end

  end
	return wind_num
end

function windshortcuts(accel,ind, i, segment_num, x, y)
  local tree = accel.tree
  local bxmin,bymin,bxmax,bymax = unpack(tree[ind].boundingBox,1,4)
  local shortcuts = tree[ind].shortcuts[i]
  local wind_shorcut = 0
  if shortcuts ~= nil then
  for data = 1, #shortcuts, 4 do
    father_segment = shortcuts[data + 3]
    if segment_num == father_segment then
      x0s, y0s, sobe = unpack(shortcuts, data, data + 2)
      local x0,y0,x1,y1
      if sobe then x0,y0,x1,y1 = x0s,y0s,x0s,bymax
      else x0,y0,x1,y1 = x0s,bymax,x0s,y0s end
      if (y0 - y)*(y1 -y) < 0 then
        if horizontal_test_linear_segment(x0,y0,x1,y1,x,y) then
          wind_shorcut = wind_shorcut + util.sign(y1-y0)
        end
      end
    end
  end
  end
  return wind_shorcut
end

function painting(accel,paint,x,y,r,g,b,a)
	local rx, gx, bx, ax
	if paint.type == "linear_gradient" then rx, gx, bx, ax = linear_gradient(accel,x,y,paint)
	elseif paint.type == "radial_gradient" then rx, gx, bx, ax = radial_gradient(accel,x,y,paint)
	elseif paint.type == "texture" then rx, gx, bx, ax = texture(accel,x,y,paint)
	else rx, gx, bx, ax = unpack(paint.rgba, 1, 4) end
	ax = ax*paint.opacity
	r,g,b,a=blend(rx,gx,bx,ax,r,g,b,a)
	return r,g,b,a
end

--  This is the other function you have to implement.
--  It receives the acceleration datastructure and a sampling
--  position. It returns the color at that position.
local function sample(accel, x, y, path_num)
local r,g,b,a = 0,0,0,0
local rept = false
local tree = accel.tree
local ind = '0'
-- Iterates through the shortcut tree, finds an ind(leaf) for itself
while tree[ind].leaf == false do
  for i = 1, 4 do
    if InsideGrid(tree, ind, x, y) then return 0,0,0,1 end
    local n_ind = ind .. i
    local bb = tree[n_ind].boundingBox
    local xmin, ymin, xmax, ymax = unpack(bb)
    if insideBoundingBox(xmin, ymin, xmax, ymax, x, y) then
      ind = n_ind
      break
    end
  end
end

-- Inside the leaf, iterates through the active segments(Segments it kept during subdivision)
for i = #tree[ind].data, 1, -1 do
  if path_num == nil or (path_num > 0 and i == path_num) then
    local j = tree[ind].data[i]
    if j ~= nil then
      local element = accel.elements[i]
      local shape = accel.shapes[i]
      local paint = accel.paints[i]
      local wind_num = tree[ind].winding[i]
      wind_num = wind_num + wind(accel, i, ind, x, y)
      if (element.winding_rule == "odd" and wind_num%2 == 1) or (element.winding_rule == "non-zero" and wind_num ~= 0) then
    		r,g,b,a = painting(accel,paint,x,y,r,g,b,a)
    	end
    	if util.is_almost_one(a) then
    		return r,g,b,a
    	end
    end
  end
end

return blend(1,1,1,1,r,g,b,a)
end

function gamma_correction(sr,sg,sb,sa,n)
    sr = sr/n
    sg = sg/n
    sb = sb/n
    sa = sa/n

    local r = sr^2.2
    local g = sg^2.2
    local b = sb^2.2
    local a = sa^2.2

    return r,g,b,a
end

local function GaussianKernel(x,y)
  return math.exp(-((x^2)/2 + (y^2)/2))/0.92131
end

local function UniformKernel(x,y)
  if (math.abs(x) <= 1/2) and (math.abs(y) <= 1/2) then
    return 1
  else
    return 0
  end
end

--  This is the other function you have to implement.
--  It receives the acceleration datastructure, the sampling pattern,
--  and a sampling position. It returns the color at that position.
local function supersample(accel, pattern, x, y, path_num, kernel)
    -- Implement your own version
    local sr,sg,sb,sa = 0,0,0,0
    if kernel == nil then
        kernel = UniformKernel
    end
    local W = 0
    for i=1, #pattern-1,2 do
	    local w_i = kernel(pattern[i], pattern[i+1])
    	local rx,gx,bx,ax = sample(accel,x+pattern[i],y+pattern[i+1], path_num)
     	sr = sr + w_i*rx^(1/2.2)
    	sg = sg + w_i*gx^(1/2.2)
    	sb = sb + w_i*bx^(1/2.2)
    	sa = sa + w_i*ax^(1/2.2)
	    W = W + w_i
    end

    return gamma_correction(sr,sg,sb,sa,W)
end

local function parseargs(args)
    local parsed = {
        pattern = blue[1],
        tx = 0,
        ty = 0,
        p = nil,
        dumpcellsprefix = nil,
    }
    -- Available options
    local options = {
        -- Selects a supersampling pattern
        { "^(%-pattern:(%d+)(.*))$", function(all, n, e)
            if not n then return false end
            assert(e == "", "trail invalid option " .. all)
            n = assert(tonumber(n), "number invalid option " .. all)
            assert(blue[n], "non exist invalid option " .. all)
            parsed.pattern = blue[n]
            return true
        end },
        -- Select a single path for rendering
        { "^(%-p:(%d+)(.*))$", function(all, n, e)
            if not n then return false end
            assert(e == "", "trail invalid option " .. all)
            parsed.p = assert(tonumber(n), "number invalid option " .. all)
            return true
        end },
        -- Translates scene by tx,ty pixels before rendering
        { "^(%-tx:(%-?%d+)(.*))$", function(all, n, e)
            if not n then return false end
            assert(e == "", "trail invalid option " .. all)
            parsed.tx = assert(tonumber(n), "number invalid option " .. all)
            return true
        end },
        { "^(%-ty:(%-?%d+)(.*))$", function(all, n, e)
            if not n then return false end
            assert(e == "", "trail invalid option " .. all)
            parsed.ty = assert(tonumber(n), "number invalid option " .. all)
            return true
        end },
        -- Dump cells matching a given prefix
        { "^%-dumpcells:(.*)$", function(n)
            if not n then return false end
            parsed.dumpcellsprefix = n
            return true
        end },
        -- Catch all unrecognized options and throw error
        { ".*", function(all)
            error("unrecognized option " .. all)
        end }
    }
    -- Process options
    for i, arg in ipairs(args) do
        for j, option in ipairs(options) do
            if option[2](arg:match(option[1])) then
                break
            end
        end
    end
    -- Return parsed values
    return parsed
end

-- In theory, you don't have to change this function.
-- It simply allocates the image, samples each pixel center,
-- and saves the image into the file.
function _M.render(scene, viewport, file, args)
    parsed = parseargs(args)
    local pattern = parsed.pattern
    local p = parsed.p
    local tx, ty = parsed.tx, parsed.ty
    -- Get viewport
      local vxmin, vymin, vxmax, vymax = unpack(viewport, 1, 4)
      if tx ~= nil then
        vxmin = vxmin - tx
        vxmax = vxmax - tx
      end
      if ty ~= nil then
        vymin = vymin - ty
        vymax = vymax - ty
      end
      -- Get image width and height from viewport
    local width, height = vxmax-vxmin, vymax-vymin
      -- Allocate output image
    local img = image.image(width, height, 4)
    local time = chronos.chronos()
      -- Rendering loop
      for i = 1, height do
      stderr("\r%5g%%", floor(1000*i/height)/10)
          local y = vymin+i-1.+.5
          for j = 1, width do
              local x = vxmin+j-1.+.5
              img:set_pixel(j, i, supersample(scene, pattern, x, y, p, GaussianKernel))
          end
      end
    stderr("\n")
    stderr("rendering in %.3fs\n", time:elapsed())
    time:reset()
        -- store output image
        image.png.store8(file, img)
    stderr("saved in %.3fs\n", time:elapsed())
    end
  --end

return _M
