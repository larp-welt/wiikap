--[[

Just for testing serialusb and arduino program.

]]

usb = require('serialusb')

do
	command = usb.readCommand()
	
	print("command: "..command)
	sleep(500)
until 1 == 0