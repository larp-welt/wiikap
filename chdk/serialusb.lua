--[[
serialusb.lua

Reading commands from usb send by an arduino or similiar device.

usage:
	require("serialusb")
	cmd = usb.readCommand()
	
Idea stolen from CHDKlover
http://forum.chdk-treff.de/viewtopic.php?f=20&t=356
]]

local usb = {}

function usb.readCommand()
	-- waits for next command on usb and 
	-- returns it as byte value.
	
	maxBits = 3
	bits = {}
	s = 0
	
	-- wait for start
	repeat 
		s = get_usb_power()
	until s >= 60

	-- get bits
	-- a loop doesn't work here	
	repeat
		bits[1] = get_usb_power()
	until bits[1] > 0
	repeat
		bits[2] = get_usb_power()
	until bits[2] > 0
	repeat
		bits[3] = get_usb_power()
	until bits[3] > 0
	
	-- calculate command value
	byte = 0
	for i=1,maxBits do
		if bits[i]>20 then
			byte = byte + 20^(i-1)
		end
	end
	
	print(byte..": "..s.."-"..bits[1].."."..bits[2].."."..bits[3])
	-- return command
	return byte
end

return usb