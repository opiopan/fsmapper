local disp_width = 1084
local disp_height = 87

local module = {}

local assets = require('a32nx/assets')
local common = require('lib/common')
local segdisp = require('lib/segdisp')

--------------------------------------------------------------------------------------
-- Image assets definition
--------------------------------------------------------------------------------------
local segfont = segdisp.create_font{type=segdisp.seg7_type2, height=100, width=96.253, color=assets.sseg_font_color}
local parts = {
    vstatic = assets.fcu_parts:create_partial_bitmap(0, 0, 206.54, 28.623),
    spd = assets.fcu_parts:create_partial_bitmap(0, 28.623, 45.096, 27),
    mach = assets.fcu_parts:create_partial_bitmap(47, 28.623, 63.145, 27),
    hdg = assets.fcu_parts:create_partial_bitmap(112, 28.623, 49.386, 27),
    trk = assets.fcu_parts:create_partial_bitmap(164, 28.623, 43.886, 27),
    lstatic = assets.fcu_parts:create_partial_bitmap(0, 57.479, 40.83, 27),
    vs = assets.fcu_parts:create_partial_bitmap(45.096, 57.479, 45.22, 27),
    fpa = assets.fcu_parts:create_partial_bitmap(95.015, 57.479, 40.004, 27),
    managed = assets.fcu_parts:create_partial_bitmap(141.708, 57.479, 22, 22),
}

--------------------------------------------------------------------------------------
-- context for drawing
--------------------------------------------------------------------------------------
local disp_context = {
    power = 1, -- 0:off, 1:on, 2:test
    nav_mode = 0, -- 0:hdg/vs, 1:trk/fpa
    vs_mode = 0, -- 1:dash, others:selected value
    vs_fpa = 0,
}

--------------------------------------------------------------------------------------
-- Declare canvas
--------------------------------------------------------------------------------------
local canvas_defs = {
    nav_mode1 = {
        rect = {250.246, 5.035, 815.097, 28.624}, value = 0,
        renderer = function (rctx, value)
            if disp_context.power > 0 then
                rctx:draw_bitmap(parts.lstatic, 109.179, 0)
                rctx:draw_bitmap(parts.vstatic, 512.264, 0)
                if disp_context.nav_mode == 0 or disp_context.power == 2 then
                    rctx:draw_bitmap(parts.hdg)
                    rctx:draw_bitmap(parts.vs, 723.92, 0.812)
                end
                if disp_context.nav_mode == 1 or disp_context.power == 2 then
                    rctx:draw_bitmap(parts.trk, 58.735, 0)
                    rctx:draw_bitmap(parts.fpa, 775.093, 0.812)
                end
            end
        end
    },

    nav_mode2 = {
        rect = {468.937, 29.188, 141.283, 55.332}, value = 0,
        renderer = function (rctx, value)
            if disp_context.power > 0 then
                if disp_context.nav_mode == 0 or disp_context.power == 2 then
                    rctx:draw_bitmap(parts.hdg)
                    rctx:draw_bitmap(parts.vs, 96.063, 0)
                end
                if disp_context.nav_mode == 1 or disp_context.power == 2 then
                    rctx:draw_bitmap(parts.trk, 2.608, 28.332)
                    rctx:draw_bitmap(parts.fpa, 98.063, 28.332)
                end
            end
        end
    },

    spd_mode = {
        rect = {33.315, 5.035, 113.821, 26.234}, value = 0,
        renderer = function (rctx, value)
            if disp_context.power == 1 then
                if value == 0 then
                    rctx:draw_bitmap(parts.spd)
                else
                    rctx:draw_bitmap(parts.mach, 50.676, 0)
                end
            elseif disp_context.power == 2 then
                rctx:draw_bitmap(parts.spd)
                rctx:draw_bitmap(parts.mach, 50.676, 0)
            end
        end
    },

    spd = {
        rect = {39.622, 37.623, 104.264, 36.108}, value = -1,
        logical_size = {288.756, 100},
        renderer = function (rctx, value)
            rctx.font = segfont
            if disp_context.power == 1 then
                if value > 1 then
                    rctx:draw_number{
                        value = value,
                        precision = 3,
                        fraction_precision = 0,
                    }
                elseif value < 0 then
                    rctx:draw_string('---')
                else
                    rctx:draw_number{
                        value = value,
                        precision = 3,
                        fraction_precision = 2,
                    }
                end
            elseif disp_context.power == 2 then
                rctx:draw_string('8.8.8')
            end
        end
    },

    hdg = {
        rect = {255.16, 37.623, 104.264, 36.108}, value = -1,
        logical_size = {288.756, 100},
        renderer = function (rctx, value)
            rctx.font = segfont
            if disp_context.power == 1 then
                if value >= 0 then
                    rctx:draw_number{
                        value = value,
                        precision = 3,
                        fraction_precision = 0,
                        leading_zero = true,
                    }
                else
                    rctx:draw_string('---')
                end
            elseif disp_context.power == 2 then
                rctx:draw_string('8.8.8')
            end
        end
    },

    alt = {
        rect = {693, 37.623, 173.774, 36.108}, value = 0,
        logical_size = {481.262, 100},
        renderer = function (rctx, value)
            rctx.font = segfont
            if disp_context.power == 1 then
                rctx:draw_number{
                    value = value,
                    precision = 5,
                    fraction_precision = 0,
                    leading_zero = true,
                }
            elseif disp_context.power == 2 then
                rctx:draw_string('88888')
            end
        end
    },

    vs = {
        rect = {903.114, 37.623, 173.774, 36.108}, value = 0,
        logical_size = {481.262, 100},
        renderer = function (rctx, value)
            rctx.font = segfont
            if disp_context.power == 1 then
                if disp_context.nav_mode == 0 then
                    if disp_context.vs_mode == 1 then
                        rctx:draw_string('-----')
                    else
                        rctx:draw_string(string.format('%+03.0foo', disp_context.vs_fpa / 100))
                    end
                else
                    if disp_context.vs_mode == 1 then
                        rctx:draw_string('--.-')
                    else
                        rctx:draw_string(string.format('%+2.1f', disp_context.vs_fpa))
                    end
                end
            elseif disp_context.power == 2 then
                rctx:draw_string('+8.888')
            end
        end
    },

    spd_managed = {
        rect = {147.136, 43.677, 22, 22}, value = 0,
        renderer = function (rctx, value)
            if disp_context.power == 1 then
                if value == 1 or disp_context.power == 2 then
                    rctx:draw_bitmap(parts.managed)
                end
            end
        end
    },

    hdg_managed = {
        rect = {363.726, 43.677, 22, 22}, value = 0,
        renderer = function (rctx, value)
            if disp_context.power == 1 then
                if value == 1 or disp_context.power == 2 then
                    rctx:draw_bitmap(parts.managed)
                end
            end
        end
    },

    alt_managed = {
        rect = {869.891, 43.677, 22, 22}, value = 0,
        renderer = function (rctx, value)
            if disp_context.power == 1 then
                if value == 1 or disp_context.power == 2 then
                    rctx:draw_bitmap(parts.managed)
                end
            end
        end
    },
}

local canvases = {}
module.view_elements = {}

for name, def in pairs(canvas_defs) do
    param = {value = def.value, renderer = def.renderer}
    if def.logical_size then
        param.logical_width = def.logical_size[1]
        param.logical_height = def.logical_size[2]
    end
    canvases[name] = mapper.view_elements.canvas(param)
    module.view_elements[#module.view_elements + 1] = {
        object = canvases[name],
        x = def.rect[1], y = def.rect[2],
        width = def.rect[3], height = def.rect[4]
    }
end

--------------------------------------------------------------------------------------
-- Event-Action mappings
--------------------------------------------------------------------------------------
local mapping_defs = {
    power = {
        rpn = '(A:ELECTRICAL MAIN BUS VOLTAGE:1, Volts) 28 < (L:A32NX_OVHD_ELEC_EXT_PWR_PB_IS_ON) ! and if{ 0 } els{ (L:A32NX_OVHD_INTLT_ANN) if{ 1 } els{ 2 } }',
        action = function (evid, value)
            disp_context.power = value
            for name, canvas in pairs(canvases) do
                canvas:refresh()
            end
        end
    },

    nav_mode = {
        rpn = '(L:A32NX_TRK_FPA_MODE_ACTIVE)',
        action = function (evid, value)
            disp_context.nav_mode = value
            canvases.nav_mode1:refresh()
            canvases.nav_mode2:refresh()
            canvases.vs:refresh()
        end
    },

    vs_mode = {
        rpn = '(L:A32NX_FCU_AFS_DISPLAY_VS_FPA_DASHES)',
        action = function (evid, value)
            disp_context.vs_mode = value
            canvases.vs:refresh()
        end
    },

    vs_fpa = {
        rpn = '(L:A32NX_FCU_AFS_DISPLAY_VS_FPA_VALUE)',
        action = function (evid, value)
            disp_context.vs_fpa = value
            canvases.vs:refresh()
        end
    },

    spd_mode = {
        rpn = '(L:A32NX_FCU_AFS_DISPLAY_MACH_MODE)',
        action = canvases.spd_mode:value_setter()
    },

    spd = {
        rpn = '(L:A32NX_AUTOPILOT_SPEED_SELECTED)',
        action = canvases.spd:value_setter()
    },

    spd_managed = {
        rpn = '(L:A32NX_FCU_SPD_MANAGED_DOT)',
        action = canvases.spd_managed:value_setter()
    },

    hdg = {
        rpn = '(L:A32NX_FCU_HDG_MANAGED_DASHES) if{ -1 } els{ (L:A32NX_AUTOPILOT_HEADING_SELECTED) }',
        action = canvases.hdg:value_setter()
    },

    hdg_managed = {
        rpn = '(L:A32NX_FCU_HDG_MANAGED_DOT)',
        action = canvases.hdg_managed:value_setter()
    },

    alt = {
        rpn = '(A:AUTOPILOT ALTITUDE LOCK VAR:3, Feet)',
        action = canvases.alt:value_setter()
    },

    alt_managed = {
        rpn = '(L:A32NX_FCU_ALT_MANAGED)',
        action = canvases.alt_managed:value_setter()
    },
}

module.global_mappings = {}
module.observed_data = {}
for name, def in pairs(mapping_defs) do
    local evid = mapper.register_event('fcu:' .. name)
    module.global_mappings[#module.global_mappings + 1] = {event=evid, action=def.action}
    module.observed_data[#module.observed_data + 1] = {rpn=def.rpn, event=evid}
end

--------------------------------------------------------------------------------------
-- Base image rendering
--------------------------------------------------------------------------------------
function module.render_base_image(ctx)
    ctx.brush = graphics.color('black')
    ctx:fill_rectangle(0, 0, disp_width, disp_height)
end

--------------------------------------------------------------------------------------
-- Return the module object
--------------------------------------------------------------------------------------
return module
