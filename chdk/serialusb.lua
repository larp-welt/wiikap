--[[
serialusb.lua

Reading commands from usb send by an arduino or similiar device.

usage:
	require("serialusb")
	cmd = usb.readCommand()
	
Idea stolen from CHDKlover
http://forum.chdk-treff.de/viewtopic.php?f=20&t=356
]]

local kap = {}

function kap.readCommand()
	-- waits for next command on usb and 
	-- returns it as byte value.
	
	maxBits = 3
	bits = {}
	
	-- wait for start
	repeat 
		s = get_usb_power()
	until s >= 6

	-- get bits
	for i=1,maxBits do
		repeat
			bits[i] = get_usb_power()
		until bits[i] > 0
	end
	
	-- calculate command value
	byte = 0
	for i=1,maxBits do
		if bits[0]>2 then
			byte = byte + 2^(i-1)
		end
	end
	
	-- return command
	return byte
end

return usb