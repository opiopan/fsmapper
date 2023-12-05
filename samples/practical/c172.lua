local config = {
    -- debug = true,
    simhid_g1000_identifier = {path = "COM3"},
    simhid_g1000_display = 2,
}

local common = require("lib/common")
local libs = {
    gns530 = require("lib/gns530"),
    gns430 = require("lib/gns430"),
    kap140 = require("lib/kap140"),
    kt76c = require("lib/kt76c"),
    kr87 = require("lib/kr87"),
    c172navgps = require("lib/c172navgps"),
    cdi = require("lib/cdi"),
    adf = require("lib/adf"),
    dg = require("lib/dg"),
}
local lib_options = {
    cdi = {
        {type=libs.cdi.type.general, gps_dependency=true, enable_nav=false, source_is_gps="(L:AS530_CDI_Source_1)"}, -- NAV1
        {type=libs.cdi.type.general, gps_dependency=true, enable_nav=false, source_is_gps="(L:AS430_CDI_Source_1)"}, -- NAV2
    },
    dg = {
        {type=libs.dg.type.general, red_mark=true, heading_bug=true},
    },
}

local captured_window_defs ={
    {key="gns530", name="GNS530 GPS", window_title="AS530"},
    {key="gns430", name="GNS430 GPS", window_title="AS430"},
    {key="kap140", name="KAP-140 Auto Pilot Control", window_title="KAP140"},
    {key="kr87", name="KR-87 ADF", window_title="KR87"},
}

local bg_color = graphics.color(30, 40, 50)
local views = {
    {
        -----------------------------------------------------------------------------------
        name = "Normal View",
        viewid = nil,
        width = 2224, height = 1668,
        background_color = bg_color,
        background_regions = {
            {x=0, y=569, width=1112, height=1099},
            {x=1112, y=0, width=1112, height=126},
        },
        components = {
            {name="gns530", module=libs.gns530, cw="gns530", type_id=1, x=1112, y=126, scale=1},
            {name="gns430", module=libs.gns430, cw="gns430", type_id=1, x=1112, y=942, scale=1},
            {name="kap140", module=libs.kap140, cw="kap140", type_id=1, x=0, y=0, scale=1},
            {name="kt76c", module=libs.kt76c, cw=nil, type_id=1, x=0, y=296, scale=1},
            {name="kr87", module=libs.kr87, cw="kr87", type_id=1, x=1112, y=1408, scale=1},
            {name="navgps", module=libs.c172navgps, cw=nil, type_id=1, x=1481.333, y=0, scale=1},
            {name="NAV1 CDI", module=libs.cdi, cw=nil, type_id=1, x=579.119, y=601, scale=1},
            {name="NAV2 CDI", module=libs.cdi, cw=nil, type_id=2, x=579.119, y=1136, scale=1},
            {name="ADF", module=libs.adf, cw=nil, type_id=1, x=32.881, y=1136, scale=1},
            {name="DG", module=libs.dg, cw=nil, type_id=1, x=32.881, y=601, scale=1},
        },
        initial_active_component = 1,       
    },
    {
        -----------------------------------------------------------------------------------
        name = "NAV1 View",
        viewid = nil,
        width = 2224, height = 1668,
        background_color = bg_color,
        background_regions = {
            {x=0, y=0, width=556, height=1668},
        },
        components = {
            {name="gns530", module=libs.gns530, cw="gns530", type_id=1, x=556, y=0, scale=1.5},
            {name="kap140", module=libs.kap140, cw="kap140", type_id=1, x=556, y=1224, scale=1.5},
            {name="navgps", module=libs.c172navgps, cw=nil, type_id=1, x=94.74, y=60.211, scale=1},
            {name="DG", module=libs.dg, cw=nil, type_id=1, x=31.406, y=288.234, scale=1},
            {name="NAV1 CDI", module=libs.cdi, cw=nil, type_id=1, x=31.406, y=879.766, scale=1},
        },
        initial_active_component = 1,       
    },
    {
        -----------------------------------------------------------------------------------
        name = "NAV2 View",
        viewid = nil,
        width = 2224, height = 1668,
        background_color = bg_color,
        background_regions = {
            {x=0, y=0, width=556, height=1668},
            {x=556, y=1566, width=1668, height=102},
        },
        components = {
            {name="gns430", module=libs.gns430, cw="gns430", type_id=1, x=556, y=0, scale=1.5},
            {name="kap140", module=libs.kap140, cw="kap140", type_id=1, x=556, y=699, scale=1.5},
            {name="kt76c", module=libs.kt76c, cw=nil, type_id=1, x=556, y=1143, scale=1.5},
            {name="navgps", module=libs.c172navgps, cw=nil, type_id=1, x=94.74, y=60.211, scale=1},
            {name="DG", module=libs.dg, cw=nil, type_id=1, x=31.406, y=288.234, scale=1},
            {name="NAV1 CDI", module=libs.cdi, cw=nil, type_id=2, x=31.406, y=879.766, scale=1},
        },
        initial_active_component = 1,       
    },
}

local display = config.simhid_g1000_display
local scale = 1.0
if config.debug then
    display = 1
    scale = 0.5
end

device = mapper.device{
    name = "SimHID G1000",
    type = "simhid",
    identifier = config.simhid_g1000_identifier,
    modifiers = {
        {class = "binary", modtype = "button"},
        {class = "relative", modtype = "incdec"},
        {name = "SW31", modtype = "button", modparam={longpress = 2000}},
    },
}
local g1000 = device.events

common.init_component_modules(libs, lib_options)

local viewport = mapper.viewport{
    name = "C172 viewport",
    displayno = display,
    x = 0, y = 0, width = scale, height = scale,
    aspect_ratio = 4 / 3,
}
local viewport_mappings = {}
local view_changer = common.create_default_view_changer(viewport, views, 1, viewport_mappings, device, {})
common.arrange_views(viewport, viewport_mappings, captured_window_defs, views, device)
viewport:set_mappings(viewport_mappings)
local target_view = views[1]
viewport:add_mappings(target_view.components[target_view.active_component].instance.component_mappings)

local global_mappings = {
    {
        {event=g1000.EC3.increment, action=fs2020.mfwasm.rpn_executer("1 (>K:HEADING_BUG_INC)")},
        {event=g1000.EC3.decrement, action=fs2020.mfwasm.rpn_executer("1 (>K:HEADING_BUG_DEC)")},
        {event=g1000.EC4X.increment, action=fs2020.mfwasm.rpn_executer("100 (>K:AP_ALT_VAR_INC)")},
        {event=g1000.EC4X.decrement, action=fs2020.mfwasm.rpn_executer("100 (>K:AP_ALT_VAR_DEC)")},
        {event=g1000.EC4Y.increment, action=fs2020.mfwasm.rpn_executer("1000 (>K:AP_ALT_VAR_INC)")},
        {event=g1000.EC4Y.decrement, action=fs2020.mfwasm.rpn_executer("1000 (>K:AP_ALT_VAR_DEC)")},
    }
}
common.set_global_mappings(global_mappings, libs)

for i, mappings in ipairs(global_mappings) do
    mapper.add_primary_mappings(mappings)
end

mapper.start_viewports()
