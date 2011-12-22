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

/* TODO:
 * - Ausgabepins der Servos sauber definieren
 * - EEPROM
 * - Roll besser vorbereiten
 * - Konfiguration über Serial
 */

#include <WProgram.h>
#include "def.h"
#include "utils.h"
#include "EEPROM.h"
#include "RX.h"
#include "chdk.h"
#include "camcontroll.h"
#include "IMU.h"
#include "output.h"
//#include "serial.h"


void setup()
{
	// setup serial communication for debugging
	Serial.begin(SERIAL_COM_SPEED);
	Serial.flush();

	// setup hardware
	pinMode(CAMPIN, OUTPUT);
	pinMode(LEDPIN, OUTPUT);
	initOutput();
	POWERPIN_PINMODE;
	POWERPIN_OFF;

	checkFirstTime();
	initSensors();
	calibratingA = 400;
	calibratingG = 400;

	// set RC
	configureReceiver();

	// setup Timer Interrupt
	// XXX: gives me 13% processor time for irq ...
	resetCmdQueque();
	timerLoadValue = SetupTimer2(1000);

	// set camera defaults
	chdkSend(CHDK_EXP_ZERO);

	currentTime = micros();
}


void loop()
{
	static uint32_t rcTime  = 0;
	static uint8_t mode = MODE_RC;

	#ifdef AURICO
		const uint8_t auricoPrg[] = AURICO_PROGRAMM;
		static uint8_t auricoState = 0;
	#endif

	int16_t delta,deltaSum;
	int16_t PTerm,ITerm,DTerm;
	int16_t panPID;
	int16_t error;
	static int16_t errorGyroI = 0;
	static int16_t lastGyro = 0;
	static int16_t delta1,delta2;


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
			servo[PANAXIX] = rcData[PAN];
			servo[TILTAXIS] = rcData[TILT];
		#else
			computeIMU();

			rcTime = micros();

			//**** PITCH & ROLL & YAW PID ****
			if (abs(rcCommand[PAN])<350) error = rcCommand[PAN]*10*8/P8 ; //16 bits is needed for calculation: 350*10*8 = 28000      16 bits is ok for result if P8>2 (P>0.2)
			else error = (int32_t)rcCommand[PAN]*10*8/P8 ; //32 bits is needed for calculation: 500*5*10*8 = 200000   16 bits is ok for result if P8>2 (P>0.2)

			error -= gyroData[PANAXIS];

			PTerm = rcCommand[PAN];

			errorGyroI  = constrain(errorGyroI+error,-16000,+16000);          //WindUp //16 bits is ok here
			if (abs(gyroData[PANAXIS])>640) errorGyroI = 0;
			ITerm = (errorGyroI/125*I8)>>6;                                   //16 bits is ok here 16000/125 = 128 ; 128*250 = 32000


			if (abs(gyroData[PANAXIS])<160) PTerm -= gyroData[PANAXIS]*P8/10/8; //16 bits is needed for calculation   160*200 = 32000         16 bits is ok for result
			else PTerm -= (int32_t)gyroData[PANAXIS]*P8/10/8; //32 bits is needed for calculation

			delta    = gyroData[PANAXIS] - lastGyro;                               //16 bits is ok here, the dif between 2 consecutive gyro reads is limited to 800
			lastGyro = gyroData[PANAXIS];
			deltaSum = delta1+delta2+delta;
			delta2   = delta1;
			delta1   = delta;

			if (abs(deltaSum)<640) DTerm = (deltaSum*D8)>>5; //16 bits is needed for calculation 640*50 = 32000           16 bits is ok for result
			else DTerm = ((int32_t)deltaSum*D8)>>5;          //32 bits is needed for calculation

			panPID =  PTerm + ITerm - DTerm;

			servo[TILT] = constrain(TILT_MIDDLE + TILT_PROP * angle[TILTAXIS] /16 + rcCommand[TILT], TILT_MIN, TILT_MAX);
			servo[PAN]  = constrain(PAN_MIDDLE  + PAN_PROP  * panPID, PAN_MIN,  PAN_MAX); // XXX ???

			Serial.print("   Tilt: "); Serial.print(servo[TILT]);
			Serial.print("    Pan: "); Serial.print(servo[PAN]);
			Serial.print("  Angle: "); Serial.print(angle[TILTAXIS]/16);
			Serial.print("   Gyro: "); Serial.print(gyroData[PANAXIS]);
			Serial.print(" panPid: "); Serial.print(panPID);
			Serial.println();

			#ifdef ROLLSTABI
				servo[ROLL] = constrain(ROLL_MIDDLE + ROLL_PROP * angle[ROLLAXIS] /16 + RCROLL, ROLL_MIN, ROLL_MAX);
			#endif

			writeServos();
		#endif

	}
}
