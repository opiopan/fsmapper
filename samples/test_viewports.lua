g1000_dev = mapper.device({
    name = "SimHID G1000",
    type = "simhid",
    identifier = {path = "COM3"},
    modifiers = {
        {class = "binary", modtype = "button"},
        {class = "relative", modtype = "incdec"},
    },
})
local g1000 = g1000_dev.events

local pfd_maps = {
    {event=g1000.EC1.increment, action=fs2020.event_sender("Mobiflight.AS1000_PFD_VOL_1_INC")},
    {event=g1000.EC1.decrement, action=fs2020.event_sender("Mobiflight.AS1000_PFD_VOL_1_DEC")},
}

local mfd_maps = {
    {event=g1000.EC1.increment, action=fs2020.event_sender("Mobiflight.AS1000_MFD_VOL_1_INC")},
    {event=g1000.EC1.decrement, action=fs2020.event_sender("Mobiflight.AS1000_MFD_VOL_1_DEC")},
}

local viewport = mapper.viewport({
    name = "G1000 Viewport",
    displayno = 1,
    x = 0.6, y = 0.75 / 2,
    width = 0.25, height = 0.25,
    bgcolor = "Green",
})
local pfd = viewport:register_view({
    name = "PFD",
    elements = {{object = mapper.captured_window({name = "G1000 PFD"})}},
    mappings = pfd_maps,
})
local mfd = viewport:register_view({
    name = "MFD",
    elements = {{object = mapper.captured_window({name = "G1000 MFD"})}},
    mappings = mfd_maps,
})

function toggle_view()
    if viewport.current_view == pfd then
        viewport:change_view(mfd)
    else
        viewport:change_view(pfd)
    end
end

viewport:set_mappings({
    {event=g1000.AUX2D.down, action=toggle_view},
})
viewport:add_mappings({
    {event=g1000.AUX2U.down, action=toggle_view},
})

mapper.start_viewports()

mapper.set_primery_mappings({
    {event=mapper.events.change_aircraft, action=function (event, value)
        if value.aircraft == "Airbus A320 Neo FlyByWire" then
            joymap.base = joymap_full
            update_secondary_mappings()
        else
            joymap.base = joymap_noab
            update_secondary_mappings()
        end
        if value.host then
            if value.aircraft then
                mapper.print("    [sim]: "..value.host.." [aircraft]: "..value.aircraft) 
            else
                mapper.print("    [sim]: "..value.host.." [aircraft]:") 
            end
        else
            mapper.print("    [sim]: disconnected")
        end
    end},
    {event=g1000.AUX1U.down, action=function () mapper.reset_viewports() end},
    {event=g1000.AUX1D.down, action=function () mapper.abort() end},
})