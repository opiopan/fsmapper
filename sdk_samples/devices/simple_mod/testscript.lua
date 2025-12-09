local s_radius = 200
local m_radius = 2000

---------------------------------------------------------------------------------
-- Open the rotation plugin device
-- The rotation device emulates circular motion with a specified RPM and diameter,
-- and periodically generates events whose values are the coordinates of the point
-- moving along the circle.
---------------------------------------------------------------------------------
local device = mapper.device{
    name = "plugin device",
    type = "rotation",
    identifier = {},
    options = {rpm=20, side_length=m_radius * 2},
}
local dev_events = device:get_events()
local dev_upstreams = device:get_upstream_ids()

---------------------------------------------------------------------------------
-- Define a viewport
-- The viewport occupies the upper-left 30% area of the primary display.
---------------------------------------------------------------------------------
local viewport = mapper.viewport{
    name = "demo",
    displayno = 1,
    x = 0, y = 0,
    width = 0.3, height = 0.3,
}

---------------------------------------------------------------------------------
-- Create a Canvas view element that displays a small circle moving in a circular
-- path according to the coordinate values raised by the rotation device.
---------------------------------------------------------------------------------
local circle = graphics.ellipse{x=s_radius, y=s_radius, radius_x=s_radius, radius_y=s_radius}
local circle_color = graphics.color("Yellow");
local messages = {'Tap to rotate counterclockwise', 'Tap to stop', 'Tap to rotate clockwise'}
local message_font = graphics.system_font{family_name = 'Segoe Print', height = 400}
local message_color = graphics.color("SkyBlue")
local is_moving = 1
local direction = 1
canvas_size = m_radius * 2 + s_radius * 2
local canvas = mapper.view_elements.canvas{
    logical_width = canvas_size,
    logical_height = canvas_size,
    value = {0, 0},
    renderer = function (ctx, value)
        ctx.brush = circle_color;
        ctx:fill_geometry{
            geometry = circle,
            x = value[1], y = value[2],
        }
        ctx.font = message_font
        ctx.brush = message_color
        ctx:draw_string{
            string = messages[(is_moving - 1) * direction +2],
            x = 0, y =0,
            width = canvas_size, height = canvas_size,
            horizontal_alignment = 'center',
            vertical_alignment = 'center',
        }
    end
}

---------------------------------------------------------------------------------
-- Create an OperableArea view element that covers the entire view, allowing
-- the user to control the rotation device's state (clockwise, counterclockwise,
-- stop).
---------------------------------------------------------------------------------
local tap = mapper.register_event("view:tapped")
local operable_area = mapper.view_elements.operable_area{
    event_tap = tap,
    reaction_color = graphics.color('Black', 0),
}

---------------------------------------------------------------------------------
-- Register a view composed of a Canvas and an OperableArea.
-- Within this view definition, actions are defined for both the events raised
-- by the rotation device and the events triggered when the OperableArea is tapped.
---------------------------------------------------------------------------------
local point = {0, 0}
viewport:register_view{
    name = "test",
    background = graphics.color("DarkBlue", 0.6),
    elements = {
        {object = operable_area, x = 0, y = 0, width = 1, height = 1},
        {object = canvas, x = 0, y = 0, width = 1, height = 1},
    },
    mappings = {
        {event=dev_events.x.change, action=function (evid, value)
            point[1] = value
            canvas.value = point
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
            canvas:refresh()
            device:send(dev_upstreams.mode, direction * is_moving)
        end},
    },
}

mapper.start_viewports()
