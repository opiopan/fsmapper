mapper.print("start script")

g1000 = mapper.device({
    name = "SimHID G1000",
    type = "simhid",
    identifier = {path = "COM3"},
    modifiers = {
        {class = "binary", modtype = "button"},
        {class = "relative", modtype = "incdec"},
		{name = "EC1P", modtype = "button", modparam={longpress = 2000}},
		{name = "SW26", modtype = "button", modparam={longpress = 2000}},
		{name = "SW31", modtype = "button", modparam={longpress = 2000}},
		{name = "SW2", modtype = "button", modparam={doubleclick = 400,}},
		{name = "EC9Y", modtype = "raw"},
    },
})

mapper.print("device SimHID G1000 is opened")

mapper.print("g1000:")
for k, v in pairs(g1000) do
    mapper.print("    "..k)
end
mapper.print("g1000.SW1:")
for k, v in pairs(g1000.SW1) do
    mapper.print("    "..k.." = "..v)
end
mapper.print("g1000.EC1:")
for k, v in pairs(g1000.EC1) do
    mapper.print("    "..k.." = "..v)

end
mapper.print("g1000.EC1P:")
for k, v in pairs(g1000.EC1P) do
    mapper.print("    "..k.." = "..v)
end
mapper.print("g1000.SW2:")
for k, v in pairs(g1000.SW2) do
    mapper.print("    "..k.." = "..v)
end

mapper.print("mapper:")
for k, v in pairs(mapper) do
    mapper.print("    "..k)
end

mapper.set_primery_mappings({
    {event=g1000.SW2.doubleclick, action=function (event, value) mapper.print("    do action!: eventid="..event)  end },
    {event=g1000.SW4.up, action=function () mapper.abort() end},
    {event=g1000.SW3.down, action=test.messenger("test of native action")},
    {event=mapper.events.change_aircraft, action=function (event, value) 
        if value.host then
            mapper.print("    sim: "..value.host) 
        else
            mapper.print("    disconnected")
        end
    end},
    {event=g1000.EC4X.increment, action=function () fs2020.send_event("Mobiflight.AP_ALT_VAR_INC100") end},
    {event=g1000.EC4X.decrement, action=function () fs2020.send_event("Mobiflight.AP_ALT_VAR_DEC100") end},
    {event=g1000.EC4Y.increment, action=fs2020.event_sender("Mobiflight.AP_ALT_VAR_INC1000")},
    {event=g1000.EC4Y.decrement, action=fs2020.event_sender("Mobiflight.AP_ALT_VAR_DEC1000")},
    {event=g1000.EC3.increment, action=fs2020.event_sender("Mobiflight.AS1000_PFD_HEADING_INC")},
    {event=g1000.EC3.decrement, action=fs2020.event_sender("Mobiflight.AS1000_PFD_HEADING_DEC")},
    {event=g1000.SW27.down, action=function () fs2020.send_event("Mobiflight.AS1000_PFD_DIRECTTO") end},
})
