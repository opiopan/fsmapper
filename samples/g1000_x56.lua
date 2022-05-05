g1000_dev = mapper.device({
    name = "SimHID G1000",
    type = "simhid",
    identifier = {path = "COM3"},
    modifiers = {
        {class = "binary", modtype = "button"},
        {class = "relative", modtype = "incdec"},
        {name = "SW26", modtype = "button", modparam={longpress = 2000}},
        {name = "SW31", modtype = "button", modparam={longpress = 2000}},
        {name = "EC8U", modtype = "button", modparam={repeat_interval = 80}},
        {name = "EC8D", modtype = "button", modparam={repeat_interval = 80}},
        {name = "EC8R", modtype = "button", modparam={repeat_interval = 80}},
        {name = "EC8L", modtype = "button", modparam={repeat_interval = 80}},
    },
})
local g1000 = g1000_dev.events

local pfd_maps = {
    {event=g1000.EC1.increment, action=fs2020.event_sender("Mobiflight.AS1000_PFD_VOL_1_INC")},
    {event=g1000.EC1.decrement, action=fs2020.event_sender("Mobiflight.AS1000_PFD_VOL_1_DEC")},
    {event=g1000.EC2X.increment, action=fs2020.event_sender("Mobiflight.AS1000_PFD_NAV_Small_INC")},
    {event=g1000.EC2X.decrement, action=fs2020.event_sender("Mobiflight.AS1000_PFD_NAV_Small_DEC")},
    {event=g1000.EC2Y.increment, action=fs2020.event_sender("Mobiflight.AS1000_PFD_NAV_Large_INC")},
    {event=g1000.EC2Y.decrement, action=fs2020.event_sender("Mobiflight.AS1000_PFD_NAV_Large_DEC")},
    {event=g1000.EC2P.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_NAV_Push")},
    {event=g1000.EC3.increment, action=fs2020.event_sender("Mobiflight.AS1000_PFD_HEADING_INC")},
    {event=g1000.EC3.decrement, action=fs2020.event_sender("Mobiflight.AS1000_PFD_HEADING_DEC")},
    {event=g1000.EC3P.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_HEADING_SYNC")},
    {event=g1000.EC4X.increment, action=fs2020.event_sender("Mobiflight.AS1000_AP_ALT_INC_100")},
    {event=g1000.EC4X.decrement, action=fs2020.event_sender("Mobiflight.AS1000_AP_ALT_DEC_100")},
    {event=g1000.EC4Y.increment, action=fs2020.event_sender("Mobiflight.AS1000_AP_ALT_INC_1000")},
    {event=g1000.EC4Y.decrement, action=fs2020.event_sender("Mobiflight.AS1000_AP_ALT_DEC_1000")},
    {event=g1000.SW1.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_NAV_Switch")},
    {event=g1000.SW2.down, action=fs2020.event_sender("Mobiflight.AP_MASTER")},
    {event=g1000.SW3.down, action=fs2020.event_sender("Mobiflight.TOGGLE_FLIGHT_DIRECTOR")},
    {event=g1000.SW4.down, action=fs2020.event_sender("Mobiflight.AP_HDG_HOLD")},
    {event=g1000.SW5.down, action=fs2020.event_sender("Mobiflight.AP_ALT_HOLD")},
    {event=g1000.SW6.down, action=fs2020.event_sender("Mobiflight.AP_NAV1_HOLD")},
    {event=g1000.SW7.down, action=fs2020.event_sender("Mobiflight.AS1000_AP_VNAV_Push")},
    {event=g1000.SW8.down, action=fs2020.event_sender("Mobiflight.AP_APR_HOLD")},
    {event=g1000.SW9.down, action=fs2020.event_sender("Mobiflight.AP_BC_HOLD")},
    {event=g1000.SW10.down, action=fs2020.event_sender("Mobiflight.AP_PANEL_VS_HOLD")},
    {event=g1000.SW11.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_NOSE_UP")},
    {event=g1000.SW12.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_FLC_Push")},
    {event=g1000.SW13.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_NOSE_DN")},

    {event=g1000.SW14.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_SOFTKEYS_1")},
    {event=g1000.SW15.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_SOFTKEYS_2")},
    {event=g1000.SW16.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_SOFTKEYS_3")},
    {event=g1000.SW17.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_SOFTKEYS_4")},
    {event=g1000.SW18.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_SOFTKEYS_5")},
    {event=g1000.SW19.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_SOFTKEYS_6")},
    {event=g1000.SW20.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_SOFTKEYS_7")},
    {event=g1000.SW21.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_SOFTKEYS_8")},
    {event=g1000.SW22.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_SOFTKEYS_9")},
    {event=g1000.SW23.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_SOFTKEYS_10")},
    {event=g1000.SW24.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_SOFTKEYS_11")},
    {event=g1000.SW25.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_SOFTKEYS_12")},

    {event=g1000.EC5.increment, action=fs2020.event_sender("Mobiflight.AS1000_PFD_VOL_2_INC")},
    {event=g1000.EC5.decrement, action=fs2020.event_sender("Mobiflight.AS1000_PFD_VOL_2_DEC")},
    {event=g1000.EC6X.increment, action=fs2020.event_sender("Mobiflight.AS1000_PFD_COM_Small_INC")},
    {event=g1000.EC6X.decrement, action=fs2020.event_sender("Mobiflight.AS1000_PFD_COM_Small_DEC")},
    {event=g1000.EC6Y.increment, action=fs2020.event_sender("Mobiflight.AS1000_PFD_COM_Large_INC")},
    {event=g1000.EC6Y.decrement, action=fs2020.event_sender("Mobiflight.AS1000_PFD_COM_Large_DEC")},
    {event=g1000.EC6P.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_COM_Push")},
    {event=g1000.EC7X.increment, action=fs2020.event_sender("Mobiflight.AS1000_PFD_CRS_INC")},
    {event=g1000.EC7X.decrement, action=fs2020.event_sender("Mobiflight.AS1000_PFD_CRS_DEC")},
    {event=g1000.EC7Y.increment, action=fs2020.event_sender("Mobiflight.AS1000_PFD_BARO_INC")},
    {event=g1000.EC7Y.decrement, action=fs2020.event_sender("Mobiflight.AS1000_PFD_BARO_DEC")},
    {event=g1000.EC7P.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_CRS_PUSH")},
    {event=g1000.EC8.increment, action=fs2020.event_sender("Mobiflight.AS1000_PFD_RANGE_INC")},
    {event=g1000.EC8.decrement, action=fs2020.event_sender("Mobiflight.AS1000_PFD_RANGE_DEC")},
    {event=g1000.EC8P.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_JOYSTICK_PUSH")},
    {event=g1000.EC8U.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_JOYSTICK_UP")},
    {event=g1000.EC8D.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_JOYSTICK_DOWN")},
    {event=g1000.EC8R.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_JOYSTICK_RIGHT")},
    {event=g1000.EC8L.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_JOYSTICK_LEFT")},
    {event=g1000.EC9X.increment, action=fs2020.event_sender("Mobiflight.AS1000_PFD_FMS_Upper_INC")},
    {event=g1000.EC9X.decrement, action=fs2020.event_sender("Mobiflight.AS1000_PFD_FMS_Upper_DEC")},
    {event=g1000.EC9Y.increment, action=fs2020.event_sender("Mobiflight.AS1000_PFD_FMS_Lower_INC")},
    {event=g1000.EC9Y.decrement, action=fs2020.event_sender("Mobiflight.AS1000_PFD_FMS_Lower_DEC")},
    {event=g1000.EC9P.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_FMS_Upper_PUSH")},

    {event=g1000.SW26.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_COM_Switch")},
    {event=g1000.SW26.longpressed, action=fs2020.event_sender("Mobiflight.AS1000_PFD_COM_Switch_Long")},
    {event=g1000.SW27.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_DIRECTTO")},
    {event=g1000.SW28.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_MENU_Push")},
    {event=g1000.SW29.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_FPL_Push")},
    {event=g1000.SW30.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_PROC_Push")},
    {event=g1000.SW31.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_CLR")},
    {event=g1000.SW31.longpressed, action=fs2020.event_sender("Mobiflight.AS1000_PFD_CLR_Long")},
    {event=g1000.SW32.down, action=fs2020.event_sender("Mobiflight.AS1000_PFD_ENT_Push")},
}

local mfd_maps = {
    {event=g1000.EC1.increment, action=fs2020.event_sender("Mobiflight.AS1000_MFD_VOL_1_INC")},
    {event=g1000.EC1.decrement, action=fs2020.event_sender("Mobiflight.AS1000_MFD_VOL_1_DEC")},
    {event=g1000.EC2X.increment, action=fs2020.event_sender("Mobiflight.AS1000_MFD_NAV_Small_INC")},
    {event=g1000.EC2X.decrement, action=fs2020.event_sender("Mobiflight.AS1000_MFD_NAV_Small_DEC")},
    {event=g1000.EC2Y.increment, action=fs2020.event_sender("Mobiflight.AS1000_MFD_NAV_Large_INC")},
    {event=g1000.EC2Y.decrement, action=fs2020.event_sender("Mobiflight.AS1000_MFD_NAV_Large_DEC")},
    {event=g1000.EC2P.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_NAV_Push")},
    {event=g1000.EC3.increment, action=fs2020.event_sender("Mobiflight.AS1000_MFD_HEADING_INC")},
    {event=g1000.EC3.decrement, action=fs2020.event_sender("Mobiflight.AS1000_MFD_HEADING_DEC")},
    {event=g1000.EC3P.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_HEADING_SYNC")},
    {event=g1000.EC4X.increment, action=fs2020.event_sender("Mobiflight.AS1000_AP_ALT_INC_100")},
    {event=g1000.EC4X.decrement, action=fs2020.event_sender("Mobiflight.AS1000_AP_ALT_DEC_100")},
    {event=g1000.EC4Y.increment, action=fs2020.event_sender("Mobiflight.AS1000_AP_ALT_INC_1000")},
    {event=g1000.EC4Y.decrement, action=fs2020.event_sender("Mobiflight.AS1000_AP_ALT_DEC_1000")},
    {event=g1000.SW1.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_NAV_Switch")},
    {event=g1000.SW2.down, action=fs2020.event_sender("Mobiflight.AP_MASTER")},
    {event=g1000.SW3.down, action=fs2020.event_sender("Mobiflight.TOGGLE_FLIGHT_DIRECTOR")},
    {event=g1000.SW4.down, action=fs2020.event_sender("Mobiflight.AP_HDG_HOLD")},
    {event=g1000.SW5.down, action=fs2020.event_sender("Mobiflight.AP_ALT_HOLD")},
    {event=g1000.SW6.down, action=fs2020.event_sender("Mobiflight.AP_NAV1_HOLD")},
    {event=g1000.SW7.down, action=fs2020.event_sender("Mobiflight.AS1000_AP_VNAV_Push")},
    {event=g1000.SW8.down, action=fs2020.event_sender("Mobiflight.AP_APR_HOLD")},
    {event=g1000.SW9.down, action=fs2020.event_sender("Mobiflight.AP_BC_HOLD")},
    {event=g1000.SW10.down, action=fs2020.event_sender("Mobiflight.AP_PANEL_VS_HOLD")},
    {event=g1000.SW11.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_NOSE_UP")},
    {event=g1000.SW12.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_FLC_Push")},
    {event=g1000.SW13.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_NOSE_DN")},

    {event=g1000.SW14.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_SOFTKEYS_1")},
    {event=g1000.SW15.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_SOFTKEYS_2")},
    {event=g1000.SW16.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_SOFTKEYS_3")},
    {event=g1000.SW17.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_SOFTKEYS_4")},
    {event=g1000.SW18.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_SOFTKEYS_5")},
    {event=g1000.SW19.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_SOFTKEYS_6")},
    {event=g1000.SW20.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_SOFTKEYS_7")},
    {event=g1000.SW21.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_SOFTKEYS_8")},
    {event=g1000.SW22.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_SOFTKEYS_9")},
    {event=g1000.SW23.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_SOFTKEYS_10")},
    {event=g1000.SW24.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_SOFTKEYS_11")},
    {event=g1000.SW25.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_SOFTKEYS_12")},

    {event=g1000.EC5.increment, action=fs2020.event_sender("Mobiflight.AS1000_MFD_VOL_2_INC")},
    {event=g1000.EC5.decrement, action=fs2020.event_sender("Mobiflight.AS1000_MFD_VOL_2_DEC")},
    {event=g1000.EC6X.increment, action=fs2020.event_sender("Mobiflight.AS1000_MFD_COM_Small_INC")},
    {event=g1000.EC6X.decrement, action=fs2020.event_sender("Mobiflight.AS1000_MFD_COM_Small_DEC")},
    {event=g1000.EC6Y.increment, action=fs2020.event_sender("Mobiflight.AS1000_MFD_COM_Large_INC")},
    {event=g1000.EC6Y.decrement, action=fs2020.event_sender("Mobiflight.AS1000_MFD_COM_Large_DEC")},
    {event=g1000.EC6P.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_COM_Push")},
    {event=g1000.EC7X.increment, action=fs2020.event_sender("Mobiflight.AS1000_MFD_CRS_INC")},
    {event=g1000.EC7X.decrement, action=fs2020.event_sender("Mobiflight.AS1000_MFD_CRS_DEC")},
    {event=g1000.EC7Y.increment, action=fs2020.event_sender("Mobiflight.AS1000_MFD_BARO_INC")},
    {event=g1000.EC7Y.decrement, action=fs2020.event_sender("Mobiflight.AS1000_MFD_BARO_DEC")},
    {event=g1000.EC7P.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_CRS_PUSH")},
    {event=g1000.EC8.increment, action=fs2020.event_sender("Mobiflight.AS1000_MFD_RANGE_INC")},
    {event=g1000.EC8.decrement, action=fs2020.event_sender("Mobiflight.AS1000_MFD_RANGE_DEC")},
    {event=g1000.EC8P.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_JOYSTICK_PUSH")},
    {event=g1000.EC8U.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_JOYSTICK_UP")},
    {event=g1000.EC8D.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_JOYSTICK_DOWN")},
    {event=g1000.EC8R.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_JOYSTICK_RIGHT")},
    {event=g1000.EC8L.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_JOYSTICK_LEFT")},
    {event=g1000.EC9X.increment, action=fs2020.event_sender("Mobiflight.AS1000_MFD_FMS_Upper_INC")},
    {event=g1000.EC9X.decrement, action=fs2020.event_sender("Mobiflight.AS1000_MFD_FMS_Upper_DEC")},
    {event=g1000.EC9Y.increment, action=fs2020.event_sender("Mobiflight.AS1000_MFD_FMS_Lower_INC")},
    {event=g1000.EC9Y.decrement, action=fs2020.event_sender("Mobiflight.AS1000_MFD_FMS_Lower_DEC")},
    {event=g1000.EC9P.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_FMS_Upper_PUSH")},

    {event=g1000.SW26.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_COM_Switch")},
    {event=g1000.SW26.longpressed, action=fs2020.event_sender("Mobiflight.AS1000_MFD_COM_Switch_Long")},
    {event=g1000.SW27.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_DIRECTTO")},
    {event=g1000.SW28.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_MENU_Push")},
    {event=g1000.SW29.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_FPL_Push")},
    {event=g1000.SW30.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_PROC_Push")},
    {event=g1000.SW31.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_CLR")},
    {event=g1000.SW31.longpressed, action=fs2020.event_sender("Mobiflight.AS1000_MFD_CLR_Long")},
    {event=g1000.SW32.down, action=fs2020.event_sender("Mobiflight.AS1000_MFD_ENT_Push")},
}

local viewport = mapper.viewport({
    name = "G1000 Viewport",
    displayno = 2,
    bgcolor = "Black",
    aspect_ratio = 4.0 / 3.0,
})
local pfd = viewport:register_view({
    name = "PFD",
    elements = {{object = mapper.captured_window({name = "G1000 PFD"})}},
    mappings = pfd_maps,
})
local mfd = viewport:register_view({
    name = "MFD",
    elements = {{object = mapper.captured_window({name = "G1000 MFD"})}},
    mappings = mfd_maps,
})

local function toggle_view()
    if viewport.current_view == pfd then
        viewport:change_view(mfd)
    else
        viewport:change_view(pfd)
    end
end

viewport:set_mappings({
    {event=g1000.AUX1D.down, action=toggle_view},
    {event=g1000.AUX1U.down, action=toggle_view},
    {event=g1000.AUX2D.down, action=toggle_view},
    {event=g1000.AUX2U.down, action=toggle_view},
})

x56throttle_dev = mapper.device({
    name = "X-56 Throttle",
    type = "dinput",
    identifier = {name = "Saitek Pro Flight X-56 Rhino Throttle"},
    options = {denylist = {"z", "rx", "ry", "rz", "slider1", "slider2"}},
    modifiers = {
        {name = "button33", modtype = "button", modparam={follow_down = 200}},
        {name = "button34", modtype = "button"},
        {name = "button35", modtype = "button"},
        {name = "button36", modtype = "button"},
        {name = "button28", modtype = "button"},
        {name = "button29", modtype = "button"},
    },
})
x56throttle = x56throttle_dev.events

vjoy = mapper.virtual_joystick(1)
local throttle1 = vjoy:get_axis("rx")
local throttle2 = vjoy:get_axis("ry")
local throttle1a = vjoy:get_axis("x")
local throttle2a = vjoy:get_axis("y")
local airbrake_open = vjoy:get_button(1)
local airbrake_close = vjoy:get_button(2)
local ab1 = vjoy:get_button(3)
local ab2 = vjoy:get_button(4)

local marm_on = vjoy:get_button(5)
local marm_off = vjoy:get_button(6)

local eng1idle = vjoy:get_button(7)
local eng1cut = vjoy:get_button(8)
local eng2idle = vjoy:get_button(9)
local eng2cut = vjoy:get_button(10)
local eng1start = vjoy:get_button(11)
local eng2start = vjoy:get_button(12)
local canopy_open = vjoy:get_button(13)
local canopy_close = vjoy:get_button(14)
local start_aux1 = vjoy:get_button(15)
local start_aux2 = vjoy:get_button(16)

local gearup = vjoy:get_button(17)
local geardown = vjoy:get_button(18)
local flapup = vjoy:get_button(19)
local flapdown = vjoy:get_button(20)
local hookup = vjoy:get_button(21)
local hookdown = vjoy:get_button(22)
local llight_on = vjoy:get_button(23)
local llight_off = vjoy:get_button(24)
local flight_aux1 = vjoy:get_button(31)
local flight_aux2 = vjoy:get_button(32)

local aa_mode = vjoy:get_button(25)
local ag_mode = vjoy:get_button(26)
local arm_aux1 = vjoy:get_button(27)
local arm_aux2 = vjoy:get_button(28)
local arm_aux3 = vjoy:get_button(29)
local arm_aux4 = vjoy:get_button(30)

local joymap_dcs = {
    {event=x56throttle.x.change, action=filter.duplicator(
        filter.lerp(throttle1a:value_setter(),{
            {-1023, -1023},
            {-619, -617},
            {-613, -617},
            {1023, 1023},
        }),
        filter.lerp(throttle1:value_setter(),{
            {-1023, -1023},
            {-609, -1023},
            {1023, 1023},
        }),
        filter.branch(
            {condition="falled", value=-900, action=ab1:value_setter(true)},
            {condition="exceeded", value=-800, action=ab1:value_setter(false)}
        )
    )},
    {event=x56throttle.y.change, action=filter.duplicator(
        filter.lerp(throttle2a:value_setter(),{
            {-1023, -1023},
            {-619, -617},
            {-613, -617},
            {1023, 1023},
        }),
        filter.lerp(throttle2:value_setter(),{
            {-1023, -1023},
            {-619, -1023},
            {1023, 1023},
        }),
        filter.branch(
            {condition="falled", value=-900, action=ab2:value_setter(true)},
            {condition="exceeded", value=-800, action=ab2:value_setter(false)}
        )
    )},
    {event=x56throttle.button33.up, action=airbrake_open:value_setter(true)},
    {event=x56throttle.button33.down, action=filter.duplicator(
        airbrake_open:value_setter(false),
        airbrake_close:value_setter(true)
    )},
    {event=x56throttle.button33.following_down, action=airbrake_close:value_setter(false)},
}

local joymap_noab = {
    {event=x56throttle.x.change, action=filter.duplicator(
        filter.lerp(throttle1:value_setter(),{
            {-1023, -1023},
            {-609, -1023},
            {1023, 1023},
        }),
        filter.branch(
            {condition="falled", value=-900, action=ab1:value_setter(true)},
            {condition="exceeded", value=-800, action=ab1:value_setter(false)}
        )
    )},
    {event=x56throttle.y.change, action=filter.duplicator(
        filter.lerp(throttle2:value_setter(),{
            {-1023, -1023},
            {-619, -1023},
            {1023, 1023},
        }),
        filter.branch(
            {condition="falled", value=-900, action=ab2:value_setter(true)},
            {condition="exceeded", value=-800, action=ab2:value_setter(false)}
        )
    )},
    {event=x56throttle.button33.up, action=airbrake_open:value_setter(true)},
    {event=x56throttle.button33.down, action=filter.duplicator(
        airbrake_open:value_setter(false),
        airbrake_close:value_setter(true)
    )},
    {event=x56throttle.button33.following_down, action=airbrake_close:value_setter(false)},
}

local joymap_full = {
    {event=x56throttle.x.change, action=throttle1:value_setter()},
    {event=x56throttle.y.change, action=throttle2:value_setter()},
    {event=x56throttle.button33.up, action=airbrake_open:value_setter(true)},
    {event=x56throttle.button33.down, action=filter.duplicator(
        airbrake_open:value_setter(false), airbrake_close:value_setter(true)
    )},
    {event=x56throttle.button33.following_down, action=airbrake_close:value_setter(false)},
}

local joymap_preflight = {
    {event=x56throttle.button6.change, action=eng1idle:value_setter()},
    {event=x56throttle.button7.change, action=eng1cut:value_setter()},
    {event=x56throttle.button8.change, action=eng2idle:value_setter()},
    {event=x56throttle.button9.change, action=eng2cut:value_setter()},
    {event=x56throttle.button10.change, action=eng1start:value_setter()},
    {event=x56throttle.button11.change, action=eng2start:value_setter()},
    {event=x56throttle.button18.change, action=canopy_open:value_setter()},
    {event=x56throttle.button19.change, action=canopy_close:value_setter()},
    {event=x56throttle.button12.change, action=start_aux1:value_setter()},
    {event=x56throttle.button13.change, action=start_aux2:value_setter()},
}

local joymap_inflight = {
    {event=x56throttle.button6.change, action=gearup:value_setter()},
    {event=x56throttle.button7.change, action=geardown:value_setter()},
    {event=x56throttle.button8.change, action=flapup:value_setter()},
    {event=x56throttle.button9.change, action=flapdown:value_setter()},
    {event=x56throttle.button10.change, action=hookup:value_setter()},
    {event=x56throttle.button11.change, action=hookdown:value_setter()},
    {event=x56throttle.button12.change, action=llight_on:value_setter()},
    {event=x56throttle.button13.change, action=llight_off:value_setter()},
    {event=x56throttle.button18.change, action=flight_aux1:value_setter()},
    {event=x56throttle.button19.change, action=flight_aux2:value_setter()},
}

local joymap_combat = {
    {event=x56throttle.button6.change, action=aa_mode:value_setter()},
    {event=x56throttle.button7.change, action=ag_mode:value_setter()},
    {event=x56throttle.button8.change, action=arm_aux1:value_setter()},
    {event=x56throttle.button9.change, action=arm_aux2:value_setter()},
    {event=x56throttle.button10.change, action=arm_aux3:value_setter()},
    {event=x56throttle.button11.change, action=arm_aux4:value_setter()},
}

local joymap ={
    base = joymap_dcs,
    modal = {}
}

function update_secondary_mappings()
    mapper.set_secondary_mappings(joymap.base)
    mapper.add_secondary_mappings(joymap.modal)
end

update_secondary_mappings()


viewport:add_mappings({
    {event=x56throttle.button28.down, action=toggle_view},
    {event=x56throttle.button29.down, action=toggle_view},
})
mapper.start_viewports()

mapper.set_primery_mappings({
    {event=mapper.events.change_aircraft, action=function (event, value)
        if value.host ~= "fs2020" then
            joymap.base = joymap_dcs
            update_secondary_mappings()
        elseif value.aircraft == "Airbus A320 Neo FlyByWire" then
            joymap.base = joymap_full
            update_secondary_mappings()
        else
            joymap.base = joymap_noab
            update_secondary_mappings()
        end
    end},

    {event=x56throttle.button34.down, action=filter.duplicator(
        function ()
            joymap.modal = joymap_combat
            update_secondary_mappings()
            marm_on:set_value(true)
        end,
        filter.delay(200, marm_on:value_setter(false)))
    },

    {event=x56throttle.button35.down, action=function ()
        joymap.modal = joymap_inflight
        update_secondary_mappings()
        marm_off:set_value(true)
        mapper.delay(200, function () marm_off:set_value(false) end)
    end},

    {event=x56throttle.button36.down, action=function ()
        joymap.modal = joymap_preflight
        update_secondary_mappings()
    end},
})
