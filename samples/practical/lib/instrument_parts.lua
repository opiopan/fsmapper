local base_image = graphics.bitmap("assets/instrument_parts.png")

local module = {
    base = {image=base_image:create_partial_bitmap(0, 0, 500, 500)},
    cdi_bearing = {origin={x=427.05/2, y=427.05/2}, image=base_image:create_partial_bitmap(502, 0, 427.05, 427.05)},
    cdi_nav = {image=base_image:create_partial_bitmap(503, 429, 54.443, 29.862)},
    cdi_vloc = {image=base_image:create_partial_bitmap(503, 460, 70.606, 31.143)},
    cdi_gps = {image=base_image:create_partial_bitmap(573.606, 429.422, 47.405, 30.578)},
    cdi_na_loc = {image=base_image:create_partial_bitmap(588, 469, 63.307, 22.075)},
    cdi_na_gs = {image=base_image:create_partial_bitmap(662, 436, 22.075, 63.306)},
    cdi_to = {image=base_image:create_partial_bitmap(694, 432, 83.607, 36.622)},
    cdi_from = {image=base_image:create_partial_bitmap(782, 432, 83.607, 36.622)},
    cdi_heading = {image=base_image:create_partial_bitmap(702, 475, 26, 22.517)},
    cdi_tail = {image=base_image:create_partial_bitmap(746, 482, 15, 12.99)},
    cdi_knob ={image=base_image:create_partial_bitmap(929, 414, 81.624, 81.624)},
    adf_bg ={image=base_image:create_partial_bitmap(930, 0, 335, 335)},
    adf_bearing ={origin={x=420/2, y=420/2}, image=base_image:create_partial_bitmap(1266, 0, 420, 420)},
    adf_starndard ={image=base_image:create_partial_bitmap(1687, 0, 420, 420)},
    adf_needle ={origin={x=317.824/2, y=22.507/2}, image=base_image:create_partial_bitmap(930, 376, 317.824, 22.507)},
    adf_knob ={image=base_image:create_partial_bitmap(1016, 414, 81.624, 81.624)},
    dg_knob_right = {image=base_image:create_partial_bitmap(1100, 414, 81.624, 81.624)},
    dg_knob_left = {image=base_image:create_partial_bitmap(1183, 414, 81.624, 81.624)},
    dg_heading_bug = {origin={x=44.182/2, y=197.783}, image=base_image:create_partial_bitmap(1272, 451, 44.182, 47.248)},
    dg_bearing = {origin={x=420/2, y=420/2}, image=base_image:create_partial_bitmap(2108, 0, 420, 420)},
    dg_standard = {image=base_image:create_partial_bitmap(2529, 0, 123, 314.478)},
    dg_standard_red = {image=base_image:create_partial_bitmap(2653, 0, 470, 470)},
    dg_standard_type2 = {image=base_image:create_partial_bitmap(3124, 0, 470, 470)},
}

local function set_origin(part)
    part.image:set_origin(part.origin.x, part.origin.y)
end

set_origin(module.cdi_bearing)
set_origin(module.adf_bearing)
set_origin(module.adf_needle)
set_origin(module.dg_heading_bug)
set_origin(module.dg_bearing)

return module