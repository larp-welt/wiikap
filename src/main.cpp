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
#include "blinkcodes.h"
#include "def.h"
#include "config.h"
#include "utils.h"
#include "RX.h"
#include "chdk.h"


static uint32_t currentTime = 0;
int mode = MODE_RC;
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
	// XXX: gives me 13% processor time for irq ...
	resetCmdQueque();
	timerLoadValue = SetupTimer2(1000);

	// set camera defaults
	Serial.println("[chdk]\t\tset camera defaults");
	chdkSend(CHDK_EXP_ZERO);

	signalLed(SIG_STARTED);
	Serial.println("[status]\tstarted");
	Serial.println();
}


void loop()
{
	static uint32_t rcTime  = 0;
	static int exposure = 0;
	static int shootWait = 0;
	static int zoomWait = 0;

	currentTime = micros();

	if (currentTime > (rcTime + 20000) ) { // 50Hz = 20000
		rcTime = currentTime;
		computeRC();

//		Serial.print(rcData[PAN]);
//		Serial.print(", ");
//		Serial.print(rcData[TILT]);
//		Serial.print(", ");
//		Serial.print(rcData[SHOOT]);
//		Serial.print(", ");
//		Serial.print(rcData[ZOOM]);
//		Serial.print(", ");
//		Serial.println(rcData[EXPOSURE]);


		// Exporsure Mode
		if (rcData[EXPOSURE] < 1300 && exposure != -1)
		{
			chdkSend(CHDK_EXP_MINUS);
			Serial.println("[chdk]\t\texposure: minus");
			exposure = -1;
		}
		if (rcData[EXPOSURE] > 1600 && exposure != 1)
		{
			chdkSend(CHDK_EXP_PLUS);
			Serial.println("[chdk]\t\texposure: plus");
			exposure = 1;
		}
		if (rcData[EXPOSURE] > 1300 && rcData[EXPOSURE] < 1600 && exposure != 0)
		{
			chdkSend(CHDK_EXP_ZERO);
			Serial.println("[chdk]\t\texposure: zero");
			exposure = 0;
		}

		// Zoom
		if (rcData[ZOOM] < 1300 && zoomWait == 0)
		{
			chdkSend(CHDK_ZOOM_OUT);
			Serial.println("[chdk]\t\tzoom out");
			zoomWait = WAIT_ZOOM;
		}
		if (rcData[ZOOM] > 1600 && zoomWait == 0)
		{
			chdkSend(CHDK_ZOOM_IN);
			Serial.println("[chdk]\t\tzoom in");
			zoomWait = WAIT_ZOOM;
		}


		// Shoot
		if (rcData[SHOOT] < 1400 || rcData[SHOOT] > 1600)
		{
			if (shootWait == 0)
			{
				chdkSend(CHDK_SHOOT);
				Serial.println("[chdk]\t\tshoot");
				signalLed(SIG_SHOOT);
				shootWait = WAIT_SHOOT;
			}
		}


		if (mode == MODE_AURICO)
		{
			/* do all the aurico stuff */
		}

		// Servos
		#ifndef STABI
			// direct through
			panServo.writeMicroseconds(rcData[PAN]);
			tiltServo.writeMicroseconds(rcData[TILT]);
		#else
			/* do something */
		#endif

		if (shootWait > 0) shootWait--;
		if (zoomWait > 0) zoomWait--;
	}
}
