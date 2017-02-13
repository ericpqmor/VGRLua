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

local unpack = unpack or table.unpack
local floor = math.floor

-- Output formatted string to stderr
local function stderr(...)
    io.stderr:write(string.format(...))
end

-- Create driver with all Lua functions needed to build
-- the scene description
local _M = driver.new()

function accel_circle(scene, shape)

    local transformed = scene.xf*shape.xf
    shape.transf = transformed:inverse()
    shape.boundingBox = {shape.cx-shape.r,shape.cy-shape.r,shape.cx+shape.r,shape.cy+shape.r}

    return viewport
end

function insideBoundingBox(boundingBox, x, y)
	local xmin, ymin, xmax, ymax = unpack(boundingBox, 1, 4)
	return xmin <= x and x < xmax and ymin <= y and y < ymax 
end

function test_linear_segment(x1, y1, x2, y2, x, y)
		local valor = (y2 - y1)*x + (x1 - x2)*y - x1*(y2 - y1) - y1*(x1 - x2)
		return (y2 - y1)*valor < 0 
end

----------------------------------------
--[[ 	QUADRATICS FUNCTIONS 		]]--
----------------------------------------

function implicit_quadratic_test(x0,y0,x1,y1,x2,y2,x,y)
		local valor = 4*(x*y1-x1*y)*(x2*y1-x1*y2)-((2*x1-x2)*y+(y2-2*y1)*x)*((2*x1-x2)*y+(y2-2*y1)*x)
		local sign = 2*y2*(x1*y2 - x2*y1)
		if sign > 0 then valor = -valor end

		return valor < 0
end

function quadratic_test(x0,y0,x1,y1,x2,y2,x,y,winding_rule,xmin,ymin,xmax,ymax,diagonal)
	local test = false
	if ymin <= y and y < ymax and x <= xmax then
		if x <= xmin then test = true
		else
			if test_linear_segment(x0,y0,x2,y2,x1,y1) == true then
				if test_linear_segment(x0,y0,x2,y2,x,y) == true then
					--Degeneration
					if (y1-y0)*(x2-x0) == (y2-y0)*(x1-x0) then
						test = test_linear_segment(x0,y0,x2,y2,x,y)
					else
						test = implicit_quadratic_test(0,0,x1-x0,y1-y0,x2-x0,y2-y0,x-x0,y-y0)
					end
				else
					test = false
				end
			else
				if test_linear_segment(x0,y0,x2,y2,x,y) == true then
					test = true
				else
					if (y1-y0)*(x2-x0) == (y2-y0)*(x1-x0) then
						test = test_linear_segment(x0,y0,x2,y2,x,y)
					else
						test = implicit_quadratic_test(0,0,x1-x0,y1-y0,x2-x0,y2-y0,x-x0,y-y0)
					end
				end
			end	
		end
	end

	if test then
		if winding_rule == "non-zero" then
			if y0 > y2 then return -1 end
			if y0 < y2 then return  1 end
		else
			return 1
		end
	end

	return 0
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

function implicit_rational_quadratic_test(a,b,c,d,e,sign,x,y)
	local valor =y*(a*y + b) + x*(c + y*d + x*e)
	return valor*sign < 0
end

function rational_quadratic_test(x0,y0,x1,y1,w1,x2,y2,x,y,winding_rule,coefs,xmin,ymin,xmax,ymax,diagonal)
	local test = false
	local rec = false
	--if x==45.5 and y==132.5 then rec = true end
	local a,b,c,d,e,sign = unpack(coefs,1,6)

	if ymin <= y and y < ymax and x <= xmax then
		if x <= xmin then test = true
		else
		--Inside the bounding box. Diagonal test
			if diagonal == true then
				if test_linear_segment(x0,y0,x2,y2,x,y) == true then
					test = implicit_rational_quadratic_test(a,b,c,d,e,sign,x-x0,y-y0)
				else
					test = false
				end
			else
				--io.write("I am here being tested by: ", x0, " ", y0, " ", x1, " ", y1, " ", x2, " ", y2, "\n")
				if test_linear_segment(x0,y0,x2,y2,x,y) == true then
					test = true
				else
					test = implicit_rational_quadratic_test(a,b,c,d,e,sign,x-x0,y-y0)
				end
			end	
		end
	end

	if test then
		if winding_rule == "non-zero" then
			if y0 > y2 then return -1 end
			if y0 < y2 then return  1 end
		else
			return 1
		end
	end

	return 0
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

function implicit_cubic_test(a,b,c,d,e,f,g,h,i,sign,x,y)
	local valor = y*(a + y*(b*y + c)) + x*(d + y*(e + y*f) + x*(g + y*h + x*i))
	return sign*valor <	 0 
end

function insideTriangle(x0,y0,x1,y1,x2,y2,x3,y3,winding_rule,x,y)
	local den = (x3-x2)*y1-x1*(y3-y2)
	local xi, yi = x1*(x3*y2-x2*y3)/den, y1*(x3*y2-x2*y3)/den

	local wind_num = 0
	wind_num = wind_num + linear_test(x0,y0,x3,y3,x,y,"odd",math.min(x0,x3),math.min(y0,y3),math.max(x0,x3),math.max(y0,y3)) 
	wind_num = wind_num + linear_test(x3,y3,xi,yi,x,y,"odd",math.min(x3,xi),math.min(y3,yi),math.max(x3,xi),math.max(y3,yi)) 
	wind_num = wind_num + linear_test(x0,y0,xi,yi,x,y,"odd",math.min(x0,xi),math.min(y0,yi),math.max(x0,xi),math.max(y0,yi))

	return wind_num == 1
end

function cubic_test(x0,y0,x1,y1,x2,y2,x3,y3,x,y,winding_rule,coefs,xmin,ymin,xmax,ymax,diagonal)
	local rec = false
	--if x == 115.5 and y == 35.5 then rec = true end
	local test = false
	local a,b,c,d,e,f,g,h,i,sign = unpack(coefs,1,10)
	--Do not forget degeneration cases
	--print(classify(x0,y0,x1,y1,x2,y2,x3,y3))
	if x0 == x1 and y0 == y1 then
		return quadratic_test(x0,y0,x2,y2,x3,y3,x,y,winding_rule,xmin,ymin,xmax,ymax,diagonal)
	elseif x2 == x3 and y2 == y3 then
		return quadratic_test(x0,y0,x1,y1,x2,y2,x,y,winding_rule,xmin,ymin,xmax,ymax,diagonal)
	end

	if bezier.classify3(x0,y0,x1,y1,x2,y2,x3,y3) == "line or point" then
		return linear_test(x0,y0,x3,y3,x,y, winding_rule,xmin,ymin,xmax,ymax)
	elseif bezier.classify3(x0,y0,x1,y1,x2,y2,x3,y3) == "quadratic" then
		return quadratic_test(x0,y0,x1,y1,x3,y3,x,y,winding_rule,xmin,ymin,xmax,ymax,diagonal)

	end

	if ymin <= y and y < ymax and x <= xmax then
		if x <= xmin then test = true
		else
		--Inside the bounding box. Diagonal test
			if diagonal == true then
				if test_linear_segment(x0,y0,x3,y3,x,y) == true then
					if insideTriangle(0,0,x1-x0,y1-y0,x2-x0,y2-y0,x3-x0,y3-y0,winding_rule,x-x0,y-y0) == true then
						test = implicit_cubic_test(a,b,c,d,e,f,g,h,i,sign,x-x0,y-y0)
					else
						test = true
					end
				else
					test = false
				end
			else
				if test_linear_segment(x0,y0,x3,y3,x,y) == true then
					test = true
				else
					if insideTriangle(0,0,x1-x0,y1-y0,x2-x0,y2-y0,x3-x0,y3-y0,winding_rule,x-x0,y-y0) == true then
						test = implicit_cubic_test(a,b,c,d,e,f,g,h,i,sign,x-x0,y-y0)
					else
						test = false
					end
				end
			end	
		end
	end

	if test then
		if winding_rule == "non-zero" then
			if y0 > y3 then return -1 end
			if y0 < y3 then return  1 end
		else
			return 1
		end
	end

	return 0
end

function linear_test(x0,y0,x1,y1,x,y,winding_rule,xmin,ymin,xmax,ymax)
	local test = false
	local wind_num = 0

	if ymin <= y and y < ymax and x <= xmax then
		if x <= xmin then test = true
		else test = test_linear_segment(x0, y0, x1, y1, x, y) end
	end

	if test then
		if winding_rule == "non-zero" then
			if y0 > y1 then wind_num = -1 end
			if y0 < y1 then wind_num =  1 end
		else
			wind_num = 1
		end
	end

	return wind_num
end

function _M.accelerate(scene, viewport)

	local new_scene = scene

	local transf = xform.identity()
	local element = 1

	function bgc(self)
	end

	function atc(self)
	end

	function enc(self)
	end

	function bgf(self)
	end

	function enf(self)
	end

	function bgb(self)
	end

	function enb(self)
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
				begin = true
			end
		  
			function boc(self) 
				begin = true
			end
		 
			function ecc(self, x0, y0)
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
					function(x, y, xclose, yclose, winding_rule, count)
						local xmin, ymin, xmax, ymax = unpack(self.bound[count],1,4)
						return linear_test(x0,y0,xclose,yclose,x,y,winding_rule,xmin,ymin,xmax,ymax)
					end
			end

			function eoc(self, x0, y0)
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
					function(x, y, xclose, yclose, winding_rule, count)
						local xmin, ymin, xmax, ymax = unpack(self.bound[count],1,4)
						return linear_test(x0,y0,xclose,yclose,x,y,winding_rule,xmin,ymin,xmax,ymax)
					end
			end

			function ls(self, x0, y0, x1, y1)
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
					function(x, y, winding_rule, count)
						local xmin, ymin, xmax, ymax = unpack(self.bound[count],1,4)
						return linear_test(x0,y0,x1,y1,x,y,winding_rule,xmin,ymin,xmax,ymax)
					end
			end

			function cs(self, x0, y0, x1, y1, x2, y2, x3, y3)
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
				self.diagonal[#self.diagonal+1] = test_linear_segment(x0,y0,x3,y3,x2,y2)
				--Update path bounding box
				updatePathBoundingBox(x0,y0,x3,y3)

				-- Winding rule
				self.winding[#self.winding+1] =
				function(x,y,winding_rule,count,x0,y0,x1,y1,x2,y2,x3,y3)
					local xmin, ymin, xmax, ymax = unpack(self.bound[count],1,4)
					--io.write("Coefficients: ", x0, " ", y0, " ", x1, " ", y1, " ", x2, " ", y2, " ", x3, " ", y3, "\n")
					local coefs = self.coef[count]
					local diagonal = self.diagonal[count]
					return cubic_test(x0,y0,x1,y1,x2,y2,x3,y3,x,y,winding_rule,coefs,xmin,ymin,xmax,ymax,diagonal)
				end
			end

			function ds(self, x0, y0, dx0, dy0, dx1, dy1, x1, y1) 
			end
		  
			function qs(self, x0, y0, x1, y1, x2, y2)
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
				self.diagonal[#self.diagonal+1] = test_linear_segment(x0,y0,x2,y2,x1,y1)
				--Implicitization
				self.winding[#self.winding+1] = 
				function(x,y,winding_rule,count)
					local xmin, ymin, xmax, ymax = unpack(self.bound[count],1,4)
					local diagonal = self.diagonal[count]
					return quadratic_test(x0,y0,x1,y1,x2,y2,x,y,winding_rule,xmin,ymin,xmax,ymax,diagonal)
				end			
			end

			function rqs(self, x0, y0, x1, y1, w1, x2, y2)
				if begin == true then
					xclose, yclose = x0, y0
					begin = false
				end

				vxmin = math.min(x0, x2)
				vymin = math.min(y0, y2)
				vxmax = math.max(x0, x2)
				vymax = math.max(y0, y2)
				self.bound[#self.bound+1] = {vxmin, vymin, vxmax, vymax}
				self.diagonal[#self.diagonal+1] = test_linear_segment(x0,y0,x2,y2,x1/w1,y1/w1)

				updatePathBoundingBox(x0,y0,x2,y2)
				local a,b,c,d,e,sign = calculate_rational_quadratic_coefs(0,0,x1/w1-x0,y1/w1-y0,1,x2-x0,y2-y0)
				self.coef[#self.coef+1] = {a,b,c,d,e,sign}

				self.winding[#self.winding+1] =
				function(x,y,winding_rule,count)
					local xmin, ymin, xmax, ymax = unpack(self.bound[count],1,4)
					return rational_quadratic_test(x0,y0,x1,y1,w1,x2,y2,x,y,winding_rule,self.coef[count],xmin,ymin,xmax,ymax,self.diagonal[count])
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

	new_scene:iterate(forward_scene)
    local vxmin, vymin, vxmax, vymax = unpack(viewport, 1, 4)
    local width, height = vxmax-vxmin, vymax-vymin
    new_scene.dimension = {width, height}
    --if new_scene.dimension ~= nil then io.write("Different\n") end
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

    local tree
    tree.leaf = false
    tree.sub = {}
    tree.boundingBox = viewport

    vxmin, vymin, vxmax, vymax = unpack(tree.boundingBox,1,4)

    dx = 0.5*(vxmin+vxmax)
    dy = 0.5*(vymin+vymax)

    tree.sub[1] = scene.scene().path

    tree.sub[1].leaf = true
    tree.sub[1].boundingBox = {vxmin+dx,vxmin+2*dx,vymin,vymin+dy}


	return new_scene
end

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

function findColor(t, ramp, paint)
	local p = wrap_function(t, ramp.spread)
	local opacity = paint.opacity
	if p == -10 then return 0,0,0,0 end

	local stops = ramp.stops
	local px, pk, i1, i2 = LocalizeStop(p, stops)
	--print(px,pk,med)
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

	--print(px,py,fx,fy)

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

function texture(accel, x, y, paint)
	local spread = paint.spread
	local G = accel.xf*paint.xf
	G = G:inverse()
	x,y = G:apply(x,y)
	 local img = paint.image
	local width, height = img.width, img.height
	--print(width,height)
	local tx = LinearMapping(x,0,0,0,width,0)
	local ty = LinearMapping(0,y,0,0,0,height)
	tx = wrap_function(tx*width,spread)
	ty = wrap_function(ty*height,spread)
    tx = math.ceil(tx*width)
    ty = math.ceil(ty*height)
   -- print(tx,ty)
	local r,g,b,a = img:get_pixel(tx, ty)
	if a == nil then a = 1 end
	return r,g,b,a
end

function test_circle(scene, shape, x, y)
        local xt, yt = shape.transf:apply(x, y, 1)
        
        local xt = xt - shape.cx
        local yt = yt - shape.cy

        if xt*xt + yt*yt < (shape.r)*(shape.r) then return 1 else return 0 end
end

function wind(accel, i, x, y)
 	local element = accel.elements[i]
    local path = accel.shapes[i]
    local paint = accel.paints[i]
	local wind_num = 0
	local cont = 1

	if path.type == "circle" then
		return test_circle(accel,path,x,y)
	end

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
			wind_num = wind_num + path.winding[cont](x,y,xclose,yclose,element.winding_rule,cont)
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
local function sample(accel, x, y, p)
    local r,g,b,a = 0,0,0,0
    local singlePath = false
    if (p ~= nil and p > 0) then singlePath = true end
 	for i = #accel.elements,1,-1 do
	 	if singlePath == false or (singlePath == true and i == p) then 
	 		local element = accel.elements[i]
	    	local shape = accel.shapes[i]
	    	local paint = accel.paints[i]
	    	if insideBoundingBox(shape.boundingBox, x, y) == true then
	    		local wind_num = wind(accel,i,x,y) + scene.wind_increment
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

function treeIterate(tree,x,y)
	for i=1,#tree.sub,1 do
		if insideBoundingBox(tree.sub[i].boundingBox,x,y)==true then return tree.sub[i] end
	end 
end

function gamma_correction(sr,sg,sb,sa,n)
	sr = 2*sr/n
    sg = 2*sg/n
    sb = 2*sb/n
    sa = 2*sa/n

    local r = sr^2.2
    local g = sg^2.2
    local b = sb^2.2
    local a = sa^2.2

    return r,g,b,a
end

--  This is the other function you have to implement.
--  It receives the acceleration datastructure, the sampling pattern,
--  and a sampling position. It returns the color at that position.
local function supersample(tree, pattern, x, y, p)
	local accel
	while accel.leaf == false do
		accel = treeIterate(tree,x,y) 
	end
    -- Implement your own version
    local r,g,b,a
    local sr,sg,sb,sa = 0,0,0,0
    for i=1, #pattern-1,2 do
    	local rx,gx,bx,ax = sample(accel,x+pattern[i],y+pattern[i+1], p)
     	sr = sr + rx^(1/2.2)
    	sg = sg + gx^(1/2.2)
    	sb = sb + bx^(1/2.2)
    	sa = sa + ax^(1/2.2)
    end

    return gamma_correction(sr,sg,sb,sa,#pattern)
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
function _M.render(tree, viewport, file, args)
    LocalizeStopl parsed = parseargs(args)
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
            img:set_pixel(j, i, supersample(tree, pattern, x, y, p))
        end
    end
	stderr("\n")
	stderr("rendering in %.3fs\n", time:elapsed())
	time:reset()
	    -- store output image
	    image.png.store8(file, img)
	stderr("saved in %.3fs\n", time:elapsed())
end

return _M
