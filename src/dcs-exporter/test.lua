local utils = require('fsmapper_utils')
local fmt = utils.struct('c1I3S2')
local s = fmt:pack('V', 'opiopan')
print(''..s:len())
