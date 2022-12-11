local module = {
    width = 1112,
    height = 296,
    type = {
        general = 1,
    },
}

local module_defs = {
    prefix = "KAP140",
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
    ap=fs2020.mfwasm.rpn_executer("(A:AUTOPILOT DISENGAGED, Bool) ! if{ (>K:AP_MASTER) (A:AUTOPILOT MASTER, Bool) ! if{ (>H:Generic_Autopilot_Manual_Off) } }"),
    hdg=fs2020.mfwasm.rpn_executer("(>H:KAP140_Push_HDG)"),
    nav=fs2020.mfwasm.rpn_executer("(>H:KAP140_Push_NAV)"),
    apr=fs2020.mfwasm.rpn_executer("(>H:KAP140_Push_APR)"),
    rev=fs2020.mfwasm.rpn_executer("(>H:KAP140_Push_REV)"),
    alt=fs2020.mfwasm.rpn_executer("(>H:KAP140_Push_ALT)"),
    up=fs2020.mfwasm.rpn_executer("(>H:KAP140_Push_UP)"),
    dn=fs2020.mfwasm.rpn_executer("(>H:KAP140_Push_DN)"),
    arm=fs2020.mfwasm.rpn_executer("(>H:KAP140_Push_ARM)"),
    baro=fs2020.mfwasm.rpn_executer("(>H:KAP140_Push_BARO)"),
    baro_long=fs2020.mfwasm.rpn_executer("(>H:KAP140_Long_Push_BARO)"),
    knob_inner_inc=fs2020.mfwasm.rpn_executer("(>H:KAP140_Knob_Inner_inc)"),
    knob_inner_dec=fs2020.mfwasm.rpn_executer("(>H:KAP140_Knob_Inner_dec)"),
    knob_outer_inc=fs2020.mfwasm.rpn_executer("(>H:KAP140_Knob_Outer_inc)"),
    knob_outer_dec=fs2020.mfwasm.rpn_executer("(>H:KAP140_Knob_Outer_dec)"),
}

--------------------------------------------------------------------------------------
-- operable area definitions
--------------------------------------------------------------------------------------
local attr_normal = {width=69.674, height=39.247, rratio=0.05}
module_defs.operables = {}
module_defs.operables[module.type.general] = {
    ap = {x=46.497, y=215.701, attr=attr_normal},
    hdg = {x=270.684, y=215.701, attr=attr_normal},
    nav = {x=376.357, y=215.701, attr=attr_normal},
    apr = {x=485.876, y=215.701, attr=attr_normal},
    rev = {x=590.718, y=215.701, attr=attr_normal},
    alt = {x=700.237, y=215.701, attr=attr_normal},
    up = {x=827.779, y=132.204, attr=attr_normal},
    dn = {x=827.779, y=215.701, attr=attr_normal},
    arm = {x=887.8, y=35.546, attr=attr_normal},
    baro = {x=994.98, y=35.546, attr=attr_normal},
}

--------------------------------------------------------------------------------------
-- captured window placeholder definition
--------------------------------------------------------------------------------------
module_defs.captured_window = {}
module_defs.captured_window[module.type.general] = {x=141, y = 22, width = 658, height = 142}

--------------------------------------------------------------------------------------
-- active indicator difinitions
--------------------------------------------------------------------------------------
module_defs.active_indicators= {}
module_defs.active_indicators[module.type.general] = {
    {x=935.997, y=122.99, width=130, height=130},
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
    local background = graphics.bitmap("assets/kap140.png")
    rctx:draw_bitmap{bitmap=background, x=x, y=y, scale=scale}

    -- Event-Action mappings which are enabled when the component is activated
    if simhid_g1000 then
        local g1000 = simhid_g1000.events
        component.component_mappings = {
            {event=g1000.EC4Y.increment, action=module.actions[id].knob_outer_inc},
            {event=g1000.EC4Y.decrement, action=module.actions[id].knob_outer_dec},
            {event=g1000.EC4X.increment, action=module.actions[id].knob_inner_inc},
            {event=g1000.EC4X.decrement, action=module.actions[id].knob_inner_dec},
        }
        component.viewport_mappings = {
            {event=g1000.SW2.down, action=module.actions[id].ap},
            {event=g1000.SW4.down, action=module.actions[id].hdg},
            {event=g1000.SW5.down, action=module.actions[id].alt},
            {event=g1000.SW6.down, action=module.actions[id].nav},
            {event=g1000.SW8.down, action=module.actions[id].apr},
            {event=g1000.SW9.down, action=module.actions[id].rev},
            {event=g1000.SW11.down, action=module.actions[id].up},
            {event=g1000.SW13.down, action=module.actions[id].dn},
        }
    end
    
    return component
end

return module
