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
        power_rpn = "(A:ADF VOLUME:1, Percent Over 100) 0 >",
    },
}

local common = require("lib/common")
--------------------------------------------------------------------------------------
-- virtual adf implementation
--------------------------------------------------------------------------------------
local adf = {
    {power=0, freq_10khz=true, active=0, standby=0},
}

local function adf_update_canvases(self, name, value)
    for i, canvas in pairs(self.canvases[name]) do
        canvas.value = value
    end
end
adf[1].update_canvases = adf_update_canvases

local function adf_set_value(self, name, value)
    if value > 0x1000000 then
        self[name] = value
        self:update_canvases(name, value * self.power)
    end
end
adf[1].set_value = adf_set_value

local function adf_update(self)
    self:set_value("active", self.active)
    self:set_value("standby", self.standby)
    self:update_canvases("adf", self.power)
    self:update_canvases("frq", self.power)
end
adf[1].update = adf_update

local function adf_set_active(self, value)
    value = math.floor(value)
    if self.active ~= value then
        self:set_value("standby", self.active)
        self:set_value("active", value)
    end
end
adf[1].set_active = adf_set_active

local function adf_set_standby(self, value)
    value = math.floor(value)
    self:set_value("standby", value)
end
adf[1].set_standby = adf_set_standby

local function adf_swap(self)
    local new_active = self.standby
    self.standby = self.active
    self.active = new_active
    self:update()
    fs2020.send_event("ADF_ACTIVE_SET", self.active)
    fs2020.send_event("ADF_STBY_SET", self.standby)
end
adf[1].swap = adf_swap

local function adf_large_knob_ops(self, delta)
    local hi_digit = (self.standby & 0xf0000000) >> 28
    local lo_digit = (self.standby & 0xf000000) >> 24
    lo_digit = lo_digit + delta
    if hi_digit == 0 and lo_digit < 1 then
        hi_digit = 1
        lo_digit = 7
    elseif hi_digit > 0 and lo_digit < 0 then
        hi_digit = 0
        lo_digit = 9
    elseif hi_digit > 0 and lo_digit > 7 then
        hi_digit = 0
        lo_digit = 1
    elseif hi_digit == 0 and lo_digit > 9 then
        hi_digit = 1
        lo_digit = 0
    end
    self:set_standby(self.standby & 0x00ff0000 | (hi_digit << 28) | (lo_digit << 24))
end
adf[1].large_knob_ops = adf_large_knob_ops

local function adf_small_knob_ops(self, delta)
    if self.freq_10khz then
        local digit = (self.standby & 0xf00000) + delta * 0x100000
        if digit < 0 then
            digit = 0x900000
        elseif digit > 0x900000 then
            digit = 0
        end
        self:set_standby(self.standby & 0xff0f0000 | digit)
    else
        local digit = (self.standby & 0xf0000) + delta * 0x10000
        if digit < 0 then
            digit = 0x90000
        elseif digit > 0x90000 then
            digit = 0
        end
        self:set_standby(self.standby & 0xfff00000 | digit)
    end
end
adf[1].small_knob_ops = adf_small_knob_ops

module_defs.reactions = {}
module_defs.reactions[module.type.general] = {}
module_defs.reactions[module.type.general][1] = {
    adf_power = {rpn="%s", action=function (value) adf[1].power=value; adf[1]:update() end},
    adf_active = {rpn="(A:ADF ACTIVE FREQUENCY:1, Frequency ADF BCD32)", action=function (value) adf[1]:set_active(value) end},
    adf_standby = {rpn="(A:ADF STANDBY FREQUENCY:1, Frequency ADF BCD32)", action=function (value) adf[1]:set_standby(value) end},
}

--------------------------------------------------------------------------------------
-- action definitions
--------------------------------------------------------------------------------------
module.actions = {}
module.actions[1] = {
    frq=function () adf[1]:swap() end,
    knob_large_inc=function () adf[1]:large_knob_ops(1) end,
    knob_large_dec=function () adf[1]:large_knob_ops(-1) end,
    knob_small_inc=function () adf[1]:small_knob_ops(1) end,
    knob_small_dec=function () adf[1]:small_knob_ops(-1) end,
    knob_push=function () adf[1].freq_10khz = not adf[1].freq_10khz end,
}
--------------------------------------------------------------------------------------
-- operable are definitions
--------------------------------------------------------------------------------------
local attr_normal = {width=62.203, height=36.762, rratio=0.05}
module_defs.operables = {}
module_defs.operables[module.type.general] = {
    frq = {x=426.969, y=182.609, attr=attr_normal},
}

--------------------------------------------------------------------------------------
-- indicator definitions
--------------------------------------------------------------------------------------
local disp_color = graphics.color(248, 87, 43)
local label_base = common.change_bitmap_color(graphics.bitmap("assets/kr87.png"):create_partial_bitmap(1113, 0, 48, 210), disp_color)
local attr_label = {width=48, height=35}
local label_adf = label_base:create_partial_bitmap(0, 0, attr_label.width, attr_label.height)
local label_frq = label_base:create_partial_bitmap(0, 105, attr_label.width, attr_label.height)
local seg_font_attr = {width=37.192, height=47.807}
local segdisp = require("lib/segdisp")
local seg_font = segdisp.create_font{type=segdisp.seg7_type1, width=seg_font_attr.width, height=seg_font_attr.height, color=disp_color}
local attr_freq = {width=seg_font_attr.width * 4, height=seg_font_attr.height}
local function freq_renderer(rctx, value)
    if value > 0 then
        rctx.font = seg_font
        rctx:draw_string(string.format("%4x", (math.floor(value) & 0xffff0000) >> 16))
    end
end
module_defs.indicators ={}
module_defs.indicators[module.type.general] = {}
module_defs.indicators[module.type.general][1]= {
    adf = {x=87.952, y=99.764, attr=attr_label, bitmaps={nil, label_adf}, pin=adf[1]},
    frq = {x=389.948, y=99.764, attr=attr_label, bitmaps={nil, label_frq}, pin=adf[1]},
    active = {x=203.141, y=66.142, attr=attr_freq, renderer=freq_renderer, pin=adf[1]},
    standby = {x=472.933, y=66.142, attr=attr_freq, renderer=freq_renderer, pin=adf[1]},
}

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
        for name, reaction in pairs(module_defs.reactions[option.type][i]) do
            if reaction.rpn ~= nil then
                module.observed_data[#module.observed_data + 1] = {
                    rpn = string.format(reaction.rpn, option.power_rpn),
                    epsilon= reaction.epsilon,
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
    rctx.brush = graphics.color("black")
    rctx:fill_rectangle(x, y, module.width * scale, module.height * scale)
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
