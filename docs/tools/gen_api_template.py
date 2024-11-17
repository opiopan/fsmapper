#!/usr/bin/env python3

from pathlib import Path
import re

libs = [
    {
        'name': 'mapper',
        'description': 'fsmapper basic library',
        'variables': [
            ('mapper.script_path', 'Path of the configuration script'),
            ('mapper.script_dir', 'Folder path where the configuration script is located'),
            ('mapper.profile_dir', 'User profile folder path'),
            ('mapper.saved_games_dir', 'Saved games folder path'),
            ('mapper.events', 'System event table'),
        ],
        'functions': [
            ('mapper.print()', 'Print a message', 'message'),
            ('mapper.abort()', 'Abort processing'),
            ('mapper.delay()', 'Deferred function execution', 'rel_time, func'),
            ('mapper.register_event()', 'Register an event', 'name'),
            ('mapper.unregister_event()', 'Unregister an event', 'event_id'),
            ('mapper.get_event_name()', 'Get the name assigned to an event', 'event_id'),
            ('mapper.raise_event()', 'Raise an event', 'event_id, value'),
            ('mapper.set_primary_mappings()', 'Set primary Event-Action mapping definitions', 'mapping_defs'),
            ('mapper.add_primary_mappings()', 'Add primary Event-Action mapping definitions', 'mapping_defs'),
            ('mapper.set_secondary_mappings()', 'Set secondary Event-Action mapping definitions', 'mapping_defs'),
            ('mapper.add_secondary_mappings()', 'Add secondary Event-Action mapping definitions', 'mapping_defs'),
            ('mapper.device()', 'Open a device', 'param_table'),
            ('mapper.viewport()', "Register a viewport", 'param_table'),
            ('mapper.view_elements.operable_area()', 'Create a OperableArea view element object', 'param_table'),
            ('mapper.view_elements.canvas()', 'Create a Canvas view element object', 'param_table'),
            ('mapper.view_elements.captured_window()', 'Create a CapturedWindow view element object', 'param_table'),
            ('mapper.window_image_streamer()', 'Create a window image streamer object', 'param_table'),
            ('mapper.start_viewports()', 'Start all viewports'),
            ('mapper.stop_viewports()', 'Stop all viewports'),
            ('mapper.reset_viewports()', 'Stop all viewports then unregister all viewports'),
            ('mapper.virtual_joystick()', 'Create vJoy feeder object', 'devid'),
            ('mapper.keystroke()', 'Create Keystroke object for keyboard emulation', 'param_table'),
            ('mapper.enumerate_display_info()', 'Enumerate information for all displays'),
        ],
        'objects': [
            {
                'name': 'Device',
                'description': 'Object representing a device',
                'properties': [
                    ('Device.events', 'A table holding event IDs for each input unit'),
                    ('Device.upstream_ids', 'A table holding unit IDs for each output unit'),
                ],
                'methods': [
                    ('Device:get_events()', 'Return a table holding event IDs for each input unit'),
                    ('Device:get_upstream_ids()', 'Return a table holding unit IDs for each output unit'),
                    ('Device:close()', 'Close the device'),
                    ('Device:send()', 'Send a value to output unit', 'unit_id, value'),
                    ('Device:sender()', 'Create a native-action to send a value to output unit', 'unit_id[, value]'),
                ],
            },
            {
                'name': 'Viewport',
                'description': 'Object representing a viewport',
                'properties': [
                    ('Viewport.empty_view', 'View ID of the empty view'),
                    ('Viewport.current_view', 'View ID of the current view'),
                ],
                'methods': [
                    ('Viewport:register_view()', 'Register a view to the viewport', 'param_table'),
                    ('Viewport:change_view()', 'Change the current view', 'view_id'),
                    ('Viewport:set_mappings()', 'Set Event-Action mapping definitions for the viewport', 'mapping_defs'),
                    ('Viewport:add_mappings()', 'Add Event-Action mapping definitions for the viewport', 'mapping_defs'),
                ],
            },
            {
                'name': 'OperableArea',
                'description': 'Object representing a operable area on a view',
            },
            {
                'name': 'Canvas',
                'description': 'Object representing a drawable area on a view',
                'properties': [
                    ('Canvas.value', 'The value associated with the canvas'),
                ],
                'methods': [
                    ('Canvas:set_value()', 'Return the value associated with the canvas', 'value'),
                    ('Canvas:get_value()', 'Set a value for the canvas'),
                    ('Canvas:value_setter()', 'Create a native-action to set a value for the canvas', '[value]'),
                    ('Canvas:refresh()', 'Redraw the canvas area'),
                    ('Canvas:refresher()', 'Create a native-action to redraw the canvas area'),
                ],
            },
            {
                'name': 'CapturedWindow',
                'description': 'Object representing a captured window placed on a view',
            },
            {
                'name': 'WindowImageStreamer',
                'description': 'Object for capturing window images in real-time',
                'methods': [
                    ('WindowsImageStreamer:create_view_element()', 'Create a view element to show a portion of captured image', 'param_table'),
                ],
            },
            {
                'name': 'CapturedImage',
                'description': 'Object for displaying images captured by the WindowImageStreamer on the view',
            },
            {
                'name': 'vJoy',
                'description': 'Object representing a virtual joystick',
                'methods': [
                    ('vJoy:get_axis()', 'Return a vJoyUnit object corresponding to an analog axis', 'axis'),
                    ('vJoy:get_button()', 'Return a vJoyUnit object corresponding to a button', 'button_id'),
                    ('vJoy:get_pov()', 'Return a vJoyUnit object corresponding to a POV', 'pov_id'),
                ],
            },
            {
                'name': 'vJoyUnit',
                'description': 'Object representing an operable unit of the virtual joystick',
                'methods': [
                    ('vJoyUnit:set_value()', 'Set a value for the operable unit', 'value'),
                    ('vJoyUnit:value_setter()', 'Create a  native-actionn to set a value for the operable unit', '[value]'),
                ],
            },
            {
                'name': 'Keystroke',
                'description': 'Object Representing a keystroke sequence to emulate keyboard',
                'properties': [
                    ('Keystroke.duration', 'The duration of key press'),
                    ('Keystroke.interval', 'The interval between keystrokes'),
                ],
                'methods': [
                    ('Keystroke:synthesize()', 'Synthesize keystroke sequence held by this object'),
                    ('Keystroke:synthesizer()', 'Create a  native-action to synthesize keystroke sequence hold by this object'),
                ],
            },
        ],
        'callbacks': [
            ('ACTION()', 'Implementation of an action by user', 'event_id, value'),
            ('RENDER()', 'Implementation of a renderer for a Canvas view element by user', 'context, value'),
        ],
    },
    {
        'name': 'filter',
        'description': 'Native-actions capable of cascading with other actions',
        'functions': [
            ('filter.duplicator()', 'Create a native-action that replicates an event for multiple subsequent actions', 'action[, ...]'),
            ('filter.branch()', 'Create a native-action to implement conditional branching between multiple actions', 'cond-exp[, ...]'),
            ('filter.delay()', 'Create a native-action that delays the execution of action', 'rel_time, action'),
            ('filter.lerp()', 'Create a native-action to modify the characteristics curve of a device\'s analog axis', 'action, val_mappings'),
        ]
    },
    {
        'name': 'msfs',
        'description': 'Interactivity features with Microsoft Flight Simulator',
        'functions': [
            ('msfs.send_event()', 'Send a SimConnect client event', 'event_name[, param1[, param2[, param3[, param4[, param5]]]]'),
            ('msfs.event_sender()', 'Create a native-action to send a SimConnect client event','event_name[, param1[, param2[, param3[, param4[, param5]]]]'),
            ('msfs.add_observed_simvars()', 'Register SimVars for observing', 'param_table'),
            ('msfs.clear_observed_simvars()', 'Clear all observed SimVars'),
            ('msfs.execute_input_event()', 'Execute an InputEvent', 'input_event, value'),
            ('msfs.input_event_executer()', 'Create a native-action to execute an InputEvent', 'input_event[, value]'),
            ('msfs.mfwasm.execute_rpn()', 'Execute an RPN script within MSFS', 'rpn'),
            ('msfs.mfwasm.rpn_executer()', 'Create a native-action to execute an RPN script within MSFS', 'rpn'),
            ('msfs.mfwasm.add_observed_data()', 'Register MSFS internal data to be observed', 'def_array'),
            ('msfs.mfwasm.clear_observed_data()', 'Clear all observed MSFS internal data'),
        ],
    },
    {
        'name': 'dcs',
        'description': 'Interactivity features with DCS World',
        'functions': [
            ('dcs.perform_clickable_action()', 'Set the position of clickable control in the cockpit', 'device_id, command, value[, value, ...]'),
            ('dcs.clickable_action_performer()', 'Create a native-action to set the position of clickable control in the cockpit', 'device_id, command[, value, ...]'),
            ('dcs.register_chunk()', 'Register a Lua chunk to be executed within the DCS World process', 'chunk'),
            ('dcs.clear_chunk()', 'Clear all Lua chunks registered for execution within the DCS World process'),
            ('dcs.execute_chunk()', 'Execute a registered chunk within the DCS World process', 'chunk-id[, argument]'),
            ('dcs.chunk_executer()', 'Create a native-action to execute a registered chunk within the DCS World process'),
            ('dcs.add_observed_data()', 'Register cockpit data to be observed', 'def_array'),
            ('dcs.clear_observed_data()', 'Clear all registered cockpit data to be observed'),
        ],
    },
    {
        'name': 'graphics',
        'description': 'Graphics library',
        'functions': [
            ('graphics.color()', 'Create a solid color brush object', 'color_name[, alpha]', 'r, g, b[, alpha]'),
            ('graphics.rectangle()', 'Create a SimpleGeometry object as a rectangle', 'param_table', 'x, y, width, height'),
            ('graphics.rounded_rectangle()', 'Create a SimpleGeometry object as a rounded rectangle', 'param_table', 'x, y, width, height, radius_x, radius_y'),
            ('graphics.ellipse()', 'Create a SimpleGeometry object as a ellipse', 'param_table', 'x, y, radius_x, radius_y'),
            ('graphics.path()', 'Create a geometry object defined as a path', '[figure_def]'),
            ('graphics.bitmap()', 'Create a bitmap object', 'path', 'width, height'),
            ('graphics.system_font()', 'Create a system font object', 'param_table'),
            ('graphics.bitmap_font()', 'Create a bitmap font object'),
            ('graphics.rendering_context()', 'Create a rendering context associated with a bitmap', 'bitmap'),
        ],
         'objects': [
            {
                'name': 'RenderingContext',
                'description': 'Object representing a rendering context',
                'properties': [
                    ('RenderingContext.brush', 'Current brush'),
                    ('RenderingContext.font', 'Current font'),
                    ('RenderingContext.stroke_width', 'Current stroke width'),
                    ('RenderingContext.opacity_mask', 'Bitmap used as opacity mask'),
                ],
                'methods': [
                    ('RenderingContext:finish_rendering()', 'Finalize the drawing process'),
                    ('RenderingContext:set_brush()', 'Set a brush to the context', 'brush'),
                    ('RenderingContext:set_font()', 'Set a font to the context', 'font'),
                    ('RenderingContext:set_stroke_width()', 'Set a stroke width to the context', 'width'),
                    ('RenderingContext:set_opacity_mask()', 'Set a bitmap used as opacity mask to the context', 'bitmap'),
                    ('RenderingContext:draw_geometry()', 'Draw a geometry', 'param_table', 'geometry, x, y[, angle[, scale]]'),
                    ('RenderingContext:fill_geometry()', 'Fill a geometry', 'param_table', 'geometry, x, y[, angle[, scale]]'),
                    ('RenderingContext:draw_bitmap()', 'Draw a bitmap', 'param_table', 'bitmap[, x, y[, width, height[, angle[, scale]]]'),
                    ('RenderingContext:draw_string()', 'Draw a string', 'param_table', 'string[, x, y]'),
                    ('RenderingContext:draw_number()', 'Draw a formated string of a numeric value', 'param_table', 'value[, x, y]'),
                    ('RenderingContext:fill_rectangle()', 'Fill a rectangle', 'x, y, width, height'),
                ],
            },
            {
                'name': 'Color',
                'description': 'Object representing a solid color brush',
            },
            {
                'name': 'SimpleGeometry',
                'description': 'Object representing a predefined simple geometry',
                'methods': [
                    ('SimpleGeometry:set_origin()', 'set new origin of the path', 'x, y'),
                ],
            },
            {
                'name': 'Path',
                'description': 'Object representing a geometry defined as a path',
                'methods': [
                    ('Path:add_figure()', 'Add a figure to the path', 'param_table'),
                    ('Path:fix()', 'Finalize the path to make it drawable'),
                    ('Path:set_origin()', 'set new origin of the path', 'x, y'),
                ],
            },
            {
                'name': 'Bitmap',
                'description': 'Object representing a bitmap',
                'properties': [
                    ('Bitmap.width', 'Width of the bitmap'),
                    ('Bitmap.height', 'Height of the bitmap'),
                    ('Bitmap.opacity', 'Opacity of the bitmap'),
                    ('Bitmap.brush_extend_mode_x', 'X axis extend mode for brush usage'),
                    ('Bitmap.brush_extend_mode_y', 'Y axis extend mode for brush usage'),
                    ('Bitmap.brush_interpolation_mode', 'Interpolation mode for brush usage'),
                ],
                'methods': [
                    ('Bitmap:set_origin()', 'set new origin of the bitmap', 'x, y'),
                    ('Bitmap:create_partial_bitmap()', 'create a bitmap corresponding to a partial area of the original bitmap', 'x, y, width, height'),
                ],
            },
            {
                'name': 'SystemFont',
                'description': 'Object representing a system font',
            },
            {
                'name': 'BitmapFont',
                'description': 'Object representing a bitmap based font',
                'methods': [
                    ('BitmapFont:add_glyph()', 'Add a glyph correnspoinding to a code point', 'param_table', 'code_point, bitmap'),
                ],
            },
         ],
   }
]

def symbol_to_filename(s):
    name = s.replace('.', '_').replace(':', '-').replace('(', '').replace(')', '')
    return re.sub('^_*', '', name)

def lua_function_defstr(funcdef):
    funcname = re.sub('\\(\\)', '', funcdef[0])
    defstr = ''
    sep = ''
    for args in funcdef[2:]:
        defstr = defstr + f'{sep}{funcname}({args})'
        sep = '\n'
    return defstr

def list_args(argdefs):
    for argdef in argdefs:
        args = re.sub('[\\[\\] ]', '', argdef).split(',')
        for arg in args:
            yield arg

def gen_group(group:dict, pos:int, base:Path, suffix:str, prefix):
    name = group['name']
    title = name + ' ' + suffix
    description = group['description']
    dir = base / symbol_to_filename(name)
    linkbase = prefix + symbol_to_filename(name) + '/'
    dir.mkdir(exist_ok = True)
    titles = [
        ('variables', 'Variables'),
        ('functions', 'Functions'),
        ('properties', 'Properties'),
        ('methods', 'Methods'),
        ('objects', 'Objects'),
        ('callbacks', 'User Defined Functions'),
    ]
    
    # generate _category_.json
    category = dir/'_category_.json'
    index_id = name + '_index'
    with category.open(mode='w') as f:
        data = \
            f'{{\n' \
            f'  "label": "{title}",\n' \
            f'  "position": {pos},\n' \
            f'  "link": {{"type": "doc", "id": "{index_id}"}}\n' \
            f'}}\n'
        f.write(data)
    
    # generate index.md
    index_file = dir / 'index.md'
    if not index_file.exists():
        with index_file.open(mode='w') as f:
            data = \
                f'---\n'\
                f'sidebar_position: 1\n'\
                f'id: {index_id}\n'\
                f'---\n\n'\
                f'# {title}\n'\
                f'{description}.\n'
            f.write(data)
            if suffix == 'object':
                f.write('\n## Constructors\n|Constructor|\n|---|\n|\n')
            for title in titles:
                if title[0] in group:
                    f.write(f'\n## {title[1]}\n|Name|Description|\n|-|-|\n')
                    for item in group[title[0]]:
                        if isinstance(item, dict):
                            filename = symbol_to_filename(item['name'])
                            itemname = item['name']
                            itemdesc = item['description']
                            f.write(f'|[```{itemname}```]({linkbase}{filename})|{itemdesc}|\n')
                        else:
                            filename = symbol_to_filename(item[0])
                            f.write(f'|[```{item[0]}```]({linkbase}{filename})|{item[1]}|\n')

    # generate child files
    cpos = 0
    for title in titles:
        if title[0] in group:
            for item in group[title[0]]:
                cpos = cpos + 1
                if isinstance(item, dict):
                    gen_group(item, cpos, dir, 'object', linkbase)
                else:
                    filename = symbol_to_filename(item[0]) + '.md'
                    filepath = dir / filename
                    if not filepath.exists():
                        luadef = item[0] if len(item) < 3 else lua_function_defstr(item)
                        funcprefix = 'function ' if title[0] == 'callbacks' else ''
                        with filepath.open(mode='w') as f:
                            data = \
                                f'---\n'\
                                f'sidebar_position: {cpos}\n'\
                                f'---\n\n'\
                                f'# {item[0]}\n'\
                                f'```lua\n{funcprefix}{luadef}\n```\n'\
                                f'{item[1]}.\n'
                            f.write(data)
                            if len(item) >= 3:
                                param_table = False
                                f.write('\n\n## Parameters\n|Parameter|Type|Description|\n|-|-|-|\n')
                                for arg in list_args(item[2:]):
                                    argtype = ''
                                    argdesc = ''
                                    if arg == 'param_table':
                                        argtype = 'table'
                                        argdesc = 'This parameter is in associative array table format, meaning it\'s specified by keys rather than parameter positions. '\
                                                  'See the [Parameters Table](#parameters-table) section.'
                                        param_table = True
                                    f.write(f'|`{arg}`|{argtype}|{argdesc}|\n')
                                if param_table:
                                    f.write('\n\n## Parameters Table\n|Key|Type|Description|\n|-|-|-|\n| | | |\n')
                            if title[0] in ('functions', 'methods', 'callbacks'):
                                f.write('\n\n## Return Values\n')
                            if title[0] in ('variables', 'properties'):
                                f.write('\n\n## Type\n')

# start from each library
base = Path('.')
pos = 1
for lib in libs:
    gen_group(lib, pos, base, 'library', '/libs/')
    pos = pos + 1
