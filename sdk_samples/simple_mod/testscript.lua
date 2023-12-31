local s_radius = 200
local m_radius = 2000

local device = mapper.device{
    name = "plugin device",
    type = "rotation",
    identifier = {},
    options = {rpm=20, side_length=m_radius * 2},
}
local dev_events = device:get_events()
local dev_upstreams = device:get_upstream_ids()

local viewport = mapper.viewport{
    name = "demo",
    displayno = 1,
    bg_color = "Black",
    x = 0, y = 0,
    width = 0.3, height = 0.3,
}

local sphere = graphics.ellipse{x=s_radius, y=s_radius, radius_x=s_radius, radius_y=s_radius}
local canvas = mapper.view_elements.canvas{
    logical_width = m_radius * 2 + s_radius * 2,
    logical_height = m_radius * 2 + s_radius * 2,
    value = {0, 0},
    renderer = function (ctx, value)
        ctx:set_brush(graphics.color("Yellow"))
        ctx:fill_geometry{
            geometry = sphere,
            x = value[1], y = value[2],
        }
    end
}

local tap = mapper.register_event("view:tapped")
local operable_area = mapper.view_elements.operable_area{
    event_tap = tap,
}

local point = {0, 0}
local is_moving = 1
local direction = 1
viewport:register_view{
    name = "test",
    logical_width = 1, logical_height = 1,
    background = graphics.color("DarkBlue"),
    elements = {
        {object = operable_area, x = 0, y = 0, width = 1, heigt = 1},
        {object = canvas, x = 0, y = 0, width = 1, height = 1},
    },
    mappings = {
        {event=dev_events.x.change, action=function (evid, value)
            point[1] = value
        end},
        {event=dev_events.y.change, action=function (evid, value)
            point[2] = value
            canvas.value = point
        end},
        {event=tap, action=function ()
            if is_moving == 1 then
                is_moving = 0
            else
                is_moving = 1
                direction = direction * -1
            end
            device:send(dev_upstreams.mode, direction * is_moving)
        end},
    },
}

mapper.start_viewports()
