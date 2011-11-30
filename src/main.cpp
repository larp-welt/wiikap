/*
 * WiiKAP V0.1
 *
 *  Remote control for canon cameras, using CHDK on camera.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version. see <http://www.gnu.org/licenses/>
 *
 *  CopyLeft by Marcus J. Ertl (aka Tiggr)
 *  http://www.colorful-sky.de/
 */

#include <WProgram.h>
#include <Servo/Servo.h>
#include "signals.h"
#include "config.h"
#include "utils.h"
#include "RX.h"
#include "chdk.h"


static uint32_t currentTime = 0;
Servo panServo;
Servo tiltServo;


void setup()
{
	// setup serial communication for debugging
	Serial.begin(SERIAL_COM_SPEED);

	// output some informations on serial
	Serial.println("KAPcontroll - (c) 2012 by Marcus J. Ertl");
	Serial.println("http://www.colorful-sky.de/");
	Serial.println();
	Serial.print("[info]\t\tversion ");
	Serial.println(VERSION);

	// setup hardware
	Serial.println("[system]\tinit hardware");
	pinMode(CAMPIN, OUTPUT);
	pinMode(LEDPIN, OUTPUT);
	panServo.attach(PANSERVO);
	tiltServo.attach(TILTSERVO);

	// set RC
	Serial.println("[system]\tsetup receiver");
	configureReceiver();

	// setup Timer Interrupt
	Serial.println("[system]\tsetup timer");
	// gives me 13% processor time for irq ... maybe 100 will work too?
	resetCmdQueque();
	timerLoadValue = SetupTimer2(1000);

	// set camera defaults
	Serial.println("[chdk]\t\tset camera defaults");
	chdkSend(CHDK_EXP_ZERO);
	delay(DEADTIME);

	signalLed(SIG_STARTED);
	Serial.println("[status]\tstarted");
	Serial.println();
}


void loop()
{
	static uint32_t rcTime  = 0;
	static int exposure = 0;

	static int count = 50; // TTT

	currentTime = micros();

	if (currentTime > (rcTime + 20000) ) { // 50Hz = 20000
		rcTime = currentTime;
		computeRC();

/*
		// Exporsure Mode
		if (rcData[EXPOSURE] < 1300 && exposure != -1)
		{
			chdkSend(CHDK_EXP_MINUS);
			Serial.println("[chdk]\t\texposure minus");
			exposure = -1;
		}
		if (rcData[EXPOSURE] > 1700 && exposure != 1)
		{
			chdkSend(CHDK_EXP_PLUS);
			Serial.println("[chdk]\t\texposure plus");
			exposure = 1;
		}
		if (rcData[EXPOSURE] > 1300 && rcData[EXPOSURE] < 1700 && exposure != 0)
		{
			chdkSend(CHDK_EXP_ZERO);
			Serial.println("[chdk]\t\texposure zero");
			exposure = 0;
		}

		// Zoom
		if (rcData[ZOOM] < 1300)
		{
			chdkSend(CHDK_ZOOM_OUT);
			Serial.println("[chdk]\t\tzoom out");
		}
		if (rcData[ZOOM] > 1700)
		{
			chdkSend(CHDK_ZOOM_IN);
			Serial.println("[chdk]\t\tzoom in");
		}


		// Shoot
		if (rcData[SHOOT] < 1300 || rcData[SHOOT] > 1700)
		{
			chdkSend(CHDK_SHOOT);
			Serial.println("[chdk]\t\tshoot");
			signalLed(SIG_SHOOT);
		}
*/

		 // TTT
		const int values[] = {CHDK_ZOOM_IN, CHDK_ZOOM_OUT, CHDK_SHOOT, CHDK_EXP_MINUS, CHDK_EXP_ZERO, CHDK_EXP_PLUS};
		static int t = 0;
		if (!count--) {
			chdkSend(values[t]);
			count = 10;
			if (++t > 5) t=0;
		}

		// Servos
		#ifndef STABI
			// direct through
			panServo.writeMicroseconds(rcData[PAN]);
			tiltServo.writeMicroseconds(rcData[TILT]);
		#endif
		#ifdef STABI
			/* do something */
		#endif
	}
}
