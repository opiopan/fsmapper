local module = {
    width = 1112,
    height = 260,
    type = {
        general = 1,
    },
}

local module_defs = {
    prefix = "KR87",
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
    adf=msfs.mfwasm.rpn_executer("(>H:adf_AntAdf)"),
    bfo=msfs.mfwasm.rpn_executer("(>H:adf_bfo)"),
    frq=msfs.mfwasm.rpn_executer("(>H:adf_frqTransfert)"),
    flt_et=msfs.mfwasm.rpn_executer("(>H:FltEt)"),
    set_rst=msfs.mfwasm.rpn_executer("(>H:SetRst)"),
    knob_large_inc=msfs.mfwasm.rpn_executer("(>K:ADF_100_INC)"),
    knob_large_dec=msfs.mfwasm.rpn_executer("(>K:ADF_100_DEC)"),
    knob_small_inc=msfs.mfwasm.rpn_executer("(L:XMLVAR_ADF_Frequency_10_Khz) if{ (>K:ADF_10_INC) } els{ (>K:ADF_1_INC) }"),
    knob_small_dec=msfs.mfwasm.rpn_executer("(L:XMLVAR_ADF_Frequency_10_Khz) if{ (>K:ADF_10_DEC) } els{ (>K:ADF_1_DEC) }"),
    knob_push=msfs.mfwasm.rpn_executer("(L:XMLVAR_ADF_Frequency_10_Khz) ! (>L:XMLVAR_ADF_Frequency_10_Khz)"),
}
--------------------------------------------------------------------------------------
-- operable are definitions
--------------------------------------------------------------------------------------
local attr_normal = {width=62.203, height=36.762, rratio=0.05}
module_defs.operables = {}
module_defs.operables[module.type.general] = {
    adf = {x=229.467, y=182.609, attr=attr_normal},
    bfo = {x=327.91, y=182.609, attr=attr_normal},
    frq = {x=426.969, y=182.609, attr=attr_normal},
    flt_et = {x=524.898, y=182.609, attr=attr_normal},
    set_rst = {x=625.701, y=182.609, attr=attr_normal},
}

--------------------------------------------------------------------------------------
-- captured window placeholder definition
--------------------------------------------------------------------------------------
module_defs.captured_window = {}
module_defs.captured_window[module.type.general] = {x=28, y = 16, width = 704, height = 131}

--------------------------------------------------------------------------------------
-- active indicator difinitions
--------------------------------------------------------------------------------------
module_defs.active_indicators= {}
module_defs.active_indicators[module.type.general] = {
    {x=877.248, y=52.11, width=160.909, height=160.909},
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
    local background = graphics.bitmap("assets/kr87.png"):create_partial_bitmap(0, 0, module.width, module.height)
    rctx:draw_bitmap{bitmap=background, x=x, y=y, scale=scale}

    -- Event-Action mappings which are enabled when the component is activated
    if simhid_g1000 then
        local g1000 = simhid_g1000.events
        component.component_mappings = {
            {event=g1000.EC2Y.increment, action=module.actions[id].knob_large_inc},
            {event=g1000.EC2Y.decrement, action=module.actions[id].knob_large_dec},
            {event=g1000.EC2X.increment, action=module.actions[id].knob_small_inc},
            {event=g1000.EC2X.decrement, action=module.actions[id].knob_small_dec},
            {event=g1000.EC2P.down, action=module.actions[id].knob_push},
            {event=g1000.EC9Y.increment, action=module.actions[id].knob_large_inc},
            {event=g1000.EC9Y.decrement, action=module.actions[id].knob_large_dec},
            {event=g1000.EC9X.increment, action=module.actions[id].knob_small_inc},
            {event=g1000.EC9X.decrement, action=module.actions[id].knob_small_dec},
            {event=g1000.EC9P.down, action=module.actions[id].knob_push},
            {event=g1000.SW1.down, action=module.actions[id].frq},
        }
    end
    
    return component
end

return module
