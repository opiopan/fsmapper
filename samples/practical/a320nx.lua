local config = {
    -- debug = true,
    simhid_g1000_identifier = {path = "COM3"},
    simhid_g1000_display = 2,
}

local height_menu = 58
local height_display = 768
local height_panel = 326
local height_total = height_menu + height_display + height_panel

local vratio_menu = height_menu / height_total
local vratio_display = height_display / height_total    
local vratio_panel = height_panel / height_total

a320_context = {}

--------------------------------------------------------------------------------------
-- Create viewports
--------------------------------------------------------------------------------------
local display = config.simhid_g1000_display
local scale = 1
if config.debug then
    display = 1
    scale = 0.5
end

local viewport_left = mapper.viewport{
    name = "A320 left Viewport",
    displayno = display,
    x = 0, y = 0,
    width = 0.5 * scale, height = (vratio_display + vratio_panel) * scale,
    aspect_ratio = 2 / 3 / (vratio_panel + vratio_display),
    horizontal_alignment = "right",
    vertical_alignment = "bottom",
}

local viewport_right = mapper.viewport{
    name = "A320 right Viewport",
    displayno = display,
    x = 0.5 * scale, y = 0,
    width = 0.5 * scale, height = (vratio_display + vratio_panel) * scale,
    aspect_ratio = 2 / 3 / (vratio_panel + vratio_display),
    horizontal_alignment = "left",
    vertical_alignment = "bottom",
}

local viewport_menu = mapper.viewport{
    name = "A320 menu Viewport",
    displayno = display,
    x = 0, y = (vratio_display + vratio_panel) * scale,
    width = scale, height = vratio_menu * scale,
    aspect_ratio = 4 / 3 / vratio_menu,
    vertical_alignment = "top",
}

--------------------------------------------------------------------------------------
-- Register views to right & left viewports
--------------------------------------------------------------------------------------
local captured_window = {
    fcu = mapper.view_elements.captured_window{name="A320 FCU"},
    pfd = mapper.view_elements.captured_window{name="A320 PFD"},
    nd = mapper.view_elements.captured_window{name="A320 ND"},
    uecam = mapper.view_elements.captured_window{name="A320 Upper ECAM"},
    lecam = mapper.view_elements.captured_window{name="A320 Lower ECAM"},
    mcdu = mapper.view_elements.captured_window{name = "A320 MCDU"},
}

local function captured_window_view(name, window)
    return {
        name = name,
        elements = {{object = window}},
    }
end

local global_mappings = {}

local fcu_panel = require("a320nx/fcu")
fs2020.mfwasm.add_observed_data(fcu_panel.observed_data)
global_mappings[#global_mappings + 1] = fcu_panel.mappings
local ecam_panel = require("a320nx/ecam")
fs2020.mfwasm.add_observed_data(ecam_panel.observed_data)
global_mappings[#global_mappings + 1] = ecam_panel.mappings
local efis_panel = require("a320nx/efis")
fs2020.mfwasm.add_observed_data(efis_panel.observed_data)
global_mappings[#global_mappings + 1] = efis_panel.mappings
local engine_panel = require("a320nx/engine")
fs2020.mfwasm.add_observed_data(engine_panel.observed_data)
global_mappings[#global_mappings + 1] = engine_panel.mappings
local mcdu_panel = require("a320nx/cdu")

local viewdef_left_pfd = fcu_panel.viewdef("l-pfd", captured_window.fcu, captured_window.pfd)
local viewdef_left_nd = efis_panel.viewdef("l-nd", captured_window.nd)
local viewdef_left_uecam = engine_panel.viewdef("l-uecam", captured_window.uecam)
local viewdef_left_lecam = ecam_panel.viewdef("l-uecam", captured_window.lecam)
local viewdef_left_mcdu = mcdu_panel.viewdef("l-mcdu", captured_window.mcdu)
local viewdef_right_pfd = fcu_panel.viewdef("r-pfd", captured_window.fcu, captured_window.pfd)
local viewdef_right_nd = efis_panel.viewdef("r-nd", captured_window.nd)
local viewdef_right_uecam = engine_panel.viewdef("r-uecam", captured_window.uecam)
local viewdef_right_lecam = ecam_panel.viewdef("r-lecam", captured_window.lecam)
local viewdef_right_mcdu = mcdu_panel.viewdef("r-mcdu", captured_window.mcdu)

local view_left_pfd = viewport_left:register_view(viewdef_left_pfd)
local view_left_nd = viewport_left:register_view(viewdef_left_nd)
local view_left_uecam = viewport_left:register_view(viewdef_left_uecam)
local view_left_lecam = viewport_left:register_view(viewdef_left_lecam)
local view_left_mcdu = viewport_left:register_view(viewdef_left_mcdu)
local view_right_nd = viewport_right:register_view(viewdef_right_nd)
local view_right_pfd = viewport_right:register_view(viewdef_right_pfd)
local view_right_uecam = viewport_right:register_view(viewdef_right_uecam)
local view_right_lecam = viewport_right:register_view(viewdef_right_lecam)
local view_right_mcdu = viewport_right:register_view(viewdef_right_mcdu)

--------------------------------------------------------------------------------------
-- Register menu view
--------------------------------------------------------------------------------------
local assets = require("a320nx/assets")
local img_menu = assets.menu
local img_width = 128
local img_height = 58
local function make_label_image(x, y)
    return img_menu:create_partial_bitmap(x * img_width, y * img_height, img_width, img_height)
end
local img_pfd_selected = make_label_image(0, 0)
local img_pfd_unselected = make_label_image(0, 1)
local img_pfd_disabled = make_label_image(0, 2)
local img_nd_selected = make_label_image(1, 0)
local img_nd_unselected = make_label_image(1, 1)
local img_nd_disabled = make_label_image(1, 2)
local img_uecam_selected = make_label_image(2, 0)
local img_uecam_unselected = make_label_image(2, 1)
local img_uecam_disabled = make_label_image(2, 2)
local img_lecam_selected = make_label_image(3, 0)
local img_lecam_unselected = make_label_image(3, 1)
local img_lecam_disabled = make_label_image(3, 2)
local img_mcdu_selected = make_label_image(4, 0)
local img_mcdu_unselected = make_label_image(4, 1)
local img_mcdu_disabled = make_label_image(4, 2)
local img_blank = make_label_image(5, 0)

local rule_left = {
    pfd = {
        pos = 0, 
        view = view_left_pfd,
        selected = img_pfd_selected,
        unselected = img_pfd_unselected,
        disabled = img_pfd_disabled,
        next = "nd", 
        prev = "mcdu",
        mutex = {pfd = true},
    },
    nd = {
        pos = 1, 
        view = view_left_nd,
        selected = img_nd_selected,
        unselected = img_nd_unselected,
        disabled = img_nd_disabled,
        next = "uecam", 
        prev = "pfd",
        mutex = {nd = true},
    },
    uecam = {
        pos = 2, 
        view = view_left_uecam,
        selected = img_uecam_selected,
        unselected = img_uecam_unselected,
        disabled = img_uecam_disabled,
        next = "lecam", 
        prev = "nd",
        mutex = {uecam = true},
    },
    lecam = {
        pos = 3, 
        view = view_left_lecam,
        selected = img_lecam_selected,
        unselected = img_lecam_unselected,
        disabled = img_lecam_disabled,
        next = "mcdu",
        prev = "uecam",
        mutex = {lecam = true},
    },
    mcdu = {
        pos = 4, 
        view = view_left_mcdu,
        selected = img_mcdu_selected,
        unselected = img_mcdu_unselected,
        disabled = img_mcdu_disabled,
        next = "pfd",
        prev = "lecam",
        mutex = {mcdu = true},
    },
}

local rule_right = {
    pfd = {
        pos = 7, 
        view = view_right_pfd,
        selected = img_pfd_selected,
        unselected = img_pfd_unselected,
        disabled = img_pfd_disabled,
        next = "nd", 
        prev = "mcdu",
        mutex = {pfd = true},
    },
    nd = {
        pos = 8, 
        view = view_right_nd,
        selected = img_nd_selected,
        unselected = img_nd_unselected,
        disabled = img_nd_disabled,
        next = "uecam", 
        prev = "pfd",
        mutex = {nd = true},
    },
    uecam = {
        pos = 9, 
        view = view_right_uecam,
        selected = img_uecam_selected,
        unselected = img_uecam_unselected,
        disabled = img_uecam_disabled,
        next = "lecam", 
        prev = "nd",
        mutex = {uecam = true},
    },
    lecam = {
        pos = 10, 
        view = view_right_lecam,
        selected = img_lecam_selected,
        unselected = img_lecam_unselected,
        disabled = img_lecam_disabled,
        next = "mcdu",
        prev = "uecam",
        mutex = {lecam = true},
    },
    mcdu = {
        pos = 11,
        view = view_right_mcdu,
        selected = img_mcdu_selected,
        unselected = img_mcdu_unselected,
        disabled = img_mcdu_disabled,
        next = "pfd",
        prev = "lecam",
        mutex = {mcdu = true},
    },
}


local menu_context = {
    left = {current = "pfd", rule = rule_left, viewport = viewport_left},
    right = {current = "nd", rule = rule_right, viewport = viewport_right},
}

local typical_views = {
    {left="pfd", right="nd"},
    {left="uecam", right="lecam"},
    {left="nd", right="mcdu"},
}

local function make_renderer_value()
    value = {}
    for k, v in pairs(menu_context.left.rule) do
        if k == menu_context.left.current then
            value[v.pos] = v.selected
        elseif v.mutex[menu_context.right.current] then
            value[v.pos] = v.disabled
        else
            value[v.pos] = v.unselected
        end
    end
    for k, v in pairs(menu_context.right.rule) do
        if k == menu_context.right.current then
            value[v.pos] = v.selected
        elseif v.mutex[menu_context.left.current] then
            value[v.pos] = v.disabled
        else
            value[v.pos] = v.unselected
        end
    end
    return value
end

local menu_bar = mapper.view_elements.canvas{
    logical_width = 1536,
    logical_height = 58,
    value = make_renderer_value(),
    renderer = function (ctx, value)
        for i = 0, 11 do
            local img = value[i]
            if img then
                ctx:draw_bitmap(img, img_width * i, 0)
            else
                ctx:draw_bitmap(img_blank, img_width * i, 0)
            end
        end
    end,
}

local function change_view(target, view_name, opposite)
    target_ctx = menu_context[target]
    opposite_ctx = menu_context[opposite]
    rule = target_ctx.rule[view_name]
    if rule.mutex[opposite_ctx.current] then
        return false
    elseif target_ctx.current ~= view_name then
        target_ctx.current = view_name
        target_ctx.viewport:change_view(rule.view)
        menu_bar:set_value(make_renderer_value())
        return true
    else
        return true
    end
end

local function change_typical_view(direction)
    local left = menu_context.left.current
    local right = menu_context.right.current
    local ix = nil
    for i, set in ipairs(typical_views) do
        if left == set.left and right == set.right then
            ix = i
            break
        end
    end
    if ix then
        if direction > 0 then
            ix = ix + 1
            if ix > #typical_views then
                ix = 1
            end
        else
            ix = ix - 1
            if ix < 1 then
                ix = #typical_views
            end
        end
    else
        if direction > 0 then
            ix = 1
        else
            ix = #typical_views
        end
    end
    local set = typical_views[ix]
    menu_context.left.current = set.left
    menu_context.right.current = set.right
    if direction > 0 then
        menu_context.left.viewport:change_view(menu_context.left.rule[set.left].view)
        menu_context.right.viewport:change_view(menu_context.right.rule[set.right].view)
    else
        menu_context.right.viewport:change_view(menu_context.right.rule[set.right].view)
        menu_context.left.viewport:change_view(menu_context.left.rule[set.left].view)
    end

    menu_bar:set_value(make_renderer_value())
end

a320_context.device = mapper.device{
    name = "SimHID G1000",
    type = "simhid",
    identifier = config.simhid_g1000_identifier,
    modifiers = {
        {class = "binary", modtype = "button"},
        {class = "relative", modtype = "incdec"},
    },
}
local g1000 = a320_context.device.events

local mappings = {
    {event=g1000.SW14.down, action=function () change_view("left", "pfd", "right") end},
    {event=g1000.SW15.down, action=function () change_view("left", "nd", "right") end},
    {event=g1000.SW16.down, action=function () change_view("left", "uecam", "right") end},
    {event=g1000.SW17.down, action=function () change_view("left", "lecam", "right") end},
    {event=g1000.SW18.down, action=function () change_view("left", "mcdu", "right") end},
    {event=g1000.SW21.down, action=function () change_view("right", "pfd", "left") end},
    {event=g1000.SW22.down, action=function () change_view("right", "nd", "left") end},
    {event=g1000.SW23.down, action=function () change_view("right", "uecam", "left") end},
    {event=g1000.SW24.down, action=function () change_view("right", "lecam", "left") end},
    {event=g1000.SW25.down, action=function () change_view("right", "mcdu", "left") end},

    {event=g1000.EC1.increment, action=fs2020.event_sender("MobiFlight.SPD_Increase")},
    {event=g1000.EC1.decrement, action=fs2020.event_sender("MobiFlight.SPD_Decrease")},
    {event=g1000.EC1P.down, action=fs2020.event_sender("MobiFlight.SPD_Push")},
    {event=g1000.SW1.down, action=fs2020.event_sender("MobiFlight.SPD_Pull")},
    {event=g1000.EC3.increment, action=fs2020.event_sender("MobiFlight.HDG_Increase")},
    {event=g1000.EC3.decrement, action=fs2020.event_sender("MobiFlight.HDG_Decrease")},
    {event=g1000.EC3P.down, action=fs2020.event_sender("MobiFlight.HDG_Push")},
    {event=g1000.SW4.down, action=fs2020.event_sender("MobiFlight.HDG_Pull")},
    {event=g1000.EC4X.increment, action=fs2020.mfwasm.rpn_executer("100 (>K:A32NX.FCU_ALT_INC)")},
    {event=g1000.EC4X.decrement, action=fs2020.mfwasm.rpn_executer("100 (>K:A32NX.FCU_ALT_DEC)")},
    {event=g1000.EC4Y.increment, action=fs2020.mfwasm.rpn_executer("1000 (>K:A32NX.FCU_ALT_INC)")},
    {event=g1000.EC4Y.decrement, action=fs2020.mfwasm.rpn_executer("1000 (>K:A32NX.FCU_ALT_DEC)")},
    {event=g1000.EC4P.down, action=fs2020.event_sender("MobiFlight.A32NX_FCU_ALT_PUSH")},
    {event=g1000.SW12.down, action=fs2020.event_sender("MobiFlight.A32NX_FCU_ALT_PULL")},
    {event=g1000.EC5.increment, action=fs2020.event_sender("MobiFlight.A32NX_FCU_VS_INC")},
    {event=g1000.EC5.decrement, action=fs2020.event_sender("MobiFlight.A32NX_FCU_VS_DEC")},
    {event=g1000.EC5P.down, action=fs2020.event_sender("MobiFlight.A32NX_FCU_VS_PUSH")},
    {event=g1000.SW26.down, action=fs2020.event_sender("MobiFlight.A32NX_FCU_VS_PULL")},
    {event=g1000.SW2.down, action=fs2020.event_sender("MobiFlight.Autopilot_1_Push")},
    {event=g1000.SW3.down, action=fs2020.event_sender("MobiFlight.A32NX_EFIS_FD_PUSH")},
    {event=g1000.SW8.down, action=fs2020.mfwasm.rpn_executer("(>K:A32NX.FCU_APPR_PUSH)")},

    {event=g1000.EC8.increment, action=fs2020.event_sender("MobiFlight.A32NX_EFIS_L_ND_RANGE_INC")},
    {event=g1000.EC8.decrement, action=fs2020.event_sender("MobiFlight.A32NX_EFIS_L_ND_RANGE_DEC")},
    {event=g1000.EC8L.down, action=fs2020.event_sender("MobiFlight.A32NX_EFIS_L_ND_MODE_DEC")},
    {event=g1000.EC8U.down, action=fs2020.event_sender("MobiFlight.A32NX_EFIS_L_ND_MODE_DEC")},
    {event=g1000.EC8R.down, action=fs2020.event_sender("MobiFlight.A32NX_EFIS_L_ND_MODE_INC")},
    {event=g1000.EC8D.down, action=fs2020.event_sender("MobiFlight.A32NX_EFIS_L_ND_MODE_INC")},

    {event=g1000.EC6X.increment, action=fs2020.event_sender("MobiFlight.A32NX_RMP_L_INNER_KNOB_TURNED_CLOCKWISE")},
    {event=g1000.EC6X.decrement, action=fs2020.event_sender("MobiFlight.A32NX_RMP_L_INNER_KNOB_TURNED_ANTICLOCKWISE")},
    {event=g1000.EC6Y.increment, action=fs2020.event_sender("MobiFlight.A32NX_RMP_L_OUTER_KNOB_TURNED_CLOCKWISE")},
    {event=g1000.EC6Y.decrement, action=fs2020.event_sender("MobiFlight.A32NX_RMP_L_OUTER_KNOB_TURNED_ANTICLOCKWISE")},
    {event=g1000.EC6P.down, action=fs2020.event_sender("MobiFlight.A32NX_RMP_L_TRANSFER_BUTTON_PRESSED")},

    {event=g1000.EC7Y.increment, action=fs2020.event_sender("Mobiflight.Baro_increase")},
    {event=g1000.EC7Y.decrement, action=fs2020.event_sender("Mobiflight.Baro_decrease")},
    {event=g1000.EC7P.down, action=fs2020.mfwasm.rpn_executer("(L:XMLVAR_Baro1_Mode) 2 == if{ 1 (>L:XMLVAR_Baro1_Mode) } els{ 2 (>L:XMLVAR_Baro1_Mode) }")},

    {event=g1000.AUX1D.down, action=function() change_typical_view(1) end},
    {event=g1000.AUX1U.down, action=function() change_typical_view(-1) end},
    {event=g1000.AUX2D.down, action=function() change_typical_view(1) end},
    {event=g1000.AUX2U.down, action=function() change_typical_view(-1) end},
}

viewport_menu:register_view{
    name = "menu",
    background = "black",
    elements = {
        {object = menu_bar},
    },
    mappings = mappings,
}

for i, mappings in ipairs(global_mappings) do
    mapper.add_primary_mappings(mappings)
end

mapper.start_viewports()
