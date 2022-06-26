local assets = {
    buttons = graphics.bitmap("assets/a320_buttons.png"),
    menu = graphics.bitmap("assets/a320_menu.png"),
    cdu = graphics.bitmap("assets/a320_cdu.png"),
    fcu = graphics.bitmap("assets/a320_fcu.png"),
    efis = graphics.bitmap("assets/a320_efis.png"),
    engine = graphics.bitmap("assets/a320_engine.png"),

    rbutton_indicator = {x=0, y=0, width=96, height=33},
    rbutton = {x=96, y=0, width=96, height=66},
    knob = {x=0, y=66, width=140, height=140},
    htoggle = {x=840, y=67, width=190, height=84},
    sbutton_indicator = {x=840, y=151, width=102, height=51},
    engsw = {x=0, y=208, width=144, height=243},
    sseg = {x=578, y=256, width=76, height=100},
    sseg_td = {x=1338, y=256, width=76, height=100},
    sseg_dot = {x=1490, y=256, width=14, height=100},
    baro_mode = {x=1420, y=76, width=100, height=30},

    sseg_font = graphics.bitmap_font(),
}

for i = 0, 9 do
    local glyph = assets.buttons:create_partial_bitmap(
        assets.sseg.x + i * assets.sseg.width, assets.sseg.y,
        assets.sseg.width, assets.sseg.height
    )
    assets.sseg_font:add_glyph(i .. "", glyph)
end
local glyph_dot = assets.buttons:create_partial_bitmap(
    assets.sseg_dot.x, assets.sseg_dot.y,
    assets.sseg_dot.width, assets.sseg_dot.height
)
glyph_dot:set_origin(assets.sseg_dot.width, 0)
assets.sseg_font:add_glyph(".", glyph_dot)
local glyph_space = assets.buttons:create_partial_bitmap(
    0, -assets.sseg.height - 2,
    assets.sseg.width, assets.sseg.height
)
assets.sseg_font:add_glyph(" ", glyph_space)
for i, code in ipairs{"t", "d"} do
    local glyph = assets.buttons:create_partial_bitmap(
        assets.sseg_td.x + (i - 1) * assets.sseg_td.width, assets.sseg_td.y,
        assets.sseg_td.width, assets.sseg_td.height
    )
    assets.sseg_font:add_glyph(code, glyph)
end

return assets