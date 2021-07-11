mapper.print("start script")

g1000 = mapper.device({
    name = "SimHID G1000",
    type = "simhid",
    identifier = {path = "/dev/tty.usbmodem3295396E31301"},
})

mapper.print("device SimHID G1000 is opened")

mapper.print("")
mapper.print("g1000:")
for k, v in pairs(g1000) do
    mapper.print("    "..k)
end
mapper.print("g1000.EC4X:")
for k, v in pairs(g1000.EC4X) do
    mapper.print("    "..k)
end
