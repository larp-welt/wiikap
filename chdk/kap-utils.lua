--[[
kap-utils.lua

Utility functions regular used inside my kap scripts.

Usage:
	require("kap-utils")

Compatibility:
	A570

]]

local kap = {}

function kap.debugMsg(level, message)
	-- prints message, if level is smaller or
	-- equal to global debugMode
	-- if global not set, do nothing!
	if debugMode == nil then debugMode = 0 end
	
	if level <= debugMode then
		print(message)
	end
end


function kap.hyperfocal(checkMode, loopWait, zoomWait)
	-- sets hyperfocal focus
	-- if checkmode <> 0 recheck aperture, takes some
	-- time - but may be useful in changing light situations.
	if checkMode == 0 then
		distance = get_hyp_dist()
		kap.debugMsg(4, "distance: "..distance)
		set_focus(distance)
	else
		press("shoot_half")
		repeat
			sleep(loopWait)
		until get_shooting() == true
		distance = get_hyp_dist()
		
		release("shoot_half")
		repeat
			sleep(loopWait)
		until get_shooting() ~= true
		kap.debugMsg(4, "distance: "..distance)
		set_focus(distance)
	end
	kap.debugMsg(3, "focus: "..(distance/1000).."m")
	kap.debugMsg(4, "near:  "..(get_near_limit()/1000).."m")
	sleep(zoomWait)
end


function kap.checkRange(test, value, diff)
	-- check if test is in range (value-diff...value+diff)
	return (test >= value-diff) and (test <= value+diff)
end


function kap.setMF(zoomWait)
	-- sets focus mode to MF
	-- works only on A570
	repeat
		click("down")
		sleep(zoomWait)
		focusMode = get_prop(6)
		kap.debugMsg(4, "focus mode: "..focusMode)
	until focusMode == 4
end


function kap.set_flash_mode(mode)
	-- stolen from
	-- http://forum.chdk-treff.de/viewtopic.php?f=7&t=800
	-- modes:
	--    0 - auto
	--    1 - on
	--    2 - off
	if get_propset() == 1 then
		set_prop(16, mode)
	else
		set_prop(143, mode)
	end
end


function kap.playSound(sound)
	-- Play sound 0 to 6, avoid annyoing sound #7
	-- Stay mute if sound number < 0
	if sound > 6 then
		sound = 3 -- default
		kap.debugMsg(2, "illegal sound")
	end

	if sound >= 0 then
		play_sound(sound)
	end
end


kap.debugMsg(5, "loaded successfully: kap-utils")
return kap
