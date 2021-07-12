mapper.print("start script")

g1000 = mapper.device({
    name = "SimHID G1000",
    type = "simhid",
    identifier = {path = "/dev/tty.usbmodem3295396E31301"},
    modifiers = {
        {class = "binary", modtype = "button"},
        {class = "relative", modtype = "incdec"},
		{name = "EC1P", modtype = "button", modparam={
	    	longpress = 3000,
		}},
		{name = "SW2", modtype = "button", modparam={
	    	doubleclick = 400,
		}},
    },
})

mapper.print("device SimHID G1000 is opened")

mapper.print("g1000:")
for k, v in pairs(g1000) do
    mapper.print("    "..k)
end
mapper.print("g1000.SW1:")
for k, v in pairs(g1000.SW1) do
    mapper.print("    "..k)
end
mapper.print("g1000.EC1:")
for k, v in pairs(g1000.EC1) do
    mapper.print("    "..k)
end
mapper.print("g1000.EC1P:")
for k, v in pairs(g1000.EC1P) do
    mapper.print("    "..k)
end
mapper.print("g1000.SW2:")
for k, v in pairs(g1000.SW2) do
    mapper.print("    "..k)
end
