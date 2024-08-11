local view_width = 1084
local view_height = 1541

local assets = require("a32nx/assets")
local fcu_disp = require('a32nx/fcu_disp')

local mod_context = {}

--------------------------------------------------------------------------------------
-- register events
--------------------------------------------------------------------------------------
local events = {
    spdmach_push = mapper.register_event("FCU:SPD_MACH:push"),
    loc_push = mapper.register_event("FCU:SPD_MACH:push"),
    loc_change = mapper.register_event("FCU:SPD_MACH:change"),
    hdgtrk_push = mapper.register_event("FCU:HDG_TRK:push"),
    ap1_push = mapper.register_event("FCU:AP1:push"),
    ap1_change = mapper.register_event("FCU:AP1:change"),
    ap2_push = mapper.register_event("FCU:AP2:push"),
    ap2_change = mapper.register_event("FCU:AP2:change"),
    athr_push = mapper.register_event("FCU:A/THR:push"),
    athr_change = mapper.register_event("FCU:A/THR:change"),
    exped_push = mapper.register_event("FCU:EXPD:push"),
    exped_change = mapper.register_event("FCU:EXPD:change"),
    metricalt_push = mapper.register_event("FCU:METRIC ALT:push"),
    appr_push = mapper.register_event("FCU:APPR:push"),
    appr_change = mapper.register_event("FCU:APPR:change"),
    fd_push = mapper.register_event("FCU:fd:push"),
    fd_change = mapper.register_event("FCU:fd:change"),
    ls_push = mapper.register_event("FCU:LS:push"),
    ls_change = mapper.register_event("FCU:LS:push"),
    hghpa_push = mapper.register_event("FCU:inHg_hPa:push"),
    baro_mode_change = mapper.register_event("FCU:BARO Mode:change"),
    baro_unit_change = mapper.register_event("FCU:BARO Unit:change"),
    baro_inhg_change = mapper.register_event("FCU:BARO InHg:change"),
    baro_hpa_change = mapper.register_event("FCU:BARO hPa:change"),
}

--------------------------------------------------------------------------------------
-- observed data definitions
--------------------------------------------------------------------------------------
local observed_data = {
    {rpn="(L:A32NX_FCU_LOC_MODE_ACTIVE)", event=events.loc_change},
    {rpn="(L:A32NX_AUTOPILOT_1_ACTIVE)", event=events.ap1_change},
    {rpn="(L:A32NX_AUTOPILOT_2_ACTIVE)", event=events.ap2_change},
    {rpn="(L:A32NX_AUTOTHRUST_STATUS)", event=events.athr_change},
    {rpn="(L:A32NX_FMA_EXPEDITE_MODE)", event=events.exped_change},
    {rpn="(L:A32NX_FCU_APPR_MODE_ACTIVE)", event=events.appr_change},
    {rpn="(A:AUTOPILOT FLIGHT DIRECTOR ACTIVE:1,Bool) (L:A32NX_ELEC_DC_1_BUS_IS_POWERED) (L:A32NX_ELEC_DC_2_BUS_IS_POWERED) or and", event=events.fd_change},
    {rpn="(L:BTN_LS_1_FILTER_ACTIVE)", event=events.ls_change},
    {rpn="(L:A32NX_ELEC_DC_1_BUS_IS_POWERED) (L:A32NX_ELEC_DC_2_BUS_IS_POWERED) or not -5 * (L:XMLVAR_Baro1_Mode) +", event=events.baro_mode_change},
    {rpn="(L:XMLVAR_BARO_SELECTOR_HPA_1)", event=events.baro_unit_change},
    {rpn="(A:KOHLSMAN SETTING HG:1,Inches of Mercury)", event=events.baro_inhg_change},
    {rpn="(A:KOHLSMAN SETTING MB:1,Millibars)", event=events.baro_hpa_change},
}
for i, def in ipairs(fcu_disp.observed_data) do
    observed_data[#observed_data + 1] = def
end

--------------------------------------------------------------------------------------
-- event-action mappings
--------------------------------------------------------------------------------------
local view_mappings = {
    {event=events.spdmach_push, action=msfs.mfwasm.rpn_executer("(>H:A320_Neo_FCU_SPEED_TOGGLE_SPEED_MACH)")},
    {event=events.loc_push, action=msfs.mfwasm.rpn_executer("(L:A32NX_AUTOPILOT_LOC_MODE, bool) if{ 0 (>L:A32NX_AUTOPILOT_APPR_MODE) 0 (>L:A32NX_AUTOPILOT_LOC_MODE) (>K:AP_LOC_HOLD) } els{ 0 (>L:A32NX_AUTOPILOT_APPR_MODE) 1 (>L:A32NX_AUTOPILOT_LOC_MODE) (>K:AP_LOC_HOLD) }")},
    {event=events.hdgtrk_push, action=msfs.mfwasm.rpn_executer("(>K:A32NX.FCU_TRK_FPA_TOGGLE_PUSH)")},
    {event=events.ap1_push, action=msfs.mfwasm.rpn_executer("(>K:A32NX.FCU_AP_1_PUSH)")},
    {event=events.ap2_push, action=msfs.mfwasm.rpn_executer("(>K:A32NX.FCU_AP_2_PUSH)")},
    {event=events.athr_push, action=msfs.mfwasm.rpn_executer("(>K:A32NX.FCU_ATHR_PUSH)")},
    {event=events.exped_push, action=msfs.mfwasm.rpn_executer("(>H:A320_Neo_FCU_EXPED_PUSH)")},
    {event=events.metricalt_push, action=msfs.mfwasm.rpn_executer("(L:A32NX_METRIC_ALT_TOGGLE, bool) ! (>L:A32NX_METRIC_ALT_TOGGLE)")},
    {event=events.appr_push, action=msfs.mfwasm.rpn_executer("(>K:A32NX.FCU_APPR_PUSH)")},
    {event=events.fd_push, action=msfs.mfwasm.rpn_executer("1 (>K:TOGGLE_FLIGHT_DIRECTOR)")},
    {event=events.ls_push, action=msfs.mfwasm.rpn_executer("(>H:A320_Neo_MFD_BTN_LS_1) (>H:A320_Neo_PFD_BTN_LS_1) and")},
    {event=events.hghpa_push, action=msfs.mfwasm.rpn_executer("(L:XMLVAR_BARO_SELECTOR_HPA_1) 0 == if{ 1 (>L:XMLVAR_BARO_SELECTOR_HPA_1) } els{ 0 (>L:XMLVAR_BARO_SELECTOR_HPA_1) }")},
}

local global_mappings = {
}
for i, def in ipairs(fcu_disp.global_mappings) do
    global_mappings[#global_mappings + 1] = def
end

--------------------------------------------------------------------------------------
-- button definitions
--------------------------------------------------------------------------------------
local rbutton_size = {width = 96, height = 66, rratio=0.1}
local cbutton_size = {width = 76, height = 76, rratio=0.5}

local buttons = {
    spdmach = {x=65, y=116, size=cbutton_size},
    loc = {x=216, y=120, size=rbutton_size},
    hdgtrk = {x=504, y=116, size=cbutton_size},
    ap1 = {x=423, y=225, size=rbutton_size},
    ap2 = {x=565, y=225, size=rbutton_size},
    athr = {x=494, y=338, size=rbutton_size},
    exped = {x=729, y=174, size=rbutton_size},
    metricalt = {x=862, y=116, size=cbutton_size},
    appr = {x=968, y=174, size=rbutton_size},
    fd = {x=56, y=330, size=rbutton_size},
    ls = {x=216, y=330, size=rbutton_size},
    hghpa = {x=952, y=322, size=cbutton_size}
}

--------------------------------------------------------------------------------------
-- create view element definition for buttons
--------------------------------------------------------------------------------------
local img_off = assets.buttons:create_partial_bitmap(0, 0, rbutton_size.width, rbutton_size.height / 2)
local img_on = assets.buttons:create_partial_bitmap(0, rbutton_size.height / 2, rbutton_size.width, rbutton_size.height / 2)

local function button_renderer(ctx, value)
    if value > 0.8 then
        ctx:draw_bitmap(img_on, 0, 0)
    else
        ctx:draw_bitmap(img_off, 0, 0)
    end
end

local view_elements={}
for i, element in ipairs(fcu_disp.view_elements) do
    view_elements[#view_elements + 1] = element
end
for key, button in pairs(buttons) do
    view_elements[#view_elements + 1] = {
        object = mapper.view_elements.operable_area{
            round_ratio = button.size.rratio,
            event_tap = events[key .. "_push"]
        },
        x = button.x, y= button.y,
        width = button.size.width, height = button.size.height,
    }
    change_event = events[key .. "_change"]
    if change_event then
        local canvas = mapper.view_elements.canvas{
            logical_width = button.size.width,
            logical_height = button.size.height,
            value = 0,
            renderer = button_renderer,
        }
        view_elements[#view_elements + 1] = {
            object = canvas,
            x = button.x, y = button.y,
            width = button.size.width, height = button.size.height,
        }
        global_mappings[#global_mappings + 1] = {
            event = change_event,
            action = canvas:value_setter()
        }
    end
end

--------------------------------------------------------------------------------------
-- create view element definition for baro display
--------------------------------------------------------------------------------------
local mode_bitmaps = {
    assets.baro_mode_images[1],
    assets.baro_mode_images[2],
}
local baro_context = {
    inhg = 29.92,
    hpa = 1013.2,
    mode = -1,
    unit = 0,
}

local canvas_digits = mapper.view_elements.canvas{
    logical_width = 4 * assets.sseg.width,
    logical_height = assets.sseg.height,
    value = 0,
    renderer = function (ctx, value)
        ctx:set_font(assets.sseg_font)
        if baro_context.mode == 2 then
            ctx:draw_string("5td")
        elseif baro_context.mode >= 0 then
            if baro_context.unit == 0 then
                ctx:draw_number{
                    value = baro_context.inhg,
                    precision = 4,
                    fraction_precision = 2,
                    leading_zero = true,
                }
            else
                ctx:draw_number{
                    value = baro_context.hpa,
                    precision = 4,
                    fraction_precision = 0,
                    leading_zero = true,
                }
            end
        end
    end
}

local canvas_mode = mapper.view_elements.canvas{
    logical_width = 2 * assets.baro_mode.width,
    logical_height = assets.baro_mode.height,
    value = 0,
    renderer = function (ctx, value)
        local bitmap = mode_bitmaps[baro_context.mode + 1]
        if bitmap then
            ctx:draw_bitmap(bitmap)
        end
    end
}

view_elements[#view_elements + 1] = {
    object = canvas_digits,
    x = 760, y = 354,
    width = 120, height = 39.474
}
view_elements[#view_elements + 1] = {
    object = canvas_mode,
    x = 768, y = 324,
    width = assets.baro_mode.width * 2, height = assets.baro_mode.height
}

global_mappings[#global_mappings + 1] = {
    event=events.baro_inhg_change, action=function (evid, value)
        baro_context.inhg = value
        if baro_context.unit == 0 and baro_context.mode ~= 2 then
            canvas_digits:set_value(baro_context)
            canvas_mode:set_value(baro_context)
        end
    end
}
global_mappings[#global_mappings + 1] = {
    event=events.baro_hpa_change, action=function (evid, value)
        baro_context.hpa = value
        if baro_context.unit == 1 and baro_context.mode ~= 2 then
            canvas_digits:set_value(baro_context)
            canvas_mode:set_value(baro_context)
        end
    end
}
global_mappings[#global_mappings + 1] = {
    event=events.baro_mode_change, action=function (evid, value)
        baro_context.mode = value
        canvas_digits:set_value(baro_context)
        canvas_mode:set_value(baro_context)
    end
}
global_mappings[#global_mappings + 1] = {
    event=events.baro_unit_change, action=function (evid, value)
        baro_context.unit = value
        canvas_mode:set_value(baro_context)
        canvas_digits:set_value(baro_context)
    end
}

--------------------------------------------------------------------------------------
-- view definition generator
--------------------------------------------------------------------------------------
local function create_view_def(name, main_window)
    local bgimage = graphics.bitmap(assets.fcu.width, assets.fcu.height)
    local ctx = graphics.rendering_context(bgimage)
    fcu_disp.render_base_image(ctx)
    ctx:draw_bitmap(assets.fcu)
    ctx:finish_rendering()

    local elements = {}
    for i, element in ipairs(view_elements) do
        elements[#elements + 1] = element
    end
    elements[#elements + 1] = {
        object=main_window, x= 0, y=view_height - view_width , width=view_width, height=view_width + 2
    }

    return {
        name = name,
        logical_width = view_width,
        logical_height = view_height,
        background = bgimage,
        elements = elements,
        mappings = view_mappings,
    }
end

mod_context.viewdef = create_view_def
mod_context.observed_data = observed_data
mod_context.mappings = global_mappings

return mod_context