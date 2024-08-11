local module = {
    width = 500,
    height = 500,
    type = {
        general = 1,
        no_glideslope = 2,
    },
}

local module_defs = {
    prefix = "CDI",
    activatable = true,
    options = {{}, {}},
    option_defaults = {
        type = module.type.general,
        gps_dependency  = false,
        source_is_gps = "0",  -- rpn fragment that return non-zero when cdi source is GPS
        enable_nav = true, 
    },
}

local common = require("lib/common")

--------------------------------------------------------------------------------------
-- action definitions
--------------------------------------------------------------------------------------
module.actions = {}
module.actions[1] = {
    obs_inc = msfs.event_sender("VOR1_OBI_INC"),
    obs_dec = msfs.event_sender("VOR1_OBI_DEC"),
}

module.actions[2] = {
    obs_inc = msfs.event_sender("VOR2_OBI_INC"),
    obs_dec = msfs.event_sender("VOR2_OBI_DEC"),
}

--------------------------------------------------------------------------------------
-- indicator definitions
--------------------------------------------------------------------------------------
local instrument_parts = require("lib/instrument_parts")
local cdi_parts = common.merge_table({}, instrument_parts)
local source_indicator = graphics.bitmap(71*2, 118)
local rctx = graphics.rendering_context(source_indicator)
rctx:draw_bitmap{bitmap=cdi_parts.cdi_vloc.image, x=0, y=0}
rctx:draw_bitmap{bitmap=cdi_parts.cdi_gps.image, x=71 + 8.738, y=86.797}
rctx:finish_rendering()
cdi_parts.source ={
    source_indicator:create_partial_bitmap(0, 0, 71, 118),
    source_indicator:create_partial_bitmap(71, 0, 71, 118),
}

local needle_width = 6
local needle_movable_length = 240
local needle_color=graphics.color("white")
local needle_canvas_size = needle_movable_length + needle_width
local needle_value_scale = needle_movable_length / (127 * 2)
cdi_parts.cdi_needle = {image = graphics.bitmap(needle_width, needle_movable_length)}
cdi_parts.gs_needle = {image = graphics.bitmap(needle_movable_length, needle_width)}
rctx = graphics.rendering_context(cdi_parts.cdi_needle.image)
rctx:set_brush(needle_color)
rctx:fill_rectangle(0, 0, cdi_parts.cdi_needle.image.width, cdi_parts.cdi_needle.image.height)
rctx:finish_rendering()
rctx = graphics.rendering_context(cdi_parts.gs_needle.image)
rctx:set_brush(needle_color)
rctx:fill_rectangle(0, 0, cdi_parts.gs_needle.image.width, cdi_parts.gs_needle.image.height)
rctx:finish_rendering()
cdi_parts.cdi_needle.image:set_origin(needle_movable_length / -2, needle_width / -2)
cdi_parts.gs_needle.image:set_origin(needle_width / -2, needle_movable_length  / -2)

module_defs.indicators ={}
module_defs.indicators[module.type.general] = {}
module_defs.indicators[module.type.general][1]= {
    heading_indicator = {x=237, y=92.731, attr={width=26, height=22.517}, bitmaps={cdi_parts.cdi_heading.image}},
    tail_indicator = {x=242.5, y=383.711, attr={width=15, height=12.99}, bitmaps={cdi_parts.cdi_tail.image}},
    bearing_indicator = {
        x=36.577, y=36.577, attr={width=427.05, height=427.05}, rotation={bitmap=cdi_parts.cdi_bearing.image, center={x=427.05/2, y=427.05/2}},
        rpn="360 (A:NAV OBS:1, Degrees) -"
    },
    cdi_needle = {
        x=(module.width - needle_canvas_size) / 2, y=(module.height - needle_canvas_size) / 2, attr={width=needle_canvas_size, height=needle_canvas_size},
        shift={bitmap=cdi_parts.cdi_needle.image, axis="x", scale=needle_value_scale}, 
        rpn="%s if{ (A:GPS CDI NEEDLE:1, Number) } els{ (A:NAV CDI:1, Number) }", epsilon=0.5
    },
    gs_needle = {
        x=(module.width - needle_canvas_size) / 2, y=(module.height - needle_canvas_size) / 2, attr={width=needle_canvas_size, height=needle_canvas_size},
        shift={bitmap=cdi_parts.gs_needle.image, axis="y", scale=needle_value_scale}, 
        rpn="%s if{ (A:GPS GSI NEEDLE:1, Number) } els{ (A:NAV GSI:1, Number) }", epsilon = 0.5
    },
    source_indicator = {
        x=138.201, y=194.857, attr={width=70.606, height=117.375}, bitmaps={cdi_parts.source[1], cdi_parts.source[2]}, rpn="%s",
        enable_condition=function (option) return option.gps_dependency end
    },
    nav_indicator = {
        x=145.419, y=157.943, attr={width=54.444, height=29.862}, bitmaps={nil, cdi_parts.cdi_nav.image},
        rpn="%s if{ (A:GPS IS ACTIVE FLIGHT PLAN:1, Bool) } els{ (A:NAV HAS NAV:1, Bool) }",
        enable_condition=function (option) return option.enable_nav end
    },
    tofrom_indicator = {
        x=281.399, y=168.058, attr={width=83.607, height=36.622}, bitmaps={nil, cdi_parts.cdi_to.image, cdi_parts.cdi_from.image},
        rpn="%s if{ (A:GPS IS ACTIVE FLIGHT PLAN, Bool) if{ (A:GPS WP TRUE BEARING, Degrees) (A:GPS WP TRUE REQ HDG, Degrees) - 90 + d360 180 < if{ 1 } els{ 2 } } els{  0 } } els{ (A:NAV TOFROM:1, Enum) }"
    },
    na_loc_indicator = {
        x=218.346, y=374.625, attr={width=63.307, height=22.075}, bitmaps={nil, cdi_parts.cdi_na_loc.image},
        rpn="%s if{ (A:GPS IS ACTIVE FLIGHT PLAN:1, Bool) } els{ (A:NAV HAS NAV:1, Bool) } !"
    },
    na_gs_indicator = {
        x=375, y=218.346, attr={width=22.075, height=63.306}, bitmaps={nil, cdi_parts.cdi_na_gs.image},
        rpn="%s if{ (A:GPS IS ACTIVE FLIGHT PLAN:1, Bool) (A:GPS HAS GLIDEPATH:1, Bool) and } els{ (A:NAV GS FLAG:1, Bool) } !"
    },
}

module_defs.indicators[module.type.general][2] = {}
for name, data in pairs(module_defs.indicators[module.type.general][1]) do
    module_defs.indicators[module.type.general][2][name] = common.merge_table({}, data)
end
module_defs.indicators[module.type.general][2].bearing_indicator.rpn = "360 (A:NAV OBS:2, Degrees) -"
module_defs.indicators[module.type.general][2].cdi_needle.rpn = "%s if{ (A:GPS CDI NEEDLE:1, Number) } els{ (A:NAV CDI:2, Number) }"
module_defs.indicators[module.type.general][2].gs_needle.rpn = "%s if{ (A:GPS HSI NEEDLE:1, Number) } els{ (A:NAV GSI:2, Number) }"
module_defs.indicators[module.type.general][2].nav_indicator.rpn = "%s if{ (A:GPS IS ACTIVE FLIGHT PLAN:1, Bool) } els{ (A:NAV HAS NAV:2, Bool) }"
module_defs.indicators[module.type.general][2].tofrom_indicator.rpn = "%s if{ (A:GPS IS ACTIVE FLIGHT PLAN, Bool) if{ (A:GPS WP TRUE BEARING, Degrees) (A:GPS WP TRUE REQ HDG, Degrees) - 90 + d360 180 < if{ 1 } els{ 2 } } els{  0 } } els{ (A:NAV TOFROM:2, Enum) }"
module_defs.indicators[module.type.general][2].na_loc_indicator.rpn = "%s if{ (A:GPS IS ACTIVE FLIGHT PLAN:1, Bool) } els{ (A:NAV HAS NAV:2, Bool) } !"
module_defs.indicators[module.type.general][2].na_gs_indicator.rpn = "%s if{ (A:GPS IS ACTIVE FLIGHT PLAN:1, Bool) (A:GPS HAS GLIDEPATH:1, Bool) and } els{ (A:NAV GS FLAG:2, Bool) } !"

module_defs.indicator_orders = {}
module_defs.indicator_orders[module.type.general] = {
    "heading_indicator",
    "tail_indicator",
    "bearing_indicator",
    "cdi_needle",
    "gs_needle",
    "source_indicator",
    "nav_indicator",
    "tofrom_indicator",
    "na_loc_indicator",
    "na_gs_indicator",
}

--------------------------------------------------------------------------------------
-- active indicator difinitions
--------------------------------------------------------------------------------------
module_defs.active_indicators= {}
module_defs.active_indicators[module.type.general] = {
    {x=18.44, y=410, width=81.624, height=81.624},
}

--------------------------------------------------------------------------------------
-- prepare module scope environment
--------------------------------------------------------------------------------------
common.component_module_init(module, module_defs)

--------------------------------------------------------------------------------------
-- reset function called when aircraft evironment is build each
--   override the default function which set in common.component_module_init()
--------------------------------------------------------------------------------------
local default_reset = module.reset

module.reset = function (options)
    default_reset(options)

    module.observed_data = {}
    for i, option in ipairs(module_defs.options) do
        for name, indicator in pairs(module_defs.indicators[option.type][i]) do
            if indicator.rpn ~= nil then
                module.observed_data[#module.observed_data + 1] = {
                    rpn = string.format(indicator.rpn, option.source_is_gps),
                    epsilon= indicator.epsilon,
                    event = module.events[i][name]
                }
            end
        end
    end
end

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
    rctx:draw_bitmap{bitmap=cdi_parts.base.image, x=x, y=y, scale=scale}
    rctx:draw_bitmap{bitmap=cdi_parts.cdi_knob.image, x=x + 18.44 * scale, y=y + 410 * scale, scale=scale}

    -- Event-Action mappings which are enabled when the component is activated
    if simhid_g1000 then
        local g1000 = simhid_g1000.events
        component.component_mappings = {
            {event=g1000.EC7X.increment, action=module.actions[id].obs_inc},
            {event=g1000.EC7X.decrement, action=module.actions[id].obs_dec},
        }
    end

    return component
end

return module
