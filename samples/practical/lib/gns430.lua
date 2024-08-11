local module = {
    width = 1112,
    height = 466,
    type = {
        general = 1,
    },
}

local module_defs = {
    prefix = "GNS430",
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
    comswap = msfs.mfwasm.rpn_executer("(>H:AS430_COMSWAP_Push)"),
    navswap = msfs.mfwasm.rpn_executer("(>H:AS430_NAVSWAP_Push)"),
    rng_dezoom = msfs.mfwasm.rpn_executer("(>H:AS430_RNG_Dezoom)"),
    rng_zoom = msfs.mfwasm.rpn_executer("(>H:AS430_RNG_Zoom)"),
    directto = msfs.mfwasm.rpn_executer("(>H:AS430_DirectTo_Push)"),
    menu = msfs.mfwasm.rpn_executer("(>H:AS430_MENU_Push)"),
    clr = msfs.mfwasm.rpn_executer("(>H:AS430_CLR_Push)"),
    clr_long = msfs.mfwasm.rpn_executer("(>H:AS430_CLR_Push_Long)"),
    ent = msfs.mfwasm.rpn_executer("(>H:AS430_ENT_Push)"),
    cdi = msfs.mfwasm.rpn_executer("(>H:AS430_CDI_Push)"),
    obs = msfs.mfwasm.rpn_executer("(>H:AS430_OBS_Push)"),
    msg = msfs.mfwasm.rpn_executer("(>H:AS430_MSG_Push)"),
    fpl = msfs.mfwasm.rpn_executer("(>H:AS430_FPL_Push)"),
    proc = msfs.mfwasm.rpn_executer("(>H:AS430_PROC_Push)"),
    left_large_knob_inc = msfs.mfwasm.rpn_executer("(>H:AS430_LeftLargeKnob_Right)"),
    left_large_knob_dec = msfs.mfwasm.rpn_executer("(>H:AS430_LeftLargeKnob_Left)"),
    left_small_knob_inc = msfs.mfwasm.rpn_executer("(>H:AS430_LeftSmallKnob_Right)"),
    left_small_knob_dec = msfs.mfwasm.rpn_executer("(>H:AS430_LeftSmallKnob_Left)"),
    left_small_knob_push = msfs.mfwasm.rpn_executer("(>H:AS430_LeftSmallKnob_Push)"),
    right_large_knob_inc = msfs.mfwasm.rpn_executer("(>H:AS430_RightLargeKnob_Right)"),
    right_large_knob_dec = msfs.mfwasm.rpn_executer("(>H:AS430_RightLargeKnob_Left)"),
    right_small_knob_inc = msfs.mfwasm.rpn_executer("(>H:AS430_RightSmallKnob_Right)"),
    right_small_knob_dec = msfs.mfwasm.rpn_executer("(>H:AS430_RightSmallKnob_Left)"),
    right_small_knob_push = msfs.mfwasm.rpn_executer("(>H:AS430_RightSmallKnob_Push)"),
}
--------------------------------------------------------------------------------------
-- operable are definitions
--------------------------------------------------------------------------------------
local attr_swap = {width=49.127, height=74.004, rratio=0.1}
local attr_range = {width=67.501, height=52.437, rratio=0.1}
local attr_left = {width=77.068, height=47.371, rratio=0.1}
local attr_bottom = {width=69.189, height=50.003, rratio=0.1}
module_defs.operables = {}
module_defs.operables[module.type.general] = {
    comswap = {x=183.481, y=71.039, attr=attr_swap},
    navswap = {x=183.481, y=180.925, attr=attr_swap},
    rng_zoom = {x=906.396, y=41.037, attr=attr_range},
    rng_dezoom = {x=1003.637, y=41.037, attr=attr_range},
    directto = {x=902.959, y=110.699, attr=attr_left},
    menu = {x=1002.637, y=110.699, attr=attr_left},
    clr = {x=902.959, y=178.925, attr=attr_left},
    ent = {x=1002.637, y=178.925, attr=attr_left},
    cdi = {x=282.645, y=398.823, attr=attr_bottom},
    obs = {x=409.329, y=398.823, attr=attr_bottom},
    msg = {x=534.746, y=398.823, attr=attr_bottom},
    fpl = {x=659.821, y=398.823, attr=attr_bottom},
    proc = {x=786.505, y=398.823, attr=attr_bottom},
}

--------------------------------------------------------------------------------------
-- captured window placeholder definition
--------------------------------------------------------------------------------------
module_defs.captured_window = {}
module_defs.captured_window[module.type.general] = {x=266, y=52, width=610, height=308}

--------------------------------------------------------------------------------------
-- active indicator difinitions
--------------------------------------------------------------------------------------
module_defs.active_indicators= {}
module_defs.active_indicators[module.type.general] = {
    {x=36.853, y=308.376, width=102.201, height=102.201},
    {x=976.504, y=308.376, width=102.201, height=102.201},
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
    local background = graphics.bitmap("assets/gns430.png")
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
            {event=g1000.EC2Y.increment, action=module.actions[id].left_large_knob_inc},
            {event=g1000.EC2Y.decrement, action=module.actions[id].left_large_knob_dec},
            {event=g1000.EC2X.increment, action=module.actions[id].left_small_knob_inc},
            {event=g1000.EC2X.decrement, action=module.actions[id].left_small_knob_dec},
            {event=g1000.EC2P.down, action=module.actions[id].left_small_knob_push},
            {event=g1000.EC6Y.increment, action=module.actions[id].left_large_knob_inc},
            {event=g1000.EC6Y.decrement, action=module.actions[id].left_large_knob_dec},
            {event=g1000.EC6X.increment, action=module.actions[id].left_small_knob_inc},
            {event=g1000.EC6X.decrement, action=module.actions[id].left_small_knob_dec},
            {event=g1000.EC6P.down, action=module.actions[id].left_small_knob_push},
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
