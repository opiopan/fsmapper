local g1000 = mapper.device({
	type = "simhid",
	name = "SimHID G1000",
	identifier = {name = "SimHID G1000 Virtual ComPort"},
	modifiers = {
		{class = "binary", modtype = "button"},
		{class = "relative", modtype = "incdec"},
		{name = "SW26", modtype = "button", modparam={
			longpress = 4000,
			doubleclick = 0,
		}},
		{name = "SW31", modtype = "button", modparam={longpress = 4000,}},
		{name = "SW5", modtype = "raw"},
	},
})

function g1000_aircraft()
	local viewport

	function initialize()
		viewport = mapper.viewport({
			name = "G1000_View",
			displayno = 2,
			x = 0,
			y = 0,
			width = -1,
			height = -1,
			-- bgcolor = "#000000",
			bgcolor = "Black",
		})
		local pfd = viewport:register_view({
			name = "PFD",
			elements = {{
				x = 0, y = 0,
				width = 1,
				height = 1,
				object = mapper.captured_window({
					name = "G1000 PFD",
					omit_system_region = true;
				})
			}},
			mappings = {
				{event=g1000.SW31.UP, action=fs2020.eventsender({event = "Mobiflight.AS1000_PFD_SOFTKEYS_1"})},
				{event=g1000.EC1.INCREMENT, action=fs2020.eventsender({event = "Mobiflight.AS1000_PFD_HEADING_INC"})},
			}
		})
		local mfd = viewport:register_view({
			name = "MFD",
			elements = {{object = mapper.captured_window({name = "G1000 MFD"})}},
			mappings = {
				{event=g1000.SW31.UP, action=fs2020.eventsender({event = "Mobiflight.AS1000_MFD_SOFTKEYS_1"})},
				{event=g1000.EC1.INCREMENT, action=fs2020.eventsender({event = "Mobiflight.AS1000_MFD_HEADING_INC"})},
			}
		})

		viewport:change_view(pfd)

		function toggle_screen()
			if viewport.current_view == pfd then
				viewport:change_view(mfd)
			else
				viewport:change_view(pfd)
			end
		end

		viewport:set_mappings({
			{event = g1000.AUX1U.DOWN, action = toggle_screen}
			{event = g1000.AUX1D.DOWN, action = toggle_screen}
			{event = g1000.AUX2U.DOWN, action = toggle_screen}
			{event = g1000.AUX2D.DOWN, action = toggle_screen}
		})

		mapper.start_viewports()
	end

	function terminate()
		viewport = nil
		mapper.reset_viewports()
	end

	return initialize, terminate
end

local terminate_aircraft_env = function () end

mapper.set_primery_mappings({
	{event = mapper.events.change_aircraft, action = function (event, value)
		terminate_aircraft_env()
		if value.host == "fs2020" and value.aircraft == "DA40-NG Asobo" then
			local initialize
			initialize, terminate_aircraft_env = g1000_aircraft()
			initialize()
		end
	end},
})
