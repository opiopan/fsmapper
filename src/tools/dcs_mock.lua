-- arg[1]: directory placed the exporter script
-- arg[2]: DCS base directory 

package.cpath = arg[2] .. 'bin-mt\\?.dll;' .. package.cpath
package.path = arg[2] .. 'LuaSocket\\?.lua;' .. package.path

local ded_data = {
[[-----------------------------------------
UHF Mode Rotary_placeholder

children are {
-----------------------------------------
UHF Mode Rotary
UHF
}
-----------------------------------------
Selected UHF Frequency
305.00
-----------------------------------------
VHF Label_placeholder

children are {
-----------------------------------------
VHF Label
VHF
}
-----------------------------------------
Selected VHF Frequency
127.00
-----------------------------------------
DED CNI IFF PH

children are {
-----------------------------------------
IFF Modes Label
M
-----------------------------------------
IFF Modes Enabled
   4  
-----------------------------------------
Active Mode 3 Code
1337
}
-----------------------------------------
Steerpoint Use
STPT
-----------------------------------------
Selected Steerpoint
  1
-----------------------------------------
WPT IncDecSymbol
a
-----------------------------------------
System Time
15:06:25
-----------------------------------------
DED CNI TACAN PH

children are {
-----------------------------------------
TACAN Label
T
-----------------------------------------
TACAN Channel
  1
-----------------------------------------
TACAN Band
X
}
]],

[[
-----------------------------------------
UHF Mode Rotary_placeholder

children are {
-----------------------------------------
UHF Mode Rotary
UHF
}
-----------------------------------------
Selected UHF Frequency
305.00
-----------------------------------------
VHF Label_placeholder

children are {
-----------------------------------------
VHF Label
VHF
}
-----------------------------------------
Selected VHF Frequency
127.00
-----------------------------------------
VHF IncDecSymbol
a
-----------------------------------------
DED CNI IFF PH

children are {
-----------------------------------------
IFF Modes Label
M
-----------------------------------------
IFF Modes Enabled
   4  
-----------------------------------------
Active Mode 3 Code
1337
}
-----------------------------------------
Steerpoint Use
STPT
-----------------------------------------
Selected Steerpoint
  1
-----------------------------------------
System Time
15:06:46
-----------------------------------------
DED CNI TACAN PH

children are {
-----------------------------------------
TACAN Label
T
-----------------------------------------
TACAN Channel
  1
-----------------------------------------
TACAN Band
X
}
]],

[[===========================================================
-----------------------------------------
TILS Scratchpad_placeholder

children are {
-----------------------------------------
TILS Scratchpad
      
}
-----------------------------------------
TILS Asterisks_lhs
*
-----------------------------------------
TILS Asterisks_rhs
*
-----------------------------------------
TCN Label
TCN
-----------------------------------------
TCN Mode
REC
-----------------------------------------
ILS Label
ILS
-----------------------------------------
ILS Mode
ON
-----------------------------------------
ILS CMD STRG_placeholder

children are {
-----------------------------------------
ILS CMD STRG
CMD STRG
}
-----------------------------------------
ILS FRQ Label
FRQ
-----------------------------------------
ILS FRQ Scratchpad_placeholder

children are {
-----------------------------------------
ILS FRQ Scratchpad
108.10
}
-----------------------------------------
ILS CRS Label
CRS
-----------------------------------------
ILS CRS Scratchpad_placeholder

children are {
-----------------------------------------
ILS CRS Scratchpad
  0o
}
-----------------------------------------
TCN CHAN Label
CHAN
-----------------------------------------
TCN CHAN Scratchpad_placeholder

children are {
-----------------------------------------
TCN CHAN Scratchpad
  1
}
-----------------------------------------
TCN BAND Label
BAND
-----------------------------------------
TCN BAND XY
X
-----------------------------------------
TCN BAND Key
(0)
]],

[[-----------------------------------------
FIX_SENSORS
FIX
-----------------------------------------
FIX_SelectedSensors_placeholder_inv

children are {
-----------------------------------------
FIX_SelectedSensors_inv
OFLY
}
-----------------------------------------
Asterisks_FIX_SENSORS_lhs
*
-----------------------------------------
Asterisks_FIX_SENSORS_rhs
*
-----------------------------------------
FIX_STPT
STPT
-----------------------------------------
INS_SelectedSteerpoint_placeholder

children are {
-----------------------------------------
INS_SelectedSteerpoint
667
}
-----------------------------------------
INS_STPT_IncDecSymbol
a
-----------------------------------------
FIX_DELTA
DELTA
-----------------------------------------
DELTA
386.06NM
-----------------------------------------
SYS ACCURACY label
SYS ACCUR
-----------------------------------------
SYS ACCURACY value
HIGH
-----------------------------------------
GPS ACCURACY label
GPS ACCUR
-----------------------------------------
GPS ACCURACY value
HIGH
]],

[[-----------------------------------------
DEST_OA1
DEST OA1
-----------------------------------------
DEST_OA1_SelectedSteerpoint_placeholder_inv

children are {
-----------------------------------------
DEST_OA1_SelectedSteerpoint_inv
  3
}
-----------------------------------------
DEST_OA1_STPT_IncDecSymbol
a
-----------------------------------------
Asterisks_NUM_STEERPOINT_lhs
*
-----------------------------------------
Asterisks_NUM_STEERPOINT_rhs
*
-----------------------------------------
DEST_OA1_RNG
RNG
-----------------------------------------
RNG_placeholder

children are {
-----------------------------------------
RNG
    0 FT
}
-----------------------------------------
DEST_OA1_BRG
BRG
-----------------------------------------
BRG_placeholder

children are {
-----------------------------------------
BRG
  0.0o
}
-----------------------------------------
DEST_OA1_ELEV
ELEV
-----------------------------------------
ELEV_placeholder

children are {
-----------------------------------------
ELEV
     0FT
}
-----------------------------------------
P3lbl
P3>
]],
}

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

function list_indication(indicator_id)
    if indicator_id == 6 then
        return ded_data[math.floor(fsmapper.time / 2) % #ded_data + 1]
    else
        return "indicator: " .. indicator_id .. ":" .. math.floor(fsmapper.time / 2)
    end
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
