/*
 * output.h
 *
 *  Created on: 08.12.2011
 *      Author: tiggr
 */

#ifndef OUTPUT_H_
#define OUTPUT_H_

#include <WProgram.h>
#include "def.h"


static int16_t servo[4] = {1500,1500};
static int16_t rcCommand[4]; // interval [1000;2000] for THROTTLE and [-500;+500] for ROLL/PITCH/YAW
volatile uint8_t atomicServo[3] = {125,125};


void writeServos() {
    atomicServo[PAN] = (servo[PAN]-1000)/4;
    atomicServo[TILT] = (servo[TILT]-1000)/4;
	#ifdef ROLLSTABI
		atomitServo[ROLL] = (servo[ROLL]-1000)/4;
	#endif
}


void initializeServo() {
	TILT_PINMODE;
	PAN_PINMODE;
	TCCR0A = 0; // normal counting mode
	TIMSK0 |= (1<<OCIE0A); // Enable CTC interrupt
}


void initOutput() {
	initializeServo();
}


// ****servo yaw with a 50Hz refresh rate****
// prescaler is set by default to 64 on Timer0
// Duemilanove : 16MHz / 64 => 4 us
// 256 steps = 1 counter cycle = 1024 us
// algorithm strategy:
// pulse high servo 0 -> do nothing for 1000 us -> do nothing for [0 to 1000] us -> pulse down servo 0
// pulse high servo 1 -> do nothing for 1000 us -> do nothing for [0 to 1000] us -> pulse down servo 1
// pulse high servo 2 -> do nothing for 1000 us -> do nothing for [0 to 1000] us -> pulse down servo 2
// pulse high servo 3 -> do nothing for 1000 us -> do nothing for [0 to 1000] us -> pulse down servo 3
// do nothing for 14 x 1000 us
// http://billgrundmann.wordpress.com/2009/03/03/to-use-or-not-use-writedigital/
ISR(TIMER0_COMPA_vect) {
  static uint8_t state = 0;
  static uint8_t count;
  if (state == 0) {
    //http://billgrundmann.wordpress.com/2009/03/03/to-use-or-not-use-writedigital/
    OCR0A+= 250; // 1000 us
    state++ ;
  } else if (state == 1) {
    OCR0A+= 250; //atomicServo[0]; // 1000 + [0-1020] us
    state++;
  } else if (state == 2) {
    PAN_HIGH;
    OCR0A+= 250; // 1000 us
    state++;
  } else if (state == 3) {
    OCR0A+= atomicServo[PAN]; // 1000 + [0-1020] us
    state++;
  } else if (state == 4) {
    PAN_LOW;
    TILT_HIGH;
    state++;
    OCR0A+= 250; // 1000 us
  } else if (state == 5) {
    OCR0A+= atomicServo[TILT]; // 1000 + [0-1020] us
    state++;
  } else if (state == 6) {
    TILT_LOW;
    state++;
    ROLL_HIGHT
    OCR0A+= 250; // 1000 us
  } else if (state == 7) {
	#ifdef ROLLSTABI
		OCR0A+= atomicServo[ROLL]; // 1000 + [0-1020] us
	#else
		OCR0A+= 250;
	#endif
    state++;
  } else if (state == 8) {
	ROLL_LOW
    count = 10; // 12 x 1000 us
    state++;
    OCR0A+= 250; // 1000 us
  } else if (state == 9) {
    if (count > 0) count--;
    else state = 0;
    OCR0A+= 250;
  }
}

#endif /* OUTPUT_H_ */
