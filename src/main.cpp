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
#include "blinkcodes.h"
#include "def.h"
#include "utils.h"
#include "RX.h"
#include "chdk.h"
#include "camcontroll.h"
#include "IMU.h"
#include "output.h"


#ifndef STABI
	#include <Servo/Servo.h>
	Servo panServo;
	Servo tiltServo;
#endif


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
	initOutput();
	#ifndef STABI
		panServo.attach(PANSERVO);
		tiltServo.attach(TILTSERVO);
	#endif
	POWERPIN_PINMODE;
	POWERPIN_OFF;

	Serial.println("[system]\tinit sensors");
	initSensors();
	calibratingA = 400;
	calibratingG = 400;

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

	Serial.println("[status]\tstarted");
	Serial.println();

	currentTime = micros();
}


void loop()
{
	const uint8_t auricoPrg[] = AURICO_PROGRAMM;
	static uint32_t rcTime  = 0;
	static uint8_t mode = MODE_RC;
	static uint8_t auricoState = 0;

	currentTime = micros();
	if (currentTime > rcTime ) { // 50Hz = 20000
		rcTime = currentTime + 20000;

		computeRC();

		camcontroll();

		if (mode == MODE_AURICO)
		{
			/* do all the aurico stuff
			 *
			 *   +--> reset tilt
			 *   |    shoot
			 *   |    tilt -> shoot
			 *   |    tilt -> shoot
			 *   |    tilt -> shoot
			 *   |    rotate
			 *   +--- start again
			 *
			 *
			 */



		}

		// Servos
		#ifndef STABI
			// direct through
			panServo.writeMicroseconds(rcData[PAN]);
			tiltServo.writeMicroseconds(rcData[TILT]);
		#else
			computeIMU();

			rcTime = micros();

			servo[TILTAXIS] = constrain(TILT_MIDDLE + TILT_PROP * angle[TILTAXIS] /16 + rcCommand[TILTAXIS], TILT_MIN, TILT_MAX);
			servo[ROLLAXIS] = constrain(ROLL_MIDDLE + ROLL_PROP * angle[ROLLAXIS] /16 + rcCommand[ROLLAXIS], ROLL_MIN, ROLL_MAX);

//			Serial.print("tilt: ");
//			Serial.print(servo[TILTAXIS]);
//			Serial.print("\troll: ");
//			Serial.print(servo[ROLLAXIS]);
//			Serial.println();

			writeServos();
		#endif

	}
}
