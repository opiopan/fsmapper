local module = {
    active_indicator_color = graphics.color(0, 254, 255, 0.36),
    view_background_color = graphics.color(50, 50, 50),
}

--------------------------------------------------------------------------------------
-- graphical assets
--------------------------------------------------------------------------------------
module.circle = graphics.path()
module.circle:add_figure{
    fill_mode = "winding", -- none | winding | alternate (default: none)
    from = {0, 0.5},
    segments = {
        {to = {0.5, 0}, radius = 0.5, direction="clockwise", arc_type="small"},
        {to = {1, 0.5}, radius = 0.5, direction="clockwise", arc_type="small"},
        {to = {0.5, 1}, radius = 0.5, direction="clockwise", arc_type="small"},
        {to = {0, 0.5}, radius = 0.5, direction="clockwise", arc_type="small"},
    }
}

--------------------------------------------------------------------------------------
-- utility functions
--------------------------------------------------------------------------------------
function module.merge_array(target, source)
    for i, value in ipairs(source) do
        target[#target + 1] = value
    end
    return target
end

function module.merge_table(target, source)
    for key, value in pairs(source) do
        target[key] = value
    end
    return target
end

function module.get_table_size(table)
    local num = 0
    for k, v in pairs(table) do num = num + 1 end
    return num
end

--------------------------------------------------------------------------------------
-- generating another color version of bitmap
--------------------------------------------------------------------------------------
function module.change_bitmap_color(bitmap, color)
    local new_bitmap = graphics.bitmap(math.ceil(bitmap.width), math.ceil(bitmap.height))
    local brush = graphics.bitmap(new_bitmap.width, new_bitmap.height)
    local rect = graphics.rectangle(0, 0, new_bitmap.width, new_bitmap.height)
    local rctx = graphics.rendering_context(brush)
    rctx.brush = color
    rctx:fill_geometry{geometry=rect, x=0, y=0}
    rctx:finish_rendering()
    rctx=graphics.rendering_context(new_bitmap)
    rctx.brush = brush
    rctx.opacity_mask = bitmap
    rctx:fill_geometry{geometry=rect, x=0, y=0}
    rctx:finish_rendering()
    return new_bitmap
end

--------------------------------------------------------------------------------------
-- handling component based view
--------------------------------------------------------------------------------------
function module.init_component_modules(libs, lib_options)
    for name, lib in pairs(libs) do
        if lib.reset ~= nil then
            if lib_options ~= nil then
                lib.reset(lib_options[name])
            else
                lib.reset()
            end
        end
        if lib.observed_data ~= nil then
            fs2020.mfwasm.add_observed_data(lib.observed_data)
        end
    end
end

function module.change_viewport_mappings(viewport, viewport_mappings, view)
    mapper.delay(1, function ()
        viewport:set_mappings(viewport_mappings)
        if view.active_component ~= nil then
            viewport:add_mappings(view.components[view.active_component].instance.component_mappings)
        end
    end)
end

function module.create_default_view_changer(viewport, views, initial_view, viewport_mappings, device, additional_viewport_mappings)
    local current_view = initial_view
    local function change_view(d)
        current_view = current_view + d
        if current_view > #views then
            current_view = 1
        elseif current_view < 1 then
            current_view = #views
        end
        viewport:change_view(views[current_view].viewid)
        module.change_viewport_mappings(viewport, viewport_mappings, views[current_view])
    end

    local g1000 = device.events
    module.merge_array(viewport_mappings, {
        {event=g1000.AUX1D.down, action=function () change_view(1) end},
        {event=g1000.AUX1U.down, action=function () change_view(-1) end},
        {event=g1000.AUX2D.down, action=function () change_view(1) end},
        {event=g1000.AUX2U.down, action=function () change_view(-1) end},
    })

    module.merge_array(viewport_mappings, additional_viewport_mappings)

    return {
        move_next_view = function () change_view(1) end,
        move_previous_view = function () change_view(-1) end,
    }
end

function module.arrange_views(viewport, viewport_mappings, captured_window_defs, views, device)
    local captured_windows ={}
    for i, def in ipairs(captured_window_defs) do
        captured_windows[def.key] ={
            object = mapper.view_elements.captured_window{name=def.name}
        }
    end

    for viewix, view in ipairs(views) do
        view.active_component = view.initial_active_component
        local background = graphics.bitmap(view.width, view.height)
        local rctx = graphics.rendering_context(background)
        local view_elements = {}
        local view_mappings = {}
        if view.mappings ~= nil then
            module.merge_array(view_mappings, view.mappings)
        end
        local background_color = module.view_background_color
        if view.background_color then
            background_color = view.background_color
        end
        if view.background_regions ~= nil then
            rctx:set_brush(background_color)
            for i, rect in ipairs(view.background_regions) do
                rctx:fill_rectangle(rect.x, rect.y, rect.width, rect.height)    
            end
        end
        local change_active_component = function (cid)
            if view.active_component ~= cid then
                view.components[view.active_component].instance.activate(0)
                view.active_component = cid
                view.components[view.active_component].instance.activate(1)
                viewport:set_mappings(viewport_mappings)
                viewport:add_mappings(view.components[view.active_component].instance.component_mappings)
            end
        end
        for i, component in ipairs(view.components) do
            local cw = nil
            if component.cw ~= nil then
                cw = captured_windows[component.cw].object
            end
            component.instance = component.module.create_component(
                i, component.type_id, cw,
                component.x, component.y, component.scale,
                rctx, device, component.options
            )
            module.merge_array(view_elements, component.instance.view_elements)
            module.merge_array(view_mappings, component.instance.view_mappings)
            component.instance.callback = change_active_component
            if component.instance.viewport_mappings ~= nil then
                module.merge_array(viewport_mappings, component.instance.viewport_mappings)
            end
        end
        rctx:finish_rendering()
        view.viewid = viewport:register_view{
            name = view.name,
            elements = view_elements,
            logical_width = view.width,
            logical_height = view.height,
            background = background,
            mappings = view_mappings,
        }
        if view.active_component ~= nil then
            view.components[view.active_component].instance.activate(1)
        end
    end
end

function module.set_global_mappings(global_mappings, libs)
    for name, lib in pairs(libs) do
        if lib.create_global_mappings ~= nil then
            global_mappings[#global_mappings + 1] = lib.create_global_mappings()
        end
    end
end

function module.clear_component_instance(views)
    for i, view in ipairs(views) do
        view.viewid = nil
        for j, component in ipairs(view.components) do
            component.instance = nil
        end
    end
    fs2020.mfwasm.clear_observed_data()
end

--------------------------------------------------------------------------------------
-- support functions to realize the component
--------------------------------------------------------------------------------------
function module.component_module_init(cmodule, cmodule_defs)
    cmodule.events = {}
    for i = 1,#cmodule.actions do
        cmodule.events[i] = {}
        if cmodule_defs.operables ~= nil then
            for typeid, operables in ipairs(cmodule_defs.operables) do
                for name, operable in pairs(operables) do
                    if cmodule.events[i][name] == nil then
                        cmodule.events[i][name] = mapper.register_event(cmodule_defs.prefix .. ":" .. name .. "_tapped")
                    end
                end
            end
        end
        if cmodule_defs.activatable then
            cmodule.events[i].all = mapper.register_event(cmodule_defs.prefix..": background_tapped")
        end
    end

    cmodule.global_mapping_sources = {}
    cmodule.observed_data = {}
    if cmodule_defs.indicators ~= nil then
        for type, typedef in ipairs(cmodule_defs.indicators) do
            for i, indicatordefs in ipairs(typedef) do
                cmodule.global_mapping_sources[i] = {}
                for name, indicator in pairs(indicatordefs) do
                    if cmodule.events[i][name] == nil and indicator.rpn ~= nil then
                        local evid = mapper.register_event(cmodule_defs.prefix..":"..i..":"..name)
                        cmodule.events[i][name] = evid
                        cmodule.observed_data[#cmodule.observed_data + 1] = {rpn=indicator.rpn, event=evid, epsilon = indicator.epsilon}
                    end
                end
            end
        end
    end

    if cmodule_defs.indicators ~= nil and cmodule_defs.indicator_orders == nil then
        cmodule_defs.indicator_orders = {}
        for typeid, indicators in ipairs(cmodule_defs.indicators) do
            cmodule_defs.indicator_orders[typeid] = {}
            for key, value in pairs(indicators[1]) do
                cmodule_defs.indicator_orders[typeid][#cmodule_defs.indicator_orders[typeid] + 1] = key
            end
        end
    end

    --
    -- reset function called when aircraft evironment is build each
    --
    cmodule.reset = function (options)
        for i, data in ipairs(cmodule_defs.options) do
            cmodule_defs.options[i] = module.merge_table({}, cmodule_defs.option_defaults)
        end
        if options ~= nil then
            for i, option in ipairs(options) do
                for key, value in pairs(option) do
                    cmodule_defs.options[i][key] = value
                end
            end
        end
    
        for i, value in ipairs(cmodule.global_mapping_sources) do
            cmodule.global_mapping_sources[i] = {}
        end
    end

    --
    -- module destructor (GC handler)
    --
    setmetatable(cmodule, {
        __gc = function (obj)
            for i = 1,#cmodule.actions do
                for key, evid in pairs(obj.events[i]) do
                    mapper.unregister_message(evid)
                end
            end
        end
    })

    --
    -- global mappings generator
    --
    cmodule.create_global_mappings = function()
        local mappings = {}
        for i, source in ipairs(cmodule.global_mapping_sources) do
            for key, actions in pairs(source) do
                mappings[#mappings + 1] = {event=cmodule.events[i][key], action = function (evid, value)
                    for num, action in pairs(actions) do
                        action(value)
                    end
                end}
            end
        end
        return mappings
    end
end

function module.component_module_create_instance(cmodule, cmodule_defs, instance_opts)
    local component_name = instance_opts.name
    local id = instance_opts.id
    local captured_window = instance_opts.captured_window
    local x = instance_opts.x
    local y = instance_opts.y
    local scale = instance_opts.scale
    local simhid_g1000 = instance_opts.simhid_g1000
    local option = cmodule_defs.options[id]

    local component = {
        name = component_name,
        view_elements = {},
        view_mappings = {},
        component_mappings = {},
        callback = nil,
    }

    -- operable area
    local function notify_tapped()
        if component.callback then
            component.callback(component_name)
        end
    end
    if cmodule_defs.operables ~= nil and cmodule_defs.operables[option.type] then
        for name, operable in pairs(cmodule_defs.operables[option.type]) do
            if operable.enable_condition == nil or operable.enable_condition(option) then
                component.view_elements[#component.view_elements + 1] = {
                    object = mapper.view_elements.operable_area{event_tap = cmodule.events[id][name], round_ratio=operable.attr.rratio},
                    x = x + operable.x * scale, y = y + operable.y * scale,
                    width = operable.attr.width * scale, height = operable.attr.height * scale
                }
                if cmodule_defs.activatable then
                    component.view_mappings[#component.view_mappings + 1] = {event=cmodule.events[id][name], action=filter.duplicator(cmodule.actions[id][name], notify_tapped)}
                else
                    component.view_mappings[#component.view_mappings + 1] = {event=cmodule.events[id][name], action=cmodule.actions[id][name]}
                end
            end
        end
    end
    if cmodule_defs.activatable then
        component.view_elements[#component.view_elements + 1] = {
            object = mapper.view_elements.operable_area{event_tap = cmodule.events[id].all, reaction_color=graphics.color(0, 0, 0, 0)},
            x = x, y = y,
            width = cmodule.width * scale, height = cmodule.height * scale
        }
        component.view_mappings[#component.view_mappings + 1] = {event=cmodule.events[id].all, action=notify_tapped}
    end

    -- activation indicator
    local active_indicators = {}
    if cmodule_defs.active_indicators ~= nil then
        for i, aindicator in ipairs(cmodule_defs.active_indicators[option.type]) do
            if aindicator.enable_condition == nil or aindicator.enable_condition(option) then
                local canvas = mapper.view_elements.canvas{
                    logical_width = 1,
                    logical_height = 1,
                    value = 0,
                    translucency = true;
                    renderer = function (rctx, value)
                        if value > 0 then
                            rctx:set_brush(module.active_indicator_color)
                            rctx:fill_geometry{geometry = module.circle, x = 0, y = 0}
                        end
                    end
                }
                component.view_elements[#component.view_elements + 1] = {
                    object = canvas,
                    x = x + aindicator.x * scale, y = y + aindicator.y * scale,
                    width = aindicator.width * scale, height = aindicator.height * scale
                }
                active_indicators[#active_indicators + 1] = canvas
            end
        end
    end
    function component.activate(state)
        for i, canvas in ipairs(active_indicators) do
            canvas:set_value(state)
        end
    end

    -- indicators
    if cmodule_defs.indicator_orders ~= nil and cmodule_defs.indicator_orders[option.type] then
        for i, name in ipairs(cmodule_defs.indicator_orders[option.type]) do
            local indicator = cmodule_defs.indicators[option.type][id][name]
            if indicator.enable_condition == nil or indicator.enable_condition(option) then
                local renderer = nil
                if indicator.bitmaps ~= nil then
                    renderer = function (ctx, value)
                        local image = indicator.bitmaps[value + 1]
                        if image then
                            ctx:draw_bitmap(image, 0, 0)
                        end
                    end
                elseif indicator.rotation ~= nil then
                    renderer = function (ctx, value)
                        ctx:draw_bitmap{bitmap=indicator.rotation.bitmap, x=indicator.rotation.center.x, y=indicator.rotation.center.y, angle=value}
                    end
                elseif indicator.shift ~= nil and indicator.shift.axis == "x" then
                    renderer = function (ctx, value)
                        ctx:draw_bitmap{bitmap=indicator.shift.bitmap, x=indicator.shift.scale * value, y=0}
                    end
                elseif indicator.shift ~= nil and indicator.shift.axis == "y" then
                    renderer = function (ctx, value)
                        ctx:draw_bitmap{bitmap=indicator.shift.bitmap, x=0, y=indicator.shift.scale * value}
                    end
                end

                local canvas = mapper.view_elements.canvas{
                    logical_width = indicator.attr.width,
                    logical_height = indicator.attr.height,
                    value = 0,
                    renderer = renderer
                }
                component.view_elements[#component.view_elements + 1] = {
                    object = canvas,
                    x = x + indicator.x * scale, y = y + indicator.y * scale,
                    width = indicator.attr.width * scale, height = indicator.attr.height * scale
                }
                if indicator.rpn ~= nil then
                    if cmodule.global_mapping_sources[id][name] == nil then
                        cmodule.global_mapping_sources[id][name] = {}
                    end
                    cmodule.global_mapping_sources[id][name][#cmodule.global_mapping_sources[id][name] + 1] = function (value) canvas:set_value(value) end
                end
            end
        end
    end

    -- captured window
    if cmodule_defs.captured_window ~= nil and cmodule_defs.captured_window[option.type] ~= nil then
        local def = cmodule_defs.captured_window[option.type]
        component.view_elements[#component.view_elements + 1] = {
            object = captured_window,
            x = x + def.x * scale, y = y + def.y * scale,
            width = def.width * scale, height = def.height * scale,
        }
    end

    return component
end

return module