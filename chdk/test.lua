--[[

Just for testing serialusb and arduino program.

]]

usb = require('serialusb')

repeat
	command = usb.readCommand()
	
	print("command: "..command)
	sleep(500)
until 1 == 0