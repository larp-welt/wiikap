--[[
rem RC mit HYP
rem Hyperfocal-Code geklaut von
rem HDR-Hyper.bas by gehtnix  13.06.2010
rem
rem For Canon A570 only

@title L:WiiKAP

@param f Belichtung x1/3
@default -1

@param p Check aperture 0/1
@default p 0
@param s do/until Sleep x10
@default s 2
@param x Zoom Sleep x10
@default x 150

@param d Debug Mode  (0 ... 5)
@default d 0

@param e Sound (0..6, -1)
@default e 3

@param r Range (>=0)
@default r 2
]]

-- better human readable parameter names
checkMode = p
loopWait = s*10
zoomWait = x*10
debugMode = d
sound = e
ev = f
range = r

kap = require('kap-utils')

-- const

CHDK_ZOOM_IN = 20
CHDK_ZOOM_OUT = 25
CHDK_SHOOT = 5
CHDK_EXP_MINUS = 40
CHDK_EXP_ZERO = 45
CHDK_EXP_PLUS = 50


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
	repeat
		command = get_usb_power()
	until command > 0

	kap.debugMsg(2, "command: "..command)

	if kap.checkRange(command, CHDK_SHOOT, range) then 
		shoot()
		kap.debugMsg(2, "shoot")
	elseif kap.checkRange(command, CHDK_ZOOM_OUT, range) then
		kap.debugMsg(2, "zoom out")
		click("zoom_out")
		sleep(zoomWait)
		kap.hyperfocal(checkMode, loopWait, zoomWait)
	elseif kap.checkRange(command, CHDK_ZOOM_IN, range) then
		kap.debugMsg(2, "zoom in")
		click("zoom_in")
		sleep(zoomWait)
		kap.hyperfocal(checkMode, loopWait, zoomWait)
	elseif kap.checkRange(command, CHDK_EXP_MINUS, range) then
		set_ev((ev-1)*32)
		print("set Ev = "..thirds(get_ev()/32))
	elseif kap.checkRange(command, CHDK_EXP_PLUS, range) then
		set_ev((ev+1)*32)
		print("set Ev = "..thirds(get_ev()/32))
	elseif kap.checkRange(command, CHDK_EXP_ZERO, range) then
		set_ev(ev*32)
		print("set Ev = "..thirds(get_ev()/32))
	end
until is_key("set") == true
