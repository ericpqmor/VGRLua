--[[
 - profiler.lua -- A Profiler module for the Lua Programming Language
 -
 - Use as follows (in your main script):
 - ---
 --   local profiler = require"profiler"
 --   local prof = profiler.profiler()
 --   prof:begin()
 --
 --   ... (your script's code)
 --
 --   prof:finish()
 --   prof:write_results("profiler_results.txt")
 - ---
 -
 - You can also write the results directly to stdout, using, instead of `prof:write_results("profiler.txt")`, `prof:print_results()`
 -
 - Please note that this profiler will slow down **significantly** your script, and that will have it's effects in the output of the
 -  profiler. Still, it will be proportional to what it would be without the slowdown.
]]--

local _M = {}
local chronos = require"chronos"
local profiler_meta = {}
profiler_meta.__index = {}
profiler_meta.name = "profiler"

--[[ Create a new profiler object ]]--
function _M.profiler()
  return setmetatable({
    calls={},
    total={},
    this={},
    time=chronos.chronos()
  }, profiler_meta)
end

--[[ Starts the profiler. You should place this at the beginning of your main script. ]]--
function profiler_meta.__index:begin()
  debug.sethook(function(event)
    local i = debug.getinfo(2, "Sln")
    if i == nil then return end
    if i.what ~= 'Lua' then return end
    local func = i.name or (i.source..':'..i.linedefined)
    if event == 'call' then
      self.this[func] = self.time:time()
    else
      if self.this[func] == nil then return end
      local time = self.time:time() - self.this[func]
      self.total[func] = (self.total[func] or 0) + time
      self.calls[func] = (self.calls[func] or 0) + 1
    end
  end, "cr")
end

--[[ Quits the profiling code. You should place this at the end of your main script. ]]--
function profiler_meta.__index:finish()
  debug.sethook()
end

--[[ Prints the profiler results to the console. You should place this after :finish ]]--
function profiler_meta.__index:print_results()
  for f,time in pairs(self.total) do
    print(("Function %s took %.3f seconds after %d calls\n"):format(f, time, self.calls[f]))
  end
end

--[[ Write the profiler results to a file `filename`. You should also place this after :finish ]]--
function profiler_meta.__index:write_results(filename)
  local file = assert(io.open(filename, "w+"))
  for f,time in pairs(self.total) do
    file:write(("Function %s took %.3f seconds after %d calls\n"):format(f, time, self.calls[f]))
  end
  file:close()
end

return _M
