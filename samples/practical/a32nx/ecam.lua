local view_width = 1084
local view_height = 1541

--------------------------------------------------------------------------------------
-- register events
--------------------------------------------------------------------------------------
local events = {
    toconfig_push = mapper.register_event("ECAM:T.O_CONFIG:push"),
    eng_push = mapper.register_event("ECAM:END:push"),
    bleed_push = mapper.register_event("ECAM:BLEED:push"),
    press_push = mapper.register_event("ECAM:PRESS:push"),
    elec_push = mapper.register_event("ECAM:ELEC:push"),
    hyd_push = mapper.register_event("ECAM:HYD:push"),
    fuel_push = mapper.register_event("ECAM:FUEL:push"),
    apu_push = mapper.register_event("ECAM:APU:push"),
    cond_push = mapper.register_event("ECAM:COND:push"),
    door_push = mapper.register_event("ECAM:DOOR:push"),
    wheel_push = mapper.register_event("ECAM:WHEEL:push"),
    fctl_push = mapper.register_event("ECAM:F_CTL:push"),
    all_push = mapper.register_event("ECAM:ALL:push"),
    lclr_push = mapper.register_event("ECAM:L-CLR:push"),
    lclr_change = mapper.register_event("ECAM:L-CLR:change"),
    sts_push = mapper.register_event("ECAM:STS:push"),
    rcl_push = mapper.register_event("ECAM:RCL:push"),
    rclr_push = mapper.register_event("ECAM:R_CLR:push"),
    rclr_change = mapper.register_event("ECAM:R_CLR:change"),

    page_change = mapper.register_event("ECAM:current page changed"),
}

--------------------------------------------------------------------------------------
-- observed data definitions
--------------------------------------------------------------------------------------
local observed_data = {
    {rpn="()", event=events.lclr_change},
    {rpn="()", event=events.rclr_change},
    {rpn="(L:A32NX_ECAM_SD_CURRENT_PAGE_INDEX)", event=events.page_change},
}

--------------------------------------------------------------------------------------
-- create bitmaps of buttons 
--------------------------------------------------------------------------------------
local assets = require("a32nx/assets")
local button_width = 96
local button_height = 66
local function button_bitmap(ix)
    return assets.buttons:create_partial_bitmap(button_width * ix, 0, button_width, button_height)
end
btn_imgs = {
    off = assets.buttons:create_partial_bitmap(0, 0, button_width, button_height / 2),
    on = assets.buttons:create_partial_bitmap(0, button_height / 2, button_width, button_height / 2),
    toconfig = button_bitmap(1),
    eng = button_bitmap(2),
    bleed = button_bitmap(3),
    press = button_bitmap(4),
    elec = button_bitmap(5),
    hyd = button_bitmap(6),
    fuel = button_bitmap(7),
    apu = button_bitmap(8),
    cond = button_bitmap(9),
    door = button_bitmap(10),
    wheel = button_bitmap(11),
    fctl = button_bitmap(12),
    all = button_bitmap(13),
    lclr = button_bitmap(14),
    rclr = button_bitmap(14),
    sts = button_bitmap(15),
    rcl = button_bitmap(16),
}

--------------------------------------------------------------------------------------
-- button definitions
--------------------------------------------------------------------------------------
local buttons = {
    eng =   {x=0, y=0, led=true, page=0},
    bleed = {x=1, y=0, led=true, page=1},
    press = {x=2, y=0, led=true, page=2},
    elec =  {x=3, y=0, led=true, page=3},
    hyd =   {x=4, y=0, led=true, page=4},
    fuel =  {x=5, y=0, led=true, page=5},
    apu =   {x=0, y=1, led=true, page=6},
    cond =  {x=1, y=1, led=true, page=7},
    door =  {x=2, y=1, led=true, page=8},
    wheel = {x=3, y=1, led=true, page=9},
    fctl =  {x=4, y=1, led=true, page=10},
    all =   {x=5, y=1},
    lclr =  {x=0, y=2, led=true},
    sts =   {x=2, y=2, led=true, page=11},
    rcl =   {x=3, y=2},
    rclr =  {x=5, y=2, led=true},
}

local grid_h_num = 6
local grid_v_num = 3
local grid_width = view_width - 200
local grid_height = view_height - view_width - 210
local grid_bottom_gap = 40
local grid_x = (view_width - grid_width) / 2
local grid_y = view_height - grid_height - grid_bottom_gap - view_width
local grid_h_space = (grid_width - button_width) / (grid_h_num - 1)
local grid_v_space = (grid_height - button_height) / (grid_v_num - 1)

for key, button in pairs(buttons) do
    buttons[key].x = buttons[key].x * grid_h_space + grid_x
    buttons[key].y = buttons[key].y * grid_v_space + grid_y
end

buttons["toconfig"] = {x=buttons.bleed.x, y=grid_bottom_gap}

--------------------------------------------------------------------------------------
-- create background image
--------------------------------------------------------------------------------------
local bg_image = graphics.bitmap(view_width, view_height)
local ctx = graphics.rendering_context(bg_image)
ctx:set_brush(graphics.color(80, 105, 123))
ctx:fill_rectangle(0, 0, view_width, view_height - view_width)
ctx:set_brush(graphics.color(40, 52, 61))
ctx:fill_rectangle(0, 0, 1.5, view_height)
ctx:fill_rectangle(view_width - 1.5, 0, 1.5, view_height)

for key, button in pairs(buttons) do
    ctx:draw_bitmap(btn_imgs[key], button.x, button.y)
end

ctx:finish_rendering()

--------------------------------------------------------------------------------------
-- reflecting button status to view image
--------------------------------------------------------------------------------------
local button_status_elements = {} -- will be build later
local current_page = -1
local function reflect_page_change(eventid, page)
    local current = button_status_elements[current_page]
    if current then
        current:set_value(0)
    end
    current_page = page
    current = button_status_elements[current_page]
    if current then
        current:set_value(1)
    end
end

--------------------------------------------------------------------------------------
-- event-action mappings
--------------------------------------------------------------------------------------
local function change_ecam_page(pgid)
    local rpn = nil
    if current_page ~= pgid then
        rpn = pgid .. " (>L:A32NX_ECAM_SD_CURRENT_PAGE_INDEX) (>H:A32NX_SD_PAGE_CHANGED)"
    else
        rpn = "-1 (>L:A32NX_ECAM_SD_CURRENT_PAGE_INDEX) (>H:A32NX_SD_PAGE_CHANGED)"
    end
    msfs.mfwasm.execute_rpn(rpn)
end

local view_mappings = {
    {event=events.toconfig_push, action=filter.duplicator(
        msfs.mfwasm.rpn_executer("1 (>L:A32NX_BTN_TOCONFIG)"),
        filter.delay(200, msfs.mfwasm.rpn_executer("0 (>L:A32NX_BTN_TOCONFIG)"))
    )},
    {event=events.eng_push, action=function () change_ecam_page(0) end},
    {event=events.bleed_push, action=function () change_ecam_page(1) end},
    {event=events.press_push, action=function () change_ecam_page(2) end},
    {event=events.elec_push, action=function () change_ecam_page(3) end},
    {event=events.hyd_push, action=function () change_ecam_page(4) end},
    {event=events.fuel_push, action=function () change_ecam_page(5) end},
    {event=events.apu_push, action=function () change_ecam_page(6) end},
    {event=events.cond_push, action=function () change_ecam_page(7) end},
    {event=events.door_push, action=function () change_ecam_page(8) end},
    {event=events.wheel_push, action=function () change_ecam_page(9) end},
    {event=events.fctl_push, action=function () change_ecam_page(10) end},
    {event=events.sts_push, action=function () change_ecam_page(11) end},

    -- need to consider which mapping method is reasonable regarding following events
    -- {event=events.all_push, action=},
    -- {event=events.rcl_push, action=},
    -- {event=events.lclr_push, action=},
    -- {event=events.rclr_push, action=},
}

local global_mappings = {
    {event=events.page_change, action=reflect_page_change},
}

--------------------------------------------------------------------------------------
-- view element definition
--------------------------------------------------------------------------------------
local function button_renderer(ctx, value)
    if value > 0.8 then
        ctx:draw_bitmap(btn_imgs.on, 0, 0)
    else
        ctx:draw_bitmap(btn_imgs.off, 0, 0)
    end
end

local view_elements = {}
for key, button in pairs(buttons) do
    view_elements[#view_elements + 1] = {
        object = mapper.view_elements.operable_area{
            round_ratio = 0.1,
            event_tap = events[key .. "_push"],
        },
        x = button.x, y = button.y,
        width = button_width, height = button_height,
    }
    if button.led then
        local canvas = mapper.view_elements.canvas{
            logical_width = button_width,
            logical_height = button_height,
            value = 0,
            renderer = button_renderer,
        }
        view_elements[#view_elements + 1] = {
            object = canvas,
            x = button.x, y = button.y,
            width = button_width, height = button_height,
        }
        local change_event = events[key .. "_change"]
        if change_event then
            global_mappings[#global_mappings + 1] = {
                event = change_event,
                action = canvas:value_setter()
            }
        else
            button_status_elements[button.page] = canvas;
        end
    end
end

--------------------------------------------------------------------------------------
-- view definition generator
--------------------------------------------------------------------------------------
local function create_view_def(name, window)
    local elements = {}
    for i, element in ipairs(view_elements) do
        elements[#elements + 1] = element
    end
    elements[#elements + 1] = {
        object=window, x= 0, y=view_height - view_width, width=view_width, height=view_width + 2
    }

    return {
        name = name,
        logical_width = view_width, logical_height = view_height,
        background = bg_image,
        elements = elements,
        mappings = view_mappings,
    }
end

return {viewdef=create_view_def, observed_data=observed_data, mappings=global_mappings}