local module = {
    width = 500,
    height = 500,
    type = {
        general = 1,
    },
}

local module_defs = {
    prefix = "DG",
    activatable = true,
    options = {{}},
    option_defaults = {
        type = module.type.general,
        indicator_type = 1,
        red_mark = false,
        heading_bug = false,
    },
}

local common = require("lib/common")

--------------------------------------------------------------------------------------
-- action definitions
--------------------------------------------------------------------------------------
module.actions = {}
module.actions[1] = {
    gyro_drift_inc = fs2020.event_sender("GYRO_DRIFT_INC"),
    gyro_drift_dec = fs2020.event_sender("GYRO_DRIFT_DEC"),
    heading_bug_inc = fs2020.event_sender("HEADING_BUG_INC"),
    heading_bug_dec = fs2020.event_sender("HEADING_BUG_DEC"),
}

--------------------------------------------------------------------------------------
-- indicator definitions
--------------------------------------------------------------------------------------
local parts = require("lib/instrument_parts")

module_defs.indicators = {}
module_defs.indicators[module.type.general] = {}
module_defs.indicators[module.type.general][1]= {
    standard_indicator = {
        x=193, y=97, attr={width=123, height=314.478}, bitmaps={parts.dg_standard.image},
        enable_condition=function (option) return option.indicator_type == 1 end
        },
    standard_red_indicator = {
        x=15, y=15, attr={width=470, height=470}, bitmaps={parts.dg_standard_red.image},
        enable_condition=function (option) return option.red_mark and option.indicator_type == 1 end
    },
    standard_type2_indicator = {
        x=15, y=15, attr={width=470, height=470}, bitmaps={parts.dg_standard_type2.image},
        enable_condition=function (option) return option.indicator_type == 2 end
    },
    heading_bug_indicator = {
        x=52.219, y=52.219, attr={width=197.783*2, height=197.783*2}, rotation={bitmap=parts.dg_heading_bug.image, center={x=197.783, y=197.783}},
        rpn="(A:AUTOPILOT HEADING LOCK DIR, Degrees) (A:HEADING INDICATOR, Degrees) -", epsilon=0.5,
        enable_condition=function (option) return option.heading_bug end
    },
    bearing_indicator = {
        x=40, y=40, attr={width=420, height=420}, rotation={bitmap=parts.dg_bearing.image, center={x=420/2, y=420/2}},
        rpn="360 (A:HEADING INDICATOR, Degrees) -", epsilon=0.5
    },
}

module_defs.indicator_orders = {}
module_defs.indicator_orders[module.type.general] = {
    "standard_indicator",
    "standard_red_indicator",
    "standard_type2_indicator",
    "heading_bug_indicator",
    "bearing_indicator",
}

--------------------------------------------------------------------------------------
-- active indicator difinitions
--------------------------------------------------------------------------------------
module_defs.active_indicators= {}
module_defs.active_indicators[module.type.general] = {
    {x=18.44, y=410, width=81.624, height=81.624},
    {x=400.376, y=410, width=81.624, height=81.624, enable_condition=function (option) return option.heading_bug end},
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
    rctx:draw_bitmap{bitmap=parts.dg_knob_left.image, x=x + 18.44 * scale, y=y + 410 * scale, scale=scale}
    if module_defs.options[id].heading_bug then
        rctx:draw_bitmap{bitmap=parts.dg_knob_right.image, x=x + 400.376 * scale, y=y + 410 * scale, scale=scale}
    end

    -- Event-Action mappings which are enabled when the component is activated
    if simhid_g1000 then
        local g1000 = simhid_g1000.events
        component.component_mappings = {
            {event=g1000.EC3.increment, action=module.actions[id].heading_bug_inc},
            {event=g1000.EC3.decrement, action=module.actions[id].heading_bug_dec},
            {event=g1000.EC4X.increment, action=module.actions[id].gyro_drift_inc},
            {event=g1000.EC4X.decrement, action=module.actions[id].gyro_drift_dec},
            {event=g1000.EC4Y.increment, action=module.actions[id].gyro_drift_inc},
            {event=g1000.EC4Y.decrement, action=module.actions[id].gyro_drift_dec},
        }
    end

    return component
end

return module
