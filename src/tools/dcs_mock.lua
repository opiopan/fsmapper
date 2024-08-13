-- arg[1]: directory placed the exporter script
-- arg[2]: DCS base directory 

package.cpath = arg[2] .. 'bin-mt\\?.dll;' .. package.cpath
package.path = arg[2] .. 'LuaSocket\\?.lua;' .. package.path

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
    return 100
end

function LoGetVersionInfo()
    return {
        ProductName = 'DCS',
        ProductVersion = {1, 2, 3, 4},
    }
end

local socket = require('socket')

fsmapper = {}
fsmapper.basedir = arg[1] .. '..\\'

dofile(arg[1] .. 'fsmapper.lua')

if LuaExportStart then LuaExportStart() end

while true do
    if LuaExportBeforeNextFrame then LuaExportBeforeNextFrame() end
    socket.sleep(0.1)
    if LuaExportAfterNextFrame then LuaExportAfterNextFrame() end
    socket.sleep(0.1)
end

if LuaExportStop then LuaExportStop() end
