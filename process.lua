local unpack = unpack or table.unpack

local chronos = require"chronos"

local profiler = require"profiler"

local total = chronos.chronos()

local quiet = false

local function stderr(...)
    if not quiet then
        io.stderr:write(string.format(...))
    end
end

-- print help and exit
local function help()
    io.stderr:write([=[
Usage:
  lua process.lua [options] <driver> [<input.rvg> [<output-name>]]
where options are:
  -profile:<output>    write profiling info to <output> (slows down significantly)
  -width:<number>      set viewport width (and height proportionally if not set)
  -height:<number>     set viewport height (and width proportionally if not set)
]=])
    os.exit()
end

-- locals for width and height override
local width, height
-- locals for driver, input, and output
local drivername, inputname, outputname, profilename, pngs

-- list of supported options
-- in each option,
--   first entry is the pattern to match
--   second entry is a callback
--     if callback returns true, the option is accepted.
--     if callback returns false, the option is rejected.
local options = {
    { "^%-help$", function(w)
        if w then
            help()
            return true
        else
            return false
        end
    end },
    { "^%-quiet$", function(d)
        if not d then return false end
        quiet = true;
        return true
    end },
    { "^%-profile%:(.*)$", function(o)
        if not o or #o < 1 then return false end
        profilename = o
        return true
    end },
    { "^(%-width%:(%d*)(.*))$", function(all, n, e)
        if not n then return false end
        assert(e == "", "invalid option " .. all)
        n = assert(tonumber(n), "invalid option " .. all)
        assert(n >= 1, "invalid option " .. all)
        width = math.floor(n)
        return true
    end },
    { "^(%-height%:(%d+)(.*))$", function(all, n, e)
        if not n then return false end
        assert(e == "", "invalid option " .. all)
        n = assert(tonumber(n), "invalid option " .. all)
        assert(n >= 1, "invalid option " .. all)
        height = math.floor(n)
        return true
    end },
}

-- rejected options are passed to driver
local rejected = {}
local nrejected = 0
-- value do not start with -
local values = {}
local nvalues = 0

-- go over command-line arguments
-- processes recognized options
-- collect unrecognized ones into rejected list,
-- collect values into another list
for i, argument in ipairs({...}) do
    if argument:sub(1,1) == "-" then
         pngstart = argument:sub(4,4)
         pngend = argument:sub(6,6)
        local recognized = false
        for j, option in ipairs(options) do
            if option[2](argument:match(option[1])) then
                recognized = true
                break
            end
        end
        if not recognized then
            nrejected = nrejected + 1
            rejected[nrejected] = argument
        end
    else
        nvalues = nvalues + 1
        values[nvalues] = argument
    end
end
drivername = values[1]
inputname = values[2]
outputname = values[3]
local pngstart = tonumber(pngstart)
local pngend = tonumber(pngend)
-- load driver
assert(drivername, "missing <driver> argument")
local driver = require(drivername)
assert(type(driver) == "table", "invalid driver")

-- load and run the Lua program that defines the scene, window, and viewport
-- the only globals visible are the ones exported by the
stderr("processing %s\n", inputname)
local time = chronos.chronos()
local input
if _VERSION == "Lua 5.1" then
    input = assert(setfenv(assert(loadfile(inputname)), driver)())
else
    input = assert(assert(loadfile(inputname, "bt", driver))())
end
stderr("loaded in %gs\n", time:elapsed())

-- print options that will be passed down to driver
if #rejected > 0 then
    stderr("options passed down to driver\n")
    for i,v in ipairs(rejected) do
        stderr("  %s\n", v)
    end
end

-- by default, dump to stadard out
-- if another argument was given, replace with the open file
--outputname = outputname .. rejected[p]

-- update viewport if width or height were given
local viewport = input.viewport
local vxmin, vymin, vxmax, vymax = unpack(viewport)
local vwidth = vxmax-vxmin
local vheight = vymax-vymin
if width and not height then
    assert(vwidth > 0, "empty viewport")
    vheight = math.floor(vheight*width/vwidth+0.5)
    assert(vheight > 0, "empty viewport")
    vwidth = width
end
if height and not width then
    assert(vheight > 0, "empty viewport")
    vwidth = math.floor(vwidth*height/vheight+0.5)
    assert(vwidth > 0, "empty viewport")
    vheight = height
end
if height and width then
    vwidth = width
    vheight = height
end
viewport = driver.viewport(0, 0, vwidth, vheight)

-- apply window-viewport transformation to scene
local scene = input.scene:windowviewport(input.window,viewport)

local prof
if profilename then
    stderr("profiling...\n")
    prof = profiler.profiler()
    prof:begin()
end

-- invoke driver-defined accelerate() function on scene
-- pass rejected options as last argument
time:reset()
local accel = driver.accelerate(scene, viewport, rejected)
if pngstart == nil then pngstart = 1 end
if pngend == nil then pngend = 1 end

for i=pngstart,pngend do
    if i == pngstart then outputname = outputname:sub(1,string.len(outputname)-4) .. i .. outputname:sub(string.len(outputname)-3,string.len(outputname))
    else outputname = outputname:sub(1,string.len(outputname)-5) .. i .. outputname:sub(string.len(outputname)-3,string.len(outputname)) end

    local output = io.stdout
    if outputname then
        output = assert(io.open(outputname, "wb"))
    end

    if rejected[1] ~= nil then rejected[1] = rejected[1]:sub(1,3) .. tostring(i) end
    stderr("accelerate in %gs\n", time:elapsed())
    -- invoke driver-defined render() on result of accelerate()
    -- pass rejected options as last argument
    driver.render(accel, viewport, output, rejected)

    if profilename then
        stderr("writing profile results into '%s'\n", profilename)
        prof:finish()
        prof:write_results(profilename)
    end

    -- close output file if we created it
    stderr("done in %gs\n", total:elapsed())  
    if outputname then output:close() end  
end