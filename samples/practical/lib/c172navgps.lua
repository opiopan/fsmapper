local module = {
    width = 373.333,
    height = 126,
    type = {
        general = 1,
    },
}

local module_defs = {
    prefix = "C172_NAVGPS",
    activatable = false,
    options = {{}},
    option_defaults = {
        type = module.type.general,
    },
}

local common = require("lib/common")
--------------------------------------------------------------------------------------
-- action definitions
--------------------------------------------------------------------------------------
module.actions = {}
module.actions[1] = {
    navgps = fs2020.event_sender("TOGGLE_GPS_DRIVES_NAV1"),
}

--------------------------------------------------------------------------------------
-- operable area definitions
--------------------------------------------------------------------------------------
local attr_normal = {width=165.172, height=67.227, rratio=0.1}
module_defs.operables = {}
module_defs.operables[module.type.general] = {
    navgps = {x=22.135, y=44.257, attr=attr_normal},
}

--------------------------------------------------------------------------------------
-- indicator definitions
--------------------------------------------------------------------------------------
local img_all = graphics.bitmap("assets/c172navgps.png")
local img_base = img_all:create_partial_bitmap(0, 0, module.width, module.height)
local img_nav = img_all:create_partial_bitmap(375, 0, 48.849, 47.172)
local img_gps = img_all:create_partial_bitmap(375, 49, 48.849, 47.172)

local attr_indicator = {width=48.849, height=47.172}
module_defs.indicators ={}
module_defs.indicators[module.type.general] = {}
module_defs.indicators[module.type.general][1]= {
    navgps_indicator = {x=46.163, y=54.047, attr=attr_indicator, bitmaps={img_nav, img_gps}, rpn="(A:GPS DRIVES NAV1, Bool)"},
}

module_defs.indicator_orders = {}
module_defs.indicator_orders[module.type.general] = {
    "navgps_indicator",
}
--------------------------------------------------------------------------------------
-- prepare module scope environment
--------------------------------------------------------------------------------------
common.component_module_init(module, module_defs, true)

--------------------------------------------------------------------------------------
-- instance generator
--------------------------------------------------------------------------------------
function module.create_component(component_name, id, captured_window, x, y, scale, rctx, simhid_g1000)
    local component = common.component_module_create_instance(module, module_defs,{
        name = component_name,
        id = id,
        captured_window = captured_window,
        x = x, y = y, scale = scale,
        simhid_g1000 = simhid_g1000
    })

    -- update view background bitmap
    rctx:draw_bitmap{bitmap=img_base, x=x, y=y, scale=scale}

    return component
end

return module
