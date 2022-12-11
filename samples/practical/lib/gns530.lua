local module = {
    width = 1112,
    height = 816,
    type = {
        general = 1,
    },
}

local module_defs = {
    prefix = "GNS530",
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
    comswap = fs2020.mfwasm.rpn_executer("(>H:AS530_COMSWAP_Push)"),
    navswap = fs2020.mfwasm.rpn_executer("(>H:AS530_NAVSWAP_Push)"),
    rng_dezoom = fs2020.mfwasm.rpn_executer("(>H:AS530_RNG_Dezoom)"),
    rng_zoom = fs2020.mfwasm.rpn_executer("(>H:AS530_RNG_Zoom)"),
    directto = fs2020.mfwasm.rpn_executer("(>H:AS530_DirectTo_Push)"),
    menu = fs2020.mfwasm.rpn_executer("(>H:AS530_MENU_Push)"),
    clr = fs2020.mfwasm.rpn_executer("(>H:AS530_CLR_Push)"),
    clr_long = fs2020.mfwasm.rpn_executer("(>H:AS530_CLR_Push_Long)"),
    ent = fs2020.mfwasm.rpn_executer("(>H:AS530_ENT_Push)"),
    cdi = fs2020.mfwasm.rpn_executer("(>H:AS530_CDI_Push)"),
    obs = fs2020.mfwasm.rpn_executer("(>H:AS530_OBS_Push)"),
    msg = fs2020.mfwasm.rpn_executer("(>H:AS530_MSG_Push)"),
    fpl = fs2020.mfwasm.rpn_executer("(>H:AS530_FPL_Push)"),
    vnav = fs2020.mfwasm.rpn_executer("(>H:AS530_VNAV_Push)"),
    proc = fs2020.mfwasm.rpn_executer("(>H:AS530_PROC_Push)"),
    left_large_knob_inc = fs2020.mfwasm.rpn_executer("(>H:AS530_LeftLargeKnob_Right)"),
    left_large_knob_dec = fs2020.mfwasm.rpn_executer("(>H:AS530_LeftLargeKnob_Left)"),
    left_small_knob_inc = fs2020.mfwasm.rpn_executer("(>H:AS530_LeftSmallKnob_Right)"),
    left_small_knob_dec = fs2020.mfwasm.rpn_executer("(>H:AS530_LeftSmallKnob_Left)"),
    left_small_knob_push = fs2020.mfwasm.rpn_executer("(>H:AS530_LeftSmallKnob_Push)"),
    right_large_knob_inc = fs2020.mfwasm.rpn_executer("(>H:AS530_RightLargeKnob_Right)"),
    right_large_knob_dec = fs2020.mfwasm.rpn_executer("(>H:AS530_RightLargeKnob_Left)"),
    right_small_knob_inc = fs2020.mfwasm.rpn_executer("(>H:AS530_RightSmallKnob_Right)"),
    right_small_knob_dec = fs2020.mfwasm.rpn_executer("(>H:AS530_RightSmallKnob_Left)"),
    right_small_knob_push = fs2020.mfwasm.rpn_executer("(>H:AS530_RightSmallKnob_Push)"),
}
--------------------------------------------------------------------------------------
-- operable are definitions
--------------------------------------------------------------------------------------
local attr_swap = {width=49.127, height=74.005, rratio=0.1}
local attr_range = {width=51.397, height=74.005, rratio=0.1}
local attr_left = {width=79.97, height=51.706, rratio=0.1}
local attr_bottom = {width=72.026, height=44.897, rratio=0.1}
module_defs.operables = {}
module_defs.operables[module.type.general] = {
    comswap = {x=97.934, y=70.371, attr=attr_swap},
    navswap = {x=97.934, y=250.403, attr=attr_swap},
    rng_zoom = {x=997.002, y=77.613, attr=attr_range},
    rng_dezoom = {x=997.002, y=175.102, attr=attr_range},
    directto = {x=983.283, y=263.553, attr=attr_left},
    menu = {x=983.283, y=337.721, attr=attr_left},
    clr = {x=983.283, y=410.233, attr=attr_left},
    ent = {x=983.283, y=484.401, attr=attr_left},
    cdi = {x=229.676, y=654.401, attr=attr_bottom},
    obs = {x=342.805, y=654.401, attr=attr_bottom},
    msg = {x=461.395, y=654.401, attr=attr_bottom},
    fpl = {x=575.615, y=654.401, attr=attr_bottom},
    vnav = {x=692.744, y=654.401, attr=attr_bottom},
    proc = {x=807.334, y=654.401, attr=attr_bottom},
}

--------------------------------------------------------------------------------------
-- captured window placeholder definition
--------------------------------------------------------------------------------------
module_defs.captured_window = {}
module_defs.captured_window[module.type.general] = {x=186, y=73, width=752, height=538}

--------------------------------------------------------------------------------------
-- active indicator difinitions
--------------------------------------------------------------------------------------
module_defs.active_indicators= {}
module_defs.active_indicators[module.type.general] = {
    {x=37.383, y=645.479, width=102.201, height=102.201},
    {x=984.283, y=641.642, width=102.201, height=102.201},
}

--------------------------------------------------------------------------------------
-- prepare module scope environment
--------------------------------------------------------------------------------------
common.component_module_init(module, module_defs)

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
    local background = graphics.bitmap("assets/gns530.png")
    rctx:draw_bitmap{bitmap=background, x=x, y=y, scale=scale}

    -- Event-Action mappings which are enabled when the component is activated
    if simhid_g1000 then
        local g1000 = simhid_g1000.events
        component.component_mappings = {
            {event=g1000.SW26.down, action=module.actions[id].comswap},
            {event=g1000.SW1.down, action=module.actions[id].navswap},
            {event=g1000.EC8.increment, action=module.actions[id].rng_dezoom},
            {event=g1000.EC8.decrement, action=module.actions[id].rng_zoom},
            {event=g1000.SW27.down, action=module.actions[id].directto},
            {event=g1000.SW28.down, action=module.actions[id].menu},
            {event=g1000.SW31.down, action=module.actions[id].clr},
            {event=g1000.SW31.longpressed, action=module.actions[id].clr_long},
            {event=g1000.SW32.down, action=module.actions[id].ent},
            {event=g1000.SW29.down, action=module.actions[id].fpl},
            {event=g1000.SW30.down, action=module.actions[id].proc},
            {event=g1000.EC4Y.increment, action=module.actions[id].left_large_knob_inc},
            {event=g1000.EC4Y.decrement, action=module.actions[id].left_large_knob_dec},
            {event=g1000.EC4X.increment, action=module.actions[id].left_small_knob_inc},
            {event=g1000.EC4X.decrement, action=module.actions[id].left_small_knob_dec},
            {event=g1000.EC4P.down, action=module.actions[id].left_small_knob_push},
            {event=g1000.EC9Y.increment, action=module.actions[id].right_large_knob_inc},
            {event=g1000.EC9Y.decrement, action=module.actions[id].right_large_knob_dec},
            {event=g1000.EC9X.increment, action=module.actions[id].right_small_knob_inc},
            {event=g1000.EC9X.decrement, action=module.actions[id].right_small_knob_dec},
            {event=g1000.EC9P.down, action=module.actions[id].right_small_knob_push},
        }
    end

    return component
end

return module
