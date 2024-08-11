local module = {
    width = 500,
    height = 500,
    type = {
        general = 1,
    },
}

local module_defs = {
    prefix = "ADF",
    activatable = true,
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
    card_inc = msfs.event_sender("ADF_CARD_INC"),
    card_dec = msfs.event_sender("ADF_CARD_DEC"),
}

--------------------------------------------------------------------------------------
-- indicator definitions
--------------------------------------------------------------------------------------
local parts = require("lib/instrument_parts")

module_defs.indicators ={}
module_defs.indicators[module.type.general] = {}
module_defs.indicators[module.type.general][1]= {
    standard_indicator = {x=40, y=40, attr={width=420, height=420}, bitmaps={parts.adf_starndard.image}},
    bearing_indicator = {
        x=40, y=40, attr={width=420, height=420}, rotation={bitmap=parts.adf_bearing.image, center={x=420/2, y=420/2}},
        rpn="360 (A:ADF CARD:1, Degrees) -"
    },
    needle_indicator = {
        x=91.088, y=91.088, attr={width=317.824, height=317.824}, rotation={bitmap=parts.adf_needle.image, center={x=317.824/2, y=317.824/2}},
        rpn="(A:ADF RADIAL:1, Degrees) 90 -", epsilon=0.5
    },
}

module_defs.indicator_orders = {}
module_defs.indicator_orders[module.type.general] = {
    "standard_indicator",
    "bearing_indicator",
    "needle_indicator",
}

--------------------------------------------------------------------------------------
-- active indicator difinitions
--------------------------------------------------------------------------------------
module_defs.active_indicators= {}
module_defs.active_indicators[module.type.general] = {
    {x=18.44, y=410, width=81.624, height=81.624},
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
    rctx:draw_bitmap{bitmap=parts.base.image, x=x, y=y, scale=scale}
    rctx:draw_bitmap{bitmap=parts.adf_bg.image, x=x + 82.5 * scale, y=y + 82.5 * scale, scale=scale}
    rctx:draw_bitmap{bitmap=parts.adf_knob.image, x=x + 18.44 * scale, y=y + 410 * scale, scale=scale}

    -- Event-Action mappings which are enabled when the component is activated
    if simhid_g1000 then
        local g1000 = simhid_g1000.events
        component.component_mappings = {
            {event=g1000.EC3.increment, action=module.actions[id].card_inc},
            {event=g1000.EC3.decrement, action=module.actions[id].card_dec},
        }
    end

    return component
end

return module
