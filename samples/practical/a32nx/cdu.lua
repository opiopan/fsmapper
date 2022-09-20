local view_width = 1084
local view_height = 1541

local mod_context = {}

--------------------------------------------------------------------------------------
-- button definitions
--------------------------------------------------------------------------------------
local attr_side = {width = 73, height = 58, rratio = 0.1}
local attr_func = {width = 119, height = 63, rratio = 0.1}
local attr_num = {width = 80, height = 80, rratio = 0.5}
local attr_alphabet = {width = 80, height = 80, rratio = 0.1}

local buttons = {
    l1 = {x = 20, y = 95, attr=attr_side, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_L1")},
    l2 = {x = 20, y = 177, attr=attr_side, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_L2")},
    l3 = {x = 20, y = 266, attr=attr_side, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_L3")},
    l4 = {x = 20, y = 354, attr=attr_side, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_L4")},
    l5 = {x = 20, y = 440, attr=attr_side, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_L5")},
    l6 = {x = 20, y = 534, attr=attr_side, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_L6")},
    r1 = {x = 988, y = 98, attr=attr_side, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_R1")},
    r2 = {x = 988, y = 180, attr=attr_side, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_R2")},
    r3 = {x = 988, y = 269, attr=attr_side, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_R3")},
    r4 = {x = 988, y = 357, attr=attr_side, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_R4")},
    r5 = {x = 988, y = 443, attr=attr_side, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_R5")},
    r6 = {x = 988, y = 537, attr=attr_side, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_R6")},
    dir = {x = 120, y = 711, attr=attr_func, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_DIR")},
    prog = {x = 247, y = 712, attr=attr_func, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_PROG")},
    perf = {x = 374, y = 712, attr=attr_func, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_PERF")},
    init = {x = 501, y = 712, attr=attr_func, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_INIT")},
    data = {x = 627, y = 712, attr=attr_func, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_DATA")},
    fpln = {x = 120, y = 799, attr=attr_func, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_FPLN")},
    rad = {x = 247, y = 799, attr=attr_func, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_RAD")},
    fuel = {x = 374, y = 799, attr=attr_func, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_FUEL")},
    sec = {x = 501, y = 799, attr=attr_func, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_SEC")},
    atc = {x = 627, y = 800, attr=attr_func, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_ATC")},
    menu = {x = 754, y = 800, attr=attr_func, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_MENU")},
    airport = {x = 120, y = 883, attr=attr_func, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_AIRPORT")},
    next = {x = 120, y = 1057, attr=attr_func, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_NEXTPAGE")},
    prev = {x = 120, y = 973, attr=attr_func, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_PREVPAGE")},
    up = {x = 247, y = 973, attr=attr_func, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_UP")},
    down = {x = 247, y = 1057, attr=attr_func, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_DOWN")},
    num0 = {x = 240, y = 1440, attr=attr_num, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_0")},
    num1 = {x = 135, y = 1163, attr=attr_num, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_1")},
    num2 = {x = 239, y = 1163, attr=attr_num, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_2")},
    num3 = {x = 344, y = 1163, attr=attr_num, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_3")},
    num4 = {x = 135, y = 1256, attr=attr_num, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_4")},
    num5 = {x = 239, y = 1256, attr=attr_num, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_5")},
    num6 = {x = 344, y = 1256, attr=attr_num, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_6")},
    num7 = {x = 136, y = 1350, attr=attr_num, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_7")},
    num8 = {x = 240, y = 1350, attr=attr_num, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_8")},
    num9 = {x = 345, y = 1350, attr=attr_num, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_9")},
    dot = {x = 136, y = 1440, attr=attr_num, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_DOT")},
    plusminus = {x = 345, y = 1440, attr=attr_num, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_PLUSMINUS")},
    a = {x = 453, y = 921, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_A")},
    b = {x = 558, y = 923, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_B")},
    c = {x = 666, y = 921, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_C")},
    d = {x = 767.5, y = 923, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_D")},
    e = {x = 874, y = 923, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_E")},
    f = {x = 453, y = 1018, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_F")},
    g = {x = 558, y = 1020, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_G")},
    h = {x = 666, y = 1018, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_H")},
    i = {x = 767.5, y = 1020, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_I")},
    j = {x = 874, y = 1020, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_J")},
    k = {x = 453, y = 1126, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_K")},
    l = {x = 558, y = 1128, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_L")},
    m = {x = 666, y = 1126, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_M")},
    n = {x = 767.5, y = 1128, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_N")},
    o = {x = 874, y = 1128, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_O")},
    p = {x = 453, y = 1233, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_P")},
    q = {x = 558, y = 1235, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_Q")},
    r = {x = 666, y = 1233, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_R")},
    s = {x = 767.5, y = 1235, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_S")},
    t = {x = 874, y = 1235, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_T")},
    u = {x = 453, y = 1339, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_U")},
    v = {x = 558, y = 1341, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_V")},
    w = {x = 666, y = 1339, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_W")},
    x = {x = 767.5, y = 1341, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_X")},
    y = {x = 874, y = 1341, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_Y")},
    z = {x = 455.5, y = 1443, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_Z")},
    div = {x = 560.5, y = 1445, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_DIV")},
    sp = {x = 668.5, y = 1443, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_SP")},
    ovfy = {x = 770, y = 1445, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_OVFY")},
    clr = {x = 876.5, y = 1445, attr=attr_alphabet, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_CLR")},
}

--------------------------------------------------------------------------------------
-- Register events / Build Event-Action mappings / Build View element definitions
--------------------------------------------------------------------------------------
local mappings = {}
local view_elements = {}
for key, button in pairs(buttons) do
    local evid = mapper.register_event("A320_CDU_" .. string.upper(key))
    mappings[#mappings + 1] = {event=evid, action=button.action}
    view_elements[#view_elements + 1] = {
        object = mapper.view_elements.operable_area{round_ratio= button.attr.rratio, event_tap=evid},
        x = button.x, y = button.y,
        width = button.attr.width, height = button.attr.height
    }
end

--------------------------------------------------------------------------------------
-- function to create view definition
--------------------------------------------------------------------------------------
local assets = require("a32nx/assets")
local scale_factor = assets.cdu.width /view_width
local bg_image = assets.cdu:create_partial_bitmap(0, 0, view_width * scale_factor, view_height * scale_factor)

function mod_context.viewdef(name, window)
    local elements = {
        {
            object =window,
            x = 168, y = 0,
            width = 726, height = 670
        }
    }

    for i, element in ipairs(view_elements) do
        elements[#elements + 1] = element
    end

    local view_definition = {
        name = name,
        elements = elements,
        background = bg_image,
        logical_width = view_width,
        logical_height = view_height,
        horizontal_alignment = "center",
        vertical_alignment = "top",
        mappings = mappings,
    }

    return view_definition
end

return mod_context
