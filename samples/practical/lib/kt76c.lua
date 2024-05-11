local module = {
    width = 1112,
    height = 282,
    actions = {},
    events = {},
    observed_data = {},
    global_mapping_sources = {},
}

local common = require("lib/common")
local segdisp = require("lib/segdisp")

--------------------------------------------------------------------------------------
-- action definitions
--------------------------------------------------------------------------------------
module.actions[1] = {
    idt = fs2020.mfwasm.rpn_executer("(>K:XPNDR_IDENT_ON)"),
    vfr = fs2020.mfwasm.rpn_executer("(>H:TransponderVFR)"),
    clr = fs2020.mfwasm.rpn_executer("(>H:TransponderCLR)"),
    num0 = fs2020.mfwasm.rpn_executer("(>H:Transponder0)"),
    num1 = fs2020.mfwasm.rpn_executer("(>H:Transponder1)"),
    num2 = fs2020.mfwasm.rpn_executer("(>H:Transponder2)"),
    num3 = fs2020.mfwasm.rpn_executer("(>H:Transponder3)"),
    num4 = fs2020.mfwasm.rpn_executer("(>H:Transponder4)"),
    num5 = fs2020.mfwasm.rpn_executer("(>H:Transponder5)"),
    num6 = fs2020.mfwasm.rpn_executer("(>H:Transponder6)"),
    num7 = fs2020.mfwasm.rpn_executer("(>H:Transponder7) (>H:Transponder6) (>H:Transponder5) (>H:Transponder4)"),
    mode_inc = fs2020.mfwasm.rpn_executer("(A:TRANSPONDER STATE:1, Enum) d 4 < if{ ++ (>A:TRANSPONDER STATE:1, Enum) }"),
    mode_dec = fs2020.mfwasm.rpn_executer("(A:TRANSPONDER STATE:1, Enum) d 0 > if{ -- (>A:TRANSPONDER STATE:1, Enum) }"),
}
--------------------------------------------------------------------------------------
-- operable area definitions
--------------------------------------------------------------------------------------
local attr_normal = {width=63.434, height=36.762, rratio=0.05}
local buttons = {
    idt = {x=42.043, y=56.615, attr=attr_normal},
    vfr = {x=1008.966, y=210.206, attr=attr_normal},
    clr = {x=866.978, y=210.206, attr=attr_normal, relay=-1},
    num0 = {x=40.101, y=207.865, attr=attr_normal, relay=0},
    num1 = {x=136.019, y=207.865, attr=attr_normal, relay=1},
    num2 = {x=231.938, y=207.865, attr=attr_normal, relay=2},
    num3 = {x=330.01, y=207.865, attr=attr_normal, relay=3},
    num4 = {x=428.083, y=207.865, attr=attr_normal, relay=4},
    num5 = {x=525.078, y=207.865, attr=attr_normal, relay=5},
    num6 = {x=622.074, y=207.865, attr=attr_normal, relay=6},
    num7 = {x=719.069, y=207.865, attr=attr_normal, relay=7},
}

for i = 1,#module.actions do
    module.events[i] = {}
    for name, button in pairs(buttons) do
        module.events[i][name] = mapper.register_event("KT-76C:" .. name .. "_tapped")
    end
    module.events[i].all = mapper.register_event("KT-76C: background_tapped")
end

--------------------------------------------------------------------------------------
-- indicator definitions
--------------------------------------------------------------------------------------
module.base_image = graphics.bitmap("assets/kt76c-indicators.png")

local display_images = {
    alt = module.base_image:create_partial_bitmap(621, 0, 39.116, 19.585),
    on = module.base_image:create_partial_bitmap(621, 29.85, 30.646, 20.266),
    sby = module.base_image:create_partial_bitmap(621, 58.075, 43.698,  20.266),
    fl = module.base_image:create_partial_bitmap(621, 85.415, 23.459, 19.585),
    r = module.base_image:create_partial_bitmap(651, 85.415, 14.161, 19.585),
}

local attr_mode = {width=124, height=105}
local mode_images = {}
do
    local width = attr_mode.width
    local height = attr_mode.height
    for i = 0, 4 do
        mode_images[i + 1] = module.base_image:create_partial_bitmap(width * i, 0, width, height)
    end
end

local indicators ={}
indicators[1]= {
    mode_indicator = {x=924.181, y=78.362, attr=attr_mode, bitmaps=mode_images},
    fl_indicator = {x=208.688, y=98.535, attr={width=23.459, height=19.585}, bitmaps={nil, display_images.fl}},
    alt_indicator = {x=411.952, y=64.617, attr={width=39.116, height=19.585}, bitmaps={nil, display_images.alt}},
    on_indicator = {x=480.519, y=64.27, attr={width=30.646, height=20.266}, bitmaps={nil, display_images.on}},
    sby_indicator = {x=476.706, y=96.119, attr={width=43.698, height=20.266}, bitmaps={nil, display_images.sby}},
    r_indicator = {x=555.776, y=64.617, attr={width=14.161, height=19.585}, bitmaps={nil, display_images.r}},
}

--------------------------------------------------------------------------------------
-- segment display definitions
--------------------------------------------------------------------------------------
local seg_font_attr = {width=31.552, height=40.557}
local seg_font = segdisp.create_font{type=segdisp.seg7_type1, width=seg_font_attr.width, height=seg_font_attr.height, color=graphics.color(248, 87, 43)}

local seg_disps = {}
seg_disps[1] = {
    fl_disp = {x=206.071, y=66.133, width=seg_font_attr.width * 5, height=seg_font_attr.height},
    code_disp = {x=605.313, y=66.133, width=seg_font_attr.width * 4, height=seg_font_attr.height},
}

--------------------------------------------------------------------------------------
-- virtual transmitter implementation
--------------------------------------------------------------------------------------
local xmtr_ctx = {}
xmtr_ctx[1] = {
    id = 1,
    mode = 0,
    altitude = 0,
    code = 0,
    ident = 0,
    editing_code = "",
    in_edit_mode = false,
}

local xmtr_sources = {}
xmtr_sources[1] = {
    mode = {rpn="(A:TRANSPONDER STATE:1, Enum)", action=function(ctx, value) ctx:update_mode(value) end},
    altitude = {rpn="(A:INDICATED ALTITUDE CALIBRATED:1, Feet)", epsilon=10, action=function(ctx, value) ctx:update_altitude(value) end},
    code = {rpn="(A:TRANSPONDER CODE:1, BCO16)", action=function(ctx, value) ctx:update_code(value) end},
    ident = {rpn="(A:TRANSPONDER IDENT:1, Bool)", action=function(ctx, value) ctx:update_ident(value) end},
}

local function update_canvases(self, name, value)
    for i, canvas in ipairs(self.canvases[name]) do
        canvas:set_value(value)
    end
end
xmtr_ctx[1].update_canvases = update_canvases

local function update_mode(self, value)
    self.mode = value
    self.editing_code = ""
    self.in_edit_mode = false
    self:reflect()
    self:update_canvases("mode_indicator", value)
end
xmtr_ctx[1].update_mode = update_mode

local function update_altitude(self, value)
    self.altitude = value
    if self.mode == 4 then
        self:reflect()
    end
end
xmtr_ctx[1].update_altitude = update_altitude

local function update_code(self, value)
    self.code = value
    self.editing_code = ""
    self.in_edit_mode = false
    self:reflect()
end
xmtr_ctx[1].update_code = update_code

local function update_ident(self, value)
    self.ident = value
    if self.mode > 2 then
        self:reflect()
    end
end
xmtr_ctx[1].update_ident = update_ident

local function relay(self, value)
    if value < 0 then
        if self.in_edit_mode then
            self.editing_code = ""
        else
            fs2020.mfwasm.execute_rpn("(>H:TransponderCLR)")
            return
        end
    elseif #self.editing_code < 4 then
        self.in_edit_mode = true
        self.editing_code = self.editing_code .. value
    end
    self:reflect()
    if #self.editing_code >= 4 then
        local freq = tonumber(self.editing_code:sub(1,1)) * 0x1000 + tonumber(self.editing_code:sub(2,2)) * 0x0100 + tonumber(self.editing_code:sub(3,3)) * 0x0010 + tonumber(self.editing_code:sub(4,4))
        fs2020.send_event("XPNDR_SET", freq)
    end
end
xmtr_ctx[1].relay = relay

local function reflect(self)
    if self.mode == 2 then
        self:update_canvases("alt_indicator", 1)
        self:update_canvases("on_indicator", 1)
        self:update_canvases("sby_indicator", 1)
        self:update_canvases("r_indicator", 1)
        self:update_canvases("fl_indicator", 1)
        self:update_canvases("fl_disp", "- 888")
        self:update_canvases("code_disp", "8888")
    elseif self.mode == 0 then
        self:update_canvases("alt_indicator", 0)
        self:update_canvases("on_indicator", 0)
        self:update_canvases("sby_indicator", 0)
        self:update_canvases("r_indicator", 0)
        self:update_canvases("fl_indicator", 0)
        self:update_canvases("fl_disp", "")
        self:update_canvases("code_disp", "")
    else
        local fl = 0
        local alt = 0
        local on = 0
        local sby = 0
        local fl_str = ""
        if self.mode == 4 then
            alt = 1
            fl = 1
            fl_str = string.format(" %.3d", math.floor((self.altitude + 50) / 100))
            if #fl_str == 4 then
                fl_str = " " .. fl_str
            end
        elseif self.mode == 3 then
            on =1
        elseif self.mode == 1 then
            sby = 1
        end
        self:update_canvases("alt_indicator", alt)
        self:update_canvases("on_indicator", on)
        self:update_canvases("sby_indicator", sby)
        self:update_canvases("r_indicator", self.ident)
        self:update_canvases("fl_indicator", fl)
        self:update_canvases("fl_disp", fl_str)
        if self.in_edit_mode then
            local code_str = self.editing_code
            for i=#self.editing_code + 1, 4 do
                code_str = code_str .. "-"
            end
            self:update_canvases("code_disp", code_str)
        else
            self:update_canvases("code_disp", string.format("%.4x", math.floor(self.code)))
        end
    end
end
xmtr_ctx[1].reflect = reflect

for i, val in ipairs(xmtr_sources) do
    module.global_mapping_sources[i] = {}
    for name, source in pairs(val) do
        local evid = mapper.register_event("KT-76C:"..name.."_source")
        module.events[i][name] = evid
        module.observed_data[#module.observed_data + 1] = {rpn=source.rpn, epsilon=source.epsilon, event=evid}
    end
end

--------------------------------------------------------------------------------------
-- module destructor (GC handler)
--------------------------------------------------------------------------------------
setmetatable(module, {
    __gc = function (obj)
        for i = 1,#module.actions do
            for key, evid in pairs(obj.events[i]) do
                mapper.unregister_message(evid)
            end
        end
    end
})

--------------------------------------------------------------------------------------
-- reset function called when aircraft evironment is build each
--------------------------------------------------------------------------------------
function module.reset()
    for i, value in ipairs(module.global_mapping_sources) do
        module.global_mapping_sources[i] = {}
        xmtr_ctx[i].canvases = {}
        xmtr_ctx[i].in_edit_mode = false
        xmtr_ctx[i].editing_code = ""
    end
end

--------------------------------------------------------------------------------------
-- instance generator
--------------------------------------------------------------------------------------
function module.create_component(component_name, id, captured_window, x, y, scale, rctx, simhid_g1000)
    local component = {
        name = component_name,
        view_elements = {},
        view_mappings = {},
        component_mappings = {},
        callback = nil,
    }

    -- update view background bitmap
    local background = graphics.bitmap("assets/kt76c.png")
    rctx:draw_bitmap{bitmap=background, x=x, y=y, scale=scale}

    -- operable area
    local function notify_tapped()
        if component.callback then
            component.callback(component_name)
        end
    end
    for name, button in pairs(buttons) do
        local action = module.actions[id][name]
        if button.relay ~= nil then
            action = function ()
                xmtr_ctx[id]:relay(button.relay)
            end
        end
        component.view_elements[#component.view_elements + 1] = {
            object = mapper.view_elements.operable_area{event_tap = module.events[id][name], round_ratio=button.attr.rratio},
            x = x + button.x * scale, y = y + button.y * scale,
            width = button.attr.width * scale, height = button.attr.height * scale
        }
        component.view_mappings[#component.view_mappings + 1] = {event=module.events[id][name], action=filter.duplicator(action, notify_tapped)}
    end
    component.view_elements[#component.view_elements + 1] = {
        object = mapper.view_elements.operable_area{event_tap = module.events[id].all, reaction_color=graphics.color(0, 0, 0, 0)},
        x = x, y = y,
        width = module.width * scale, height = module.height * scale
    }
    component.view_mappings[#component.view_mappings + 1] = {event=module.events[id].all, action=notify_tapped}

    -- indicators
    for name, indicator in pairs(indicators[id]) do
        local canvas = mapper.view_elements.canvas{
            logical_width = indicator.attr.width,
            logical_height = indicator.attr.height,
            value = 0,
            renderer = function (ctx, value)
                local image = indicator.bitmaps[value + 1]
                if image then
                    ctx:draw_bitmap(image, 0, 0)
                end
            end
        }
        component.view_elements[#component.view_elements + 1] = {
            object = canvas,
            x = x + indicator.x * scale, y = y + indicator.y * scale,
            width = indicator.attr.width * scale, height = indicator.attr.height * scale
        }
        if xmtr_ctx[id].canvases[name] == nil then
            xmtr_ctx[id].canvases[name] = {}
        end
        xmtr_ctx[id].canvases[name][#xmtr_ctx[id].canvases[name] + 1] = canvas
    end

    -- 7segment display
    for name, disp in pairs(seg_disps[id]) do
        local canvas = mapper.view_elements.canvas{
            logical_width = disp.width,
            logical_height = disp.height,
            value = "",
            renderer = function (ctx, value)
                if #value > 0 then
                    ctx:set_font(seg_font)
                    ctx:draw_string(value)
                end
            end
        }
        component.view_elements[#component.view_elements + 1] = {
            object = canvas,
            x = x + disp.x * scale, y = y + disp.y * scale,
            width = disp.width * scale, height = disp.height * scale
        }
        if xmtr_ctx[id].canvases[name] == nil then
            xmtr_ctx[id].canvases[name] = {}
        end
        xmtr_ctx[id].canvases[name][#xmtr_ctx[id].canvases[name] + 1] = canvas
    end

    -- event-action mappings to drive virtual transmitter
    for name, source in pairs(xmtr_sources[id]) do
        module.global_mapping_sources[id][name] = {function (value) source.action(xmtr_ctx[id], value) end}
    end

    -- activation indicator
    local canvas1 = mapper.view_elements.canvas{
        logical_width = 1,
        logical_height = 1,
        value = 0,
        renderer = function (rctx, value)
            if value > 0 then
                rctx:set_brush(common.active_indicator_color)
                rctx:fill_geometry{geometry = common.circle, x = 0, y = 0}
            end
        end
    }
    component.view_elements[#component.view_elements + 1] = {
        object = canvas1,
        x = x + 957.831 * scale, y = y + 102.31 * scale,
        width = 75 * scale, height = 75 * scale
    }
    function component.activate(state)
        canvas1:set_value(state)
    end

    -- Event-Action mappings which are enabled when the component is activated
    if simhid_g1000 then
        local g1000 = simhid_g1000.events
        component.component_mappings = {
            {event=g1000.EC9Y.increment, action=module.actions[id].mode_inc},
            {event=g1000.EC9Y.decrement, action=module.actions[id].mode_dec},
            {event=g1000.EC9X.increment, action=module.actions[id].mode_inc},
            {event=g1000.EC9X.decrement, action=module.actions[id].mode_dec},
        }
    end

    return component
end

--------------------------------------------------------------------------------------
-- global mappings generator
--------------------------------------------------------------------------------------
function module.create_global_mappings()
    local mappings = {}
    for i, source in ipairs(module.global_mapping_sources) do
        for key, actions in pairs(source) do
            mappings[#mappings + 1] = {event=module.events[i][key], action = function (evid, value)
                for num, action in pairs(actions) do
                    action(value)
                end
            end}
        end
    end
    return mappings
end

return module
