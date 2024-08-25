-- arg[1]: directory placed the exporter script
-- arg[2]: DCS base directory 

package.cpath = arg[2] .. 'bin-mt\\?.dll;' .. package.cpath
package.path = arg[2] .. 'LuaSocket\\?.lua;' .. package.path

fsmapper = {time = 1}
fsmapper.basedir = arg[1] .. '..\\'

log = {}
function log.write(mod, type, msg)
    print(mod .. ': ' .. msg)
end

function LoGetSelfData()
    return {
        Name = 'F-16C_50'
    }
end

function LoGetModelTime()
    return fsmapper.time
end

function LoGetVersionInfo()
    return {
        ProductName = 'DCS',
        ProductVersion = {1, 2, 3, 4},
    }
end

fsmapper.Device = {
    performClickableAction = function (self, command, value)
        print(string.format("device(%d):performClickableAction(%d, %f)", self.device_id, command, value))
    end,

    get_argument_value = function (self, arg_number)
        return math.floor(fsmapper.time / 2) + arg_number / 10000
    end,
}

function GetDevice(device_id)
    local device = {device_id = device_id}
    setmetatable(device, {__index = fsmapper.Device})
    return device
end

local socket = require('socket')


dofile(arg[1] .. 'fsmapper.lua')

fsmapper.log = function (msg)
    print("debug: " .. msg)
end

if LuaExportStart then LuaExportStart() end

while true do
    if LuaExportBeforeNextFrame then LuaExportBeforeNextFrame() end
    socket.sleep(0.1)
    if LuaExportAfterNextFrame then LuaExportAfterNextFrame() end
    socket.sleep(0.1)
    fsmapper.time = fsmapper.time + 0.2
end

if LuaExportStop then LuaExportStop() end
