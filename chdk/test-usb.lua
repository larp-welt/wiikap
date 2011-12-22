--[[
rem Read USB

@title L:Test USB
]]


repeat
	repeat
		power = get_usb_power()
	until power > 0
	
	print("usb: "..power)
until is_key("set") == true
