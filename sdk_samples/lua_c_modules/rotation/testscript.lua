local s_radius = 200
local m_radius = 2000
local rpm = 20

---------------------------------------------------------------------------------
-- Loads the rotation Lua C module and creates an emitter object provided by 
-- the module. The emitter object emulates circular motion with a specified RPM
-- and diameter, and periodically generates events whose values are the coordinates
-- of the point moving along the circle.
---------------------------------------------------------------------------------
package.cpath = package.cpath .. ';' .. mapper.script_dir .. 'x64/Release/?.dll' ..
                                 ';' .. mapper.script_dir .. 'x64/Debug/?.dll'
local rotation = require('rotation')
local change_coordinates = mapper.register_event('change coordinates')
local emitter = rotation.emitter(change_coordinates, rpm, m_radius * 2)
emitter:start_cw()

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
-- path according to the coordinate values emitted by the emitter object.
---------------------------------------------------------------------------------
local circle = graphics.ellipse{x=s_radius, y=s_radius, radius_x=s_radius, radius_y=s_radius}
local circle_color = graphics.color("Yellow");
local messages = {'Tap to rotate counterclockwise', 'Tap to stop', 'Tap to rotate clockwise'}
local message_font = graphics.system_font{family_name = 'Segoe Print', height = 400}
local message_color = graphics.color("SkyBlue")
local is_moving = 1
local direction = 1
local canvas_size = m_radius * 2 + s_radius * 2
local canvas = mapper.view_elements.canvas{
    logical_width = canvas_size,
    logical_height = canvas_size,
    value = {x = 0, y = 0},
    renderer = function (ctx, value)
        ctx.brush = circle_color;
        ctx:fill_geometry{
            geometry = circle,
            x = value.x, y = value.y,
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
-- the user to control the emitter object's state (clockwise, counterclockwise,
-- stop).
---------------------------------------------------------------------------------
local tap = mapper.register_event("view:tapped")
local operable_area = mapper.view_elements.operable_area{
    event_tap = tap,
    reaction_color = graphics.color('Black', 0),
}

---------------------------------------------------------------------------------
-- Register a view composed of a Canvas and an OperableArea.
-- Within this view definition, actions are defined for both the events emitted
-- by the emitter object and the events triggered when the OperableArea is tapped.
---------------------------------------------------------------------------------
viewport:register_view{
    name = "test",
    background = graphics.color("DarkBlue", 0.6),
    elements = {
        {object = operable_area, x = 0, y = 0, width = 1, height = 1},
        {object = canvas, x = 0, y = 0, width = 1, height = 1},
    },
    mappings = {
        {event=change_coordinates, action=canvas:value_setter()},
        {event=tap, action=function ()
            if is_moving == 1 then
                is_moving = 0
                emitter:stop()
            else
                is_moving = 1
                direction = direction * -1
                if direction > 0 then
                    emitter:start_cw()
                else
                    emitter:start_ccw()
                end
            end
            canvas:refresh()
        end},
    },
}

mapper.start_viewports()
