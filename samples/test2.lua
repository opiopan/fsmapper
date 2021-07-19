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
    {event=g1000.SW3.up, action=function () mapper.abort() end},
    {event=g1000.SW27.down, action=test.messenger("test of native action")},
})
