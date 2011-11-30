--[[
rem RC mit HYP
rem Hyperfocal-Code geklaut von
rem HDR-Hyper.bas by gehtnix  13.06.2010
rem
rem For Canon A570 only

@title L:RC with HYP

@param f Belichtung x1/3
@default -1

@param p Check aperture 0/1
@default p 0
@param s do/until Sleep x10
@default s 0
@param x Zoom Sleep x10
@default x 100

@param d Debug Mode  (0 ... 5)
@default d 0

@param e Sound (0..6, -1)
@default e 3
]]

-- better human readable parameter names
checkMode = p
loopWait = s*10
zoomWait = x*10
debugMode = d
sound = e
ev = f

kap = require('kap-utils')
usb = require('serialusb')


function thirds(x)
	-- converts int to fracture with base 3
	fract = ""
	if x%3 == 0 then
		fract = math.abs(x/3)
	else
		if x/3 ~= 0 then
			fract = math.abs(x/3).." "
		end
		fract = fract..math.abs(x%3).."/3"
	end
	if x < 0 then
		fract = "-"..fract
	end
	return fract
end


print("disable flash")
kap.set_flash_mode(2)

print("changing to MF and HYP")
kap.setMF(zoomWait)
kap.hyperfocal(checkMode, loopWait, zoomWait)

set_ev(ev*32)
print("set Ev = "..thirds(get_ev()/32))

print("ready for KAP! :-)")
kap.playSound(sound)

repeat
	command = usb.readCommand()

	kap.debugMsg(2, "command: "..command)

	if command == 3 then 
		shoot()
		kap.debugMsg(2, "shoot")
	elseif command == 2 then
		kap.debugMsg(2, "zoom out")
		click("zoom_out")
		sleep(zoomWait)
		kap.hyperfocal(checkMode, loopWait, zoomWait)
	elseif command == 1 then
		kap.debugMsg(2, "zoom in")
		click("zoom_in")
		sleep(zoomWait)
		kap.hyperfocal(checkMode, loopWait, zoomWait)
	-- TODO set exposure values (relativ to setting)
	end
until is_key("set") == true
