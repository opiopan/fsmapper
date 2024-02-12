local module = {
    seg7_type1 = 1,
    seg7_type2 = 2,
}

local common = require("lib/common")

local defs = {}

defs[module.seg7_type1] = {
    height = 100, width = 59.539,
    point_diameter = 16.304,
    path_defs = {
        {{0, 2.791}, {11.89, 13.918}, {11.89, 41.436}, {0, 48.121}},
        {{0, 51.698}, {11.89, 58.383}, {11.89, 85.901}, {0, 97.028}},
        {{47.65, 13.918}, {59.539, 2.791}, {59.539, 48.121}, {47.649, 41.436}},
        {{47.65, 58.383}, {59.539, 51.698}, {59.539, 97.028}, {47.65, 85.901}},
        {{4.51, 0}, {56.534, 0}, {44.427, 10.703}, {16.001, 10.703}},
        {{4.51, 50.068}, {14.372, 44.371}, {43.923, 44.371}, {54.96, 50.246}, {45.169, 55.765}, {13.66, 55.587}},
        {{16.001, 89.297}, {44.427, 89.297}, {56.534, 100}, {4.51, 100}}
    },
    patterns = {}
}
defs[module.seg7_type1].patterns['0'] = {1, 2, 3, 4, 5, 7}
defs[module.seg7_type1].patterns['1'] = {3, 4}
defs[module.seg7_type1].patterns['2'] = {2, 3, 5, 6, 7}
defs[module.seg7_type1].patterns['3'] = {3, 4, 5, 6, 7}
defs[module.seg7_type1].patterns['4'] = {1, 3, 4, 6}
defs[module.seg7_type1].patterns['5'] = {1, 4, 5, 6, 7}
defs[module.seg7_type1].patterns['6'] = {1, 2, 4, 5, 6, 7}
defs[module.seg7_type1].patterns['7'] = {1, 3, 4, 5}
defs[module.seg7_type1].patterns['8'] = {1, 2, 3, 4, 5, 6, 7}
defs[module.seg7_type1].patterns['9'] = {1, 3, 4, 5, 6, 7}
defs[module.seg7_type1].patterns['-'] = {6}
defs[module.seg7_type1].patterns['o'] = {2, 4, 6, 7}
defs[module.seg7_type1].patterns[' '] = {}

defs[module.seg7_type2] = {
    height = 100, width = 59.349,
    point_diameter = 15.013,
    path_defs = {
        {{3.48, 5.199}, {0, 9.172}, {0, 42.905}, {5.664, 47.119}, {13.086, 41.972}, {13.086, 15.229}},
        {{5.664, 52.85}, {0, 57.064}, {0, 90.797}, {3.48, 94.769}, {13.086, 84.74}, {13.037, 57.323}},
        {{52.653, 0.897}, {45.406, 16.154}, {45.787, 41.519}, {53.887, 47.92}, {56.658, 48.385}, {59.349, 44.952}, {59.349, 10.358}, {56.343, 3.149}},
        {{56.658, 51.583}, {53.887, 52.049}, {45.787, 58.449}, {45.406, 84.987}, {53.022, 100}, {59.349, 89.611}, {59.328, 55.016}},
        {{8.616, 0}, {4.672, 3.472}, {14.77, 13.949}, {43.663, 13.949}, {50.505, 0}},
        {{17, 41.972}, {6.708, 49.789}, {16.07, 56.52}, {44.324, 56.52}, {53.022, 50.044}, {43.081, 41.972}},
        {{14.77, 86.02}, {4.672, 96.496}, {8.616, 100}, {50.505, 100}, {43.663, 86.283}},
        {{30.148, 21.485}, {22.591, 30.183}, {22.591, 40.972}, {37.139, 40.972}, {37.139, 30.183}},
        {{37.139, 57.589}, {22.591, 57.589}, {22.591, 68.378}, {29.581, 77.076}, {37.139, 68.378}},
    },
    patterns = {}
}
defs[module.seg7_type2].patterns['0'] = {1, 2, 3, 4, 5, 7}
defs[module.seg7_type2].patterns['1'] = {3, 4}
defs[module.seg7_type2].patterns['2'] = {2, 3, 5, 6, 7}
defs[module.seg7_type2].patterns['3'] = {3, 4, 5, 6, 7}
defs[module.seg7_type2].patterns['4'] = {1, 3, 4, 6}
defs[module.seg7_type2].patterns['5'] = {1, 4, 5, 6, 7}
defs[module.seg7_type2].patterns['6'] = {1, 2, 4, 5, 6, 7}
defs[module.seg7_type2].patterns['7'] = {1, 3, 4, 5}
defs[module.seg7_type2].patterns['8'] = {1, 2, 3, 4, 5, 6, 7}
defs[module.seg7_type2].patterns['9'] = {1, 3, 4, 5, 6, 7}
defs[module.seg7_type2].patterns['-'] = {6}
defs[module.seg7_type2].patterns['+'] = {6, 8, 9}
defs[module.seg7_type2].patterns['o'] = {2, 4, 6, 7}
defs[module.seg7_type2].patterns[' '] = {}


--------------------------------------------------------------------------------------
-- create_font():
--    Create a font object for segmented display
--
--    Pramameters:
--        type       font type number
--        height     font height
--        width      font widhth include gap between each letter
--        color      OPTIONAL: font color, default value is "white"
--        patterns   OPTIONAL: patterns for additional letters
--------------------------------------------------------------------------------------
function module.create_font(params)
    local def = defs[params.type]

    -- instancing the path objects for each segment
    if def.paths == nil then
        def.paths = {}
        for i, path_def in pairs(def.path_defs) do
            local from = path_def[1]
            local figure = {fill_mode="winding",  from=from, segments={}}
            for j = 2, #path_def do
                figure.segments[#figure.segments + 1] = {to = path_def[j]}
            end
            figure.segments[#figure.segments + 1] = {to = from}
            local path = graphics.path()
            path:add_figure(figure)
            path:fix()
            def.paths[i] = path
        end
    end

    -- maerge the system predefined patterns and user specified patterns
    local num_letter = common.get_table_size(def.patterns) + 1 -- "+ 1" means floating point
    local patterns = common.merge_table({}, def.patterns)
    if params.patterns ~= nil then
        num_letter = num_letter + common.get_table_size(params.patterns)
        common.merge_table(patterns, params.patterns)
    end
    local bitmap = graphics.bitmap(math.floor((params.width + 1) * num_letter + 0.9), math.floor(params.height + 0.9))

    -- render a bitmap as a source of font object
    local x = 0
    local scale = params.height / def.height
    local glyphs = {}
    local rctx = graphics.rendering_context(bitmap)
    if params.color == nil then
        rctx:set_brush(graphics.color("white"))
    else
        rctx:set_brush(params.color)
    end
    for key, pattern in pairs(patterns) do
        for i, path_no in ipairs(pattern) do
            rctx:fill_geometry{geometry = def.paths[path_no], x = x, y = 0, scale = scale}
        end
        glyphs[key] = bitmap:create_partial_bitmap(x, 0, params.width, params.height)
        x = x + params.width + 1
    end
    local gap_width = params.width - def.width * scale
    local point_diameter = def.point_diameter * scale
    rctx:fill_geometry{
        geometry = common.circle,
        x = x + params.width - gap_width + (gap_width - point_diameter) / 2,
        y = params.height - point_diameter,
        scale = point_diameter
    }
    glyphs["."] = bitmap:create_partial_bitmap(x, 0, params.width, params.height)
    glyphs["."]:set_origin(params.width, 0)
    rctx:finish_rendering()

    -- generate font object
    local font = graphics.bitmap_font()
    for key, glyph in pairs(glyphs) do
        font:add_glyph(key, glyph)
    end

    return font
end

return module