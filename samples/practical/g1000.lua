local config = {
    simhid_g1000_identifier = {path = "COM3"},
    simhid_g1000_display = 2,
}

local common = require("lib/common")
common.commit_config(config)

g1000_dev = common.open_simhid_g1000{
    config = config,
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
}
local g1000 = g1000_dev.events

local pfd_maps = {
    {event=g1000.EC1.increment, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_VOL_1_INC)")},
    {event=g1000.EC1.decrement, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_VOL_1_DEC)")},
    {event=g1000.EC2X.increment, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_NAV_Small_INC)")},
    {event=g1000.EC2X.decrement, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_NAV_Small_DEC)")},
    {event=g1000.EC2Y.increment, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_NAV_Large_INC)")},
    {event=g1000.EC2Y.decrement, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_NAV_Large_DEC)")},
    {event=g1000.EC2P.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_NAV_Push)")},
    {event=g1000.EC3.increment, action=msfs.mfwasm.rpn_executer("(>K:HEADING_BUG_INC)")},
    {event=g1000.EC3.decrement, action=msfs.mfwasm.rpn_executer("(>K:HEADING_BUG_DEC)")},
    {event=g1000.EC3P.down, action=msfs.mfwasm.rpn_executer("(A:HEADING INDICATOR, degrees) (>K:HEADING_BUG_SET)")},
    {event=g1000.EC4X.increment, action=msfs.mfwasm.rpn_executer("(A:AUTOPILOT ALTITUDE LOCK VAR, feet) 100 + (>K:AP_ALT_VAR_SET_ENGLISH) (>H:AP_KNOB_Up)")},
    {event=g1000.EC4X.decrement, action=msfs.mfwasm.rpn_executer("(A:AUTOPILOT ALTITUDE LOCK VAR, feet) 100 - (>K:AP_ALT_VAR_SET_ENGLISH) (>H:AP_KNOB_Up)")},
    {event=g1000.EC4Y.increment, action=msfs.mfwasm.rpn_executer("(A:AUTOPILOT ALTITUDE LOCK VAR, feet) 1000 + (>K:AP_ALT_VAR_SET_ENGLISH) (>H:AP_KNOB_Up)")},
    {event=g1000.EC4Y.decrement, action=msfs.mfwasm.rpn_executer("(A:AUTOPILOT ALTITUDE LOCK VAR, feet) 1000 - (>K:AP_ALT_VAR_SET_ENGLISH) (>H:AP_KNOB_Up)")},
    {event=g1000.SW1.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_NAV_Switch)")},
    {event=g1000.SW2.down, action=msfs.mfwasm.rpn_executer("(>K:AP_MASTER)")},
    {event=g1000.SW3.down, action=msfs.mfwasm.rpn_executer("(>K:TOGGLE_FLIGHT_DIRECTOR)")},
    {event=g1000.SW4.down, action=msfs.mfwasm.rpn_executer("(>K:AP_HDG_HOLD)")},
    {event=g1000.SW5.down, action=msfs.mfwasm.rpn_executer("(>K:AP_ALT_HOLD)")},
    {event=g1000.SW6.down, action=msfs.mfwasm.rpn_executer("(>K:AP_NAV1_HOLD)")},
    {event=g1000.SW7.down, action=msfs.mfwasm.rpn_executer("(L:XMLVAR_VNAVButtonValue, Bool) ! (>L:XMLVAR_VNAVButtonValue)")},
    {event=g1000.SW8.down, action=msfs.mfwasm.rpn_executer("(>K:AP_APR_HOLD)")},
    {event=g1000.SW9.down, action=msfs.mfwasm.rpn_executer("(>K:AP_BC_HOLD)")},
    {event=g1000.SW10.down, action=msfs.mfwasm.rpn_executer("(>K:AP_PANEL_VS_HOLD)")},
    {event=g1000.SW11.down, action=msfs.mfwasm.rpn_executer("(A:AUTOPILOT VERTICAL HOLD, Bool) if{ (>K:AP_VS_VAR_INC) (>H:AP_DN) } (A:AUTOPILOT FLIGHT LEVEL CHANGE, Bool) if{ (>K:AP_SPD_VAR_DEC) } (A:AUTOPILOT PITCH HOLD, Bool) if{ (>K:AP_PITCH_REF_INC_UP) }")},
    {event=g1000.SW12.down, action=msfs.mfwasm.rpn_executer("(>K:FLIGHT_LEVEL_CHANGE) (A:AIRSPEED INDICATED, knots) (>K:AP_SPD_VAR_SET)")},
    {event=g1000.SW13.down, action=msfs.mfwasm.rpn_executer("(A:AUTOPILOT VERTICAL HOLD, Bool) if{ (>K:AP_VS_VAR_DEC) (>H:AP_UP) } (A:AUTOPILOT FLIGHT LEVEL CHANGE, Bool) if{ (>K:AP_SPD_VAR_INC) } (A:AUTOPILOT PITCH HOLD, Bool) if{ (>K:AP_PITCH_REF_INC_DN) }")},

    {event=g1000.SW14.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_SOFTKEYS_1)")},
    {event=g1000.SW15.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_SOFTKEYS_2)")},
    {event=g1000.SW16.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_SOFTKEYS_3)")},
    {event=g1000.SW17.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_SOFTKEYS_4)")},
    {event=g1000.SW18.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_SOFTKEYS_5)")},
    {event=g1000.SW19.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_SOFTKEYS_6)")},
    {event=g1000.SW20.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_SOFTKEYS_7)")},
    {event=g1000.SW21.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_SOFTKEYS_8)")},
    {event=g1000.SW22.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_SOFTKEYS_9)")},
    {event=g1000.SW23.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_SOFTKEYS_10)")},
    {event=g1000.SW24.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_SOFTKEYS_11)")},
    {event=g1000.SW25.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_SOFTKEYS_12)")},

    {event=g1000.EC5.increment, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_VOL_2_INC)")},
    {event=g1000.EC5.decrement, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_VOL_2_DEC)")},
    {event=g1000.EC6X.increment, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_COM_Small_INC)")},
    {event=g1000.EC6X.decrement, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_COM_Small_DEC)")},
    {event=g1000.EC6Y.increment, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_COM_Large_INC)")},
    {event=g1000.EC6Y.decrement, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_COM_Large_DEC)")},
    {event=g1000.EC6P.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_COM_Push)")},
    {event=g1000.EC7X.increment, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_CRS_INC)")},
    {event=g1000.EC7X.decrement, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_CRS_DEC)")},
    {event=g1000.EC7Y.increment, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_BARO_INC)")},
    {event=g1000.EC7Y.decrement, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_BARO_DEC)")},
    {event=g1000.EC7P.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_CRS_PUSH)")},
    {event=g1000.EC8.increment, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_RANGE_INC)")},
    {event=g1000.EC8.decrement, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_RANGE_DEC)")},
    {event=g1000.EC8P.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_JOYSTICK_PUSH)")},
    {event=g1000.EC8U.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_JOYSTICK_UP)")},
    {event=g1000.EC8D.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_JOYSTICK_DOWN)")},
    {event=g1000.EC8R.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_JOYSTICK_RIGHT)")},
    {event=g1000.EC8L.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_JOYSTICK_LEFT)")},
    {event=g1000.EC9X.increment, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_FMS_Upper_INC)")},
    {event=g1000.EC9X.decrement, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_FMS_Upper_DEC)")},
    {event=g1000.EC9Y.increment, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_FMS_Lower_INC)")},
    {event=g1000.EC9Y.decrement, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_FMS_Lower_DEC)")},
    {event=g1000.EC9P.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_FMS_Upper_PUSH)")},

    {event=g1000.SW26.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_COM_Switch)")},
    {event=g1000.SW26.longpressed, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_COM_Switch_Long)")},
    {event=g1000.SW27.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_DIRECTTO)")},
    {event=g1000.SW28.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_MENU_Push)")},
    {event=g1000.SW29.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_FPL_Push)")},
    {event=g1000.SW30.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_PROC_Push)")},
    {event=g1000.SW31.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_CLR)")},
    {event=g1000.SW31.longpressed, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_CLR_Long)")},
    {event=g1000.SW32.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_PFD_ENT_Push)")},
}

local mfd_maps = {
    {event=g1000.EC1.increment, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_VOL_1_INC)")},
    {event=g1000.EC1.decrement, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_VOL_1_DEC)")},
    {event=g1000.EC2X.increment, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_NAV_Small_INC)")},
    {event=g1000.EC2X.decrement, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_NAV_Small_DEC)")},
    {event=g1000.EC2Y.increment, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_NAV_Large_INC)")},
    {event=g1000.EC2Y.decrement, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_NAV_Large_DEC)")},
    {event=g1000.EC2P.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_NAV_Push)")},
    {event=g1000.EC3.increment, action=msfs.mfwasm.rpn_executer("(>K:HEADING_BUG_INC)")},
    {event=g1000.EC3.decrement, action=msfs.mfwasm.rpn_executer("(>K:HEADING_BUG_DEC)")},
    {event=g1000.EC3P.down, action=msfs.mfwasm.rpn_executer("(A:HEADING INDICATOR, degrees) (>K:HEADING_BUG_SET)")},
    {event=g1000.EC4X.increment, action=msfs.mfwasm.rpn_executer("(A:AUTOPILOT ALTITUDE LOCK VAR, feet) 100 + (>K:AP_ALT_VAR_SET_ENGLISH) (>H:AP_KNOB_Up)")},
    {event=g1000.EC4X.decrement, action=msfs.mfwasm.rpn_executer("(A:AUTOPILOT ALTITUDE LOCK VAR, feet) 100 - (>K:AP_ALT_VAR_SET_ENGLISH) (>H:AP_KNOB_Up)")},
    {event=g1000.EC4Y.increment, action=msfs.mfwasm.rpn_executer("(A:AUTOPILOT ALTITUDE LOCK VAR, feet) 1000 + (>K:AP_ALT_VAR_SET_ENGLISH) (>H:AP_KNOB_Up)")},
    {event=g1000.EC4Y.decrement, action=msfs.mfwasm.rpn_executer("(A:AUTOPILOT ALTITUDE LOCK VAR, feet) 1000 - (>K:AP_ALT_VAR_SET_ENGLISH) (>H:AP_KNOB_Up)")},
    {event=g1000.SW1.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_NAV_Switch)")},
    {event=g1000.SW2.down, action=msfs.mfwasm.rpn_executer("(>K:AP_MASTER)")},
    {event=g1000.SW3.down, action=msfs.mfwasm.rpn_executer("(>K:TOGGLE_FLIGHT_DIRECTOR)")},
    {event=g1000.SW4.down, action=msfs.mfwasm.rpn_executer("(>K:AP_HDG_HOLD)")},
    {event=g1000.SW5.down, action=msfs.mfwasm.rpn_executer("(>K:AP_ALT_HOLD)")},
    {event=g1000.SW6.down, action=msfs.mfwasm.rpn_executer("(>K:AP_NAV1_HOLD)")},
    {event=g1000.SW7.down, action=msfs.mfwasm.rpn_executer("(L:XMLVAR_VNAVButtonValue, Bool) ! (>L:XMLVAR_VNAVButtonValue)")},
    {event=g1000.SW8.down, action=msfs.mfwasm.rpn_executer("(>K:AP_APR_HOLD)")},
    {event=g1000.SW9.down, action=msfs.mfwasm.rpn_executer("(>K:AP_BC_HOLD)")},
    {event=g1000.SW10.down, action=msfs.mfwasm.rpn_executer("(>K:AP_PANEL_VS_HOLD)")},
    {event=g1000.SW11.down, action=msfs.mfwasm.rpn_executer("(A:AUTOPILOT VERTICAL HOLD, Bool) if{ (>K:AP_VS_VAR_INC) (>H:AP_DN) } (A:AUTOPILOT FLIGHT LEVEL CHANGE, Bool) if{ (>K:AP_SPD_VAR_DEC) } (A:AUTOPILOT PITCH HOLD, Bool) if{ (>K:AP_PITCH_REF_INC_UP) }")},
    {event=g1000.SW12.down, action=msfs.mfwasm.rpn_executer("(>K:FLIGHT_LEVEL_CHANGE) (A:AIRSPEED INDICATED, knots) (>K:AP_SPD_VAR_SET)")},
    {event=g1000.SW13.down, action=msfs.mfwasm.rpn_executer("(A:AUTOPILOT VERTICAL HOLD, Bool) if{ (>K:AP_VS_VAR_DEC) (>H:AP_UP) } (A:AUTOPILOT FLIGHT LEVEL CHANGE, Bool) if{ (>K:AP_SPD_VAR_INC) } (A:AUTOPILOT PITCH HOLD, Bool) if{ (>K:AP_PITCH_REF_INC_DN) }")},

    {event=g1000.SW14.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_SOFTKEYS_1)")},
    {event=g1000.SW15.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_SOFTKEYS_2)")},
    {event=g1000.SW16.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_SOFTKEYS_3)")},
    {event=g1000.SW17.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_SOFTKEYS_4)")},
    {event=g1000.SW18.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_SOFTKEYS_5)")},
    {event=g1000.SW19.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_SOFTKEYS_6)")},
    {event=g1000.SW20.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_SOFTKEYS_7)")},
    {event=g1000.SW21.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_SOFTKEYS_8)")},
    {event=g1000.SW22.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_SOFTKEYS_9)")},
    {event=g1000.SW23.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_SOFTKEYS_10)")},
    {event=g1000.SW24.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_SOFTKEYS_11)")},
    {event=g1000.SW25.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_SOFTKEYS_12)")},

    {event=g1000.EC5.increment, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_VOL_2_INC)")},
    {event=g1000.EC5.decrement, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_VOL_2_DEC)")},
    {event=g1000.EC6X.increment, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_COM_Small_INC)")},
    {event=g1000.EC6X.decrement, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_COM_Small_DEC)")},
    {event=g1000.EC6Y.increment, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_COM_Large_INC)")},
    {event=g1000.EC6Y.decrement, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_COM_Large_DEC)")},
    {event=g1000.EC6P.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_COM_Push)")},
    {event=g1000.EC7X.increment, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_CRS_INC)")},
    {event=g1000.EC7X.decrement, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_CRS_DEC)")},
    {event=g1000.EC7Y.increment, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_BARO_INC)")},
    {event=g1000.EC7Y.decrement, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_BARO_DEC)")},
    {event=g1000.EC7P.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_CRS_PUSH)")},
    {event=g1000.EC8.increment, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_RANGE_INC)")},
    {event=g1000.EC8.decrement, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_RANGE_DEC)")},
    {event=g1000.EC8P.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_JOYSTICK_PUSH)")},
    {event=g1000.EC8U.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_JOYSTICK_UP)")},
    {event=g1000.EC8D.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_JOYSTICK_DOWN)")},
    {event=g1000.EC8R.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_JOYSTICK_RIGHT)")},
    {event=g1000.EC8L.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_JOYSTICK_LEFT)")},
    {event=g1000.EC9X.increment, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_FMS_Upper_INC)")},
    {event=g1000.EC9X.decrement, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_FMS_Upper_DEC)")},
    {event=g1000.EC9Y.increment, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_FMS_Lower_INC)")},
    {event=g1000.EC9Y.decrement, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_FMS_Lower_DEC)")},
    {event=g1000.EC9P.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_FMS_Upper_PUSH)")},

    {event=g1000.SW26.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_COM_Switch)")},
    {event=g1000.SW26.longpressed, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_COM_Switch_Long)")},
    {event=g1000.SW27.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_DIRECTTO)")},
    {event=g1000.SW28.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_MENU_Push)")},
    {event=g1000.SW29.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_FPL_Push)")},
    {event=g1000.SW30.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_PROC_Push)")},
    {event=g1000.SW31.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_CLR)")},
    {event=g1000.SW31.longpressed, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_CLR_Long)")},
    {event=g1000.SW32.down, action=msfs.mfwasm.rpn_executer("(>H:AS1000_MFD_ENT_Push)")},
}

local viewport = mapper.viewport({
    name = "G1000 Viewport",
    displayno = config.simhid_g1000_display,
    bgcolor = "Black",
    x = 0, y =0, width = config.scale_horizontal, height = config.scale_vertical,
    aspect_ratio = 4.0 / 3.0,
})
local pfd = viewport:register_view({
    name = "PFD",
    elements = {{object = mapper.captured_window({name = "G1000 PFD", window_title="AS1000_PFD"})}},
    mappings = pfd_maps,
})
local mfd = viewport:register_view({
    name = "MFD",
    elements = {{object = mapper.captured_window({name = "G1000 MFD", window_title="AS1000_MFD"})}},
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
})
viewport:add_mappings({
    {event=g1000.AUX2D.down, action=toggle_view},
    {event=g1000.AUX2U.down, action=toggle_view},
})

mapper.start_viewports()
