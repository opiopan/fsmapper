local view_width = 1084
local view_height = 1541

local mod_context = {}

--------------------------------------------------------------------------------------
-- Register events which occurrs when cdu buttons displayed on the view are tapped.
--------------------------------------------------------------------------------------
local cdu = {
    l1 = mapper.register_event("A320_CDU_L1"),
    l2 = mapper.register_event("A320_CDU_L2"),
    l3 = mapper.register_event("A320_CDU_L3"),
    l4 = mapper.register_event("A320_CDU_L4"),
    l5 = mapper.register_event("A320_CDU_L5"),
    l6 = mapper.register_event("A320_CDU_L6"),
    r1 = mapper.register_event("A320_CDU_R1"),
    r2 = mapper.register_event("A320_CDU_R2"),
    r3 = mapper.register_event("A320_CDU_R3"),
    r4 = mapper.register_event("A320_CDU_R4"),
    r5 = mapper.register_event("A320_CDU_R5"),
    r6 = mapper.register_event("A320_CDU_R6"),
    dir = mapper.register_event("A320_CDU_DIR"),
    prog = mapper.register_event("A320_CDU_PROG"),
    perf = mapper.register_event("A320_CDU_PERF"),
    init = mapper.register_event("A320_CDU_INIT"),
    data = mapper.register_event("A320_CDU_PERF"),
    fpln = mapper.register_event("A320_CDU_FPLN"),
    rad = mapper.register_event("A320_CDU_RAD"),
    fuel = mapper.register_event("A320_CDU_FUEL"),
    sec = mapper.register_event("A320_CDU_SEC"),
    atc = mapper.register_event("A320_CDU_ATC"),
    menu = mapper.register_event("A320_CDU_MENU"),
    airport = mapper.register_event("A320_CDU_AIRPORT"),
    next = mapper.register_event("A320_CDU_NEXT"),
    prev = mapper.register_event("A320_CDU_PREV"),
    up = mapper.register_event("A320_CDU_UP"),
    down = mapper.register_event("A320_CDU_DOWN"),
    num0 = mapper.register_event("A320_CDU_0"),
    num1 = mapper.register_event("A320_CDU_1"),
    num2 = mapper.register_event("A320_CDU_2"),
    num3 = mapper.register_event("A320_CDU_3"),
    num4 = mapper.register_event("A320_CDU_4"),
    num5 = mapper.register_event("A320_CDU_5"),
    num6 = mapper.register_event("A320_CDU_6"),
    num7 = mapper.register_event("A320_CDU_7"),
    num8 = mapper.register_event("A320_CDU_8"),
    num9 = mapper.register_event("A320_CDU_9"),
    dot = mapper.register_event("A320_CDU_DOT"),
    plusminus = mapper.register_event("A320_CDU_+/-"),
    a = mapper.register_event("A320_CDU_A"),
    b = mapper.register_event("A320_CDU_B"),
    c = mapper.register_event("A320_CDU_C"),
    d = mapper.register_event("A320_CDU_D"),
    e = mapper.register_event("A320_CDU_E"),
    f = mapper.register_event("A320_CDU_F"),
    g = mapper.register_event("A320_CDU_G"),
    h = mapper.register_event("A320_CDU_H"),
    i = mapper.register_event("A320_CDU_I"),
    j = mapper.register_event("A320_CDU_J"),
    k = mapper.register_event("A320_CDU_K"),
    l = mapper.register_event("A320_CDU_L"),
    m = mapper.register_event("A320_CDU_M"),
    n = mapper.register_event("A320_CDU_N"),
    o = mapper.register_event("A320_CDU_O"),
    p = mapper.register_event("A320_CDU_P"),
    q = mapper.register_event("A320_CDU_Q"),
    r = mapper.register_event("A320_CDU_R"),
    s = mapper.register_event("A320_CDU_S"),
    t = mapper.register_event("A320_CDU_T"),
    u = mapper.register_event("A320_CDU_U"),
    v = mapper.register_event("A320_CDU_V"),
    w = mapper.register_event("A320_CDU_W"),
    x = mapper.register_event("A320_CDU_X"),
    y = mapper.register_event("A320_CDU_Y"),
    z = mapper.register_event("A320_CDU_Z"),
    div = mapper.register_event("A320_CDU_DIV"),
    sp = mapper.register_event("A320_CDU_SP"),
    ovfy = mapper.register_event("A320_CDU_OVFY"),
    clr = mapper.register_event("A320_CDU_CLR"),
}

--------------------------------------------------------------------------------------
-- Define event-action mappings
--------------------------------------------------------------------------------------
local maps = {
    {event=cdu.l1, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_L1")},
    {event=cdu.l2, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_L2")},
    {event=cdu.l3, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_L3")},
    {event=cdu.l4, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_L4")},
    {event=cdu.l5, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_L5")},
    {event=cdu.l6, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_L6")},
    {event=cdu.r1, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_R1")},
    {event=cdu.r2, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_R2")},
    {event=cdu.r3, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_R3")},
    {event=cdu.r4, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_R4")},
    {event=cdu.r5, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_R5")},
    {event=cdu.r6, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_R6")},
    {event=cdu.dir, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_DIR")},
    {event=cdu.prog, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_PROG")},
    {event=cdu.perf, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_PERF")},
    {event=cdu.init, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_INIT")},
    {event=cdu.data, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_DATA")},
    {event=cdu.fpln, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_FPLN")},
    {event=cdu.rad, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_RAD")},
    {event=cdu.fuel, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_FUEL")},
    {event=cdu.sec, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_SEC")},
    {event=cdu.atc, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_ATC")},
    {event=cdu.menu, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_MENU")},
    {event=cdu.airport, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_AIRPORT")},
    {event=cdu.next, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_NEXTPAGE")},
    {event=cdu.prev, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_PREVPAGE")},
    {event=cdu.up, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_UP")},
    {event=cdu.down, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_DOWN")},
    {event=cdu.num0, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_0")},
    {event=cdu.num1, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_1")},
    {event=cdu.num2, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_2")},
    {event=cdu.num3, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_3")},
    {event=cdu.num4, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_4")},
    {event=cdu.num5, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_5")},
    {event=cdu.num6, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_6")},
    {event=cdu.num7, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_7")},
    {event=cdu.num8, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_8")},
    {event=cdu.num9, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_9")},
    {event=cdu.dot, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_DOT")},
    {event=cdu.plusminus, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_PLUSMINUS")},
    {event=cdu.a, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_A")},
    {event=cdu.b, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_B")},
    {event=cdu.c, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_C")},
    {event=cdu.d, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_D")},
    {event=cdu.e, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_E")},
    {event=cdu.f, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_F")},
    {event=cdu.g, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_G")},
    {event=cdu.h, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_H")},
    {event=cdu.i, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_I")},
    {event=cdu.j, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_J")},
    {event=cdu.k, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_K")},
    {event=cdu.l, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_L")},
    {event=cdu.m, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_M")},
    {event=cdu.n, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_N")},
    {event=cdu.o, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_O")},
    {event=cdu.p, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_P")},
    {event=cdu.q, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_Q")},
    {event=cdu.r, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_R")},
    {event=cdu.s, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_S")},
    {event=cdu.t, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_T")},
    {event=cdu.u, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_U")},
    {event=cdu.v, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_V")},
    {event=cdu.w, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_W")},
    {event=cdu.x, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_X")},
    {event=cdu.y, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_Y")},
    {event=cdu.z, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_Z")},
    {event=cdu.div, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_DIV")},
    {event=cdu.sp, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_SP")},
    {event=cdu.ovfy, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_OVFY")},
    {event=cdu.clr, action=fs2020.event_sender("Mobiflight.A320_Neo_CDU_1_BTN_CLR")},
}

--------------------------------------------------------------------------------------
-- View definition  CDU
--------------------------------------------------------------------------------------
local rcolor = graphics.color("yellow", 0.25)
local rratio_side = 0.1
local rratio_func = 0.1
local rratio_num = 0.5
local rratio_alphabet = 0.1
local assets = require("a32nx/assets")
local scale_factor = assets.cdu.width /view_width
local bg_image = assets.cdu:create_partial_bitmap(0, 0, view_width * scale_factor, view_height * scale_factor)

local view_elements = {
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_side,
            event_tap = cdu.l1,
        }),
        x = 20, y = 95,
        width = 73, height = 58,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_side,
            event_tap = cdu.l2,
        }),
        x = 20, y = 177,
        width = 73, height = 58,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_side,
            event_tap = cdu.l3,
        }),
        x = 20, y = 266,
        width = 73, height = 58,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_side,
            event_tap = cdu.l4,
        }),
        x = 20, y = 354,
        width = 73, height = 58,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_side,
            event_tap = cdu.l5,
        }),
        x = 20, y = 440,
        width = 73, height = 58,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_side,
            event_tap = cdu.l6,
        }),
        x = 20, y = 534,
        width = 73, height = 58,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_side,
            event_tap = cdu.r1,
        }),
        x = 988, y = 98,
        width = 73, height = 58,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_side,
            event_tap = cdu.r2,
        }),
        x = 988, y = 180,
        width = 73, height = 58,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_side,
            event_tap = cdu.r3,
        }),
        x = 988, y = 269,
        width = 73, height = 58,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_side,
            event_tap = cdu.r4,
        }),
        x = 988, y = 357,
        width = 73, height = 58,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_side,
            event_tap = cdu.r5,
        }),
        x = 988, y = 443,
        width = 73, height = 58,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_side,
            event_tap = cdu.r6,
        }),
        x = 988, y = 537,
        width = 73, height = 58,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_func,
            event_tap = cdu.dir,
        }),
        x = 120, y = 711,
        width = 119, height = 63,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_func,
            event_tap = cdu.prog,
        }),
        x = 247, y = 712,
        width = 119, height = 63,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_func,
            event_tap = cdu.perf,
        }),
        x = 374, y = 712,
        width = 119, height = 63,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_func,
            event_tap = cdu.init,
        }),
        x = 501, y = 712,
        width = 119, height = 63,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_func,
            event_tap = cdu.data,
        }),
        x = 627, y = 712,
        width = 119, height = 63,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_func,
            event_tap = cdu.fpln,
        }),
        x = 120, y = 799,
        width = 119, height = 63,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_func,
            event_tap = cdu.rad,
        }),
        x = 247, y = 799,
        width = 119, height = 63,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_func,
            event_tap = cdu.fuel,
        }),
        x = 374, y = 799,
        width = 119, height = 63,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_func,
            event_tap = cdu.sec,
        }),
        x = 501, y = 799,
        width = 119, height = 63,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_func,
            event_tap = cdu.atc,
        }),
        x = 627, y = 800,
        width = 119, height = 63,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_func,
            event_tap = cdu.menu,
        }),
        x = 754, y = 800,
        width = 119, height = 63,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_func,
            event_tap = cdu.airport,
        }),
        x = 120, y = 883,
        width = 119, height = 63,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_func,
            event_tap = cdu.prev,
        }),
        x = 120, y = 973,
        width = 119, height = 63,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_func,
            event_tap = cdu.up,
        }),
        x = 247, y = 973,
        width = 119, height = 63,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_func,
            event_tap = cdu.next,
        }),
        x = 120, y = 1057,
        width = 119, height = 63,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_func,
            event_tap = cdu.down,
        }),
        x = 247, y = 1057,
        width = 119, height = 63,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_num,
            event_tap = cdu.num1,
        }),
        x = 135, y = 1163,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_num,
            event_tap = cdu.num2,
        }),
        x = 239, y = 1163,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_num,
            event_tap = cdu.num3,
        }),
        x = 344, y = 1163,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_num,
            event_tap = cdu.num4,
        }),
        x = 135, y = 1256,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_num,
            event_tap = cdu.num5,
        }),
        x = 239, y = 1256,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_num,
            event_tap = cdu.num6,
        }),
        x = 344, y = 1256,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_num,
            event_tap = cdu.num7,
        }),
        x = 136, y = 1350,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_num,
            event_tap = cdu.num8,
        }),
        x = 240, y = 1350,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_num,
            event_tap = cdu.num9,
        }),
        x = 345, y = 1350,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_num,
            event_tap = cdu.dot,
        }),
        x = 136, y = 1440,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_num,
            event_tap = cdu.num0,
        }),
        x = 240, y = 1440,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_num,
            event_tap = cdu.plusminus,
        }),
        x = 345, y = 1440,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.a,
        }),
        x = 453, y = 921,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.b,
        }),
        x = 558, y = 923,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.c,
        }),
        x = 666, y = 921,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.d,
        }),
        x = 767.5, y = 923,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.e,
        }),
        x = 874, y = 923,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.f,
        }),
        x = 453, y = 1018,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.g,
        }),
        x = 558, y = 1020,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.h,
        }),
        x = 666, y = 1018,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.i,
        }),
        x = 767.5, y = 1020,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.j,
        }),
        x = 874, y = 1020,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.k,
        }),
        x = 453, y = 1126,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.l,
        }),
        x = 558, y = 1128,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.m,
        }),
        x = 666, y = 1126,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.n,
        }),
        x = 767.5, y = 1128,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.o,
        }),
        x = 874, y = 1128,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.p,
        }),
        x = 453, y = 1233,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.q,
        }),
        x = 558, y = 1235,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.r,
        }),
        x = 666, y = 1233,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.s,
        }),
        x = 767.5, y = 1235,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.t,
        }),
        x = 874, y = 1235,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.u,
        }),
        x = 453, y = 1339,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.v,
        }),
        x = 558, y = 1341,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.w,
        }),
        x = 666, y = 1339,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.x,
        }),
        x = 767.5, y = 1341,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.y,
        }),
        x = 874, y = 1341,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.z,
        }),
        x = 455.5, y = 1443,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.div,
        }),
        x = 560.5, y = 1445,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.sp,
        }),
        x = 668.5, y = 1443,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.ovfy,
        }),
        x = 770, y = 1445,
        width = 80, height = 80,
    },
    {
        object = mapper.view_elements.operable_area({
            reaction_color = rcolor,
            round_ratio = rratio_alphabet,
            event_tap = cdu.clr,
        }),
        x = 876.5, y = 1445,
        width = 80, height = 80,
    },
}

local function create_view_def(name, window)
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
        mappings = maps,
    }

    return view_definition
end

mod_context.viewdef = create_view_def

return mod_context
