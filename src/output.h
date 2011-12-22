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
    atomicServo[0] = (servo[0]-1000)/4;
    atomicServo[1] = (servo[1]-1000)/4;
}


void initializeServo() {
	DIGITAL_TILT_ROLL_PINMODE;
	DIGITAL_TILT_PITCH_PINMODE;
	TCCR0A = 0; // normal counting mode
	TIMSK0 |= (1<<OCIE0A); // Enable CTC interrupt
}


void initOutput() {
	initializeServo();
}


/*
 * XXX: Das muss ich noch verstehen, und vereinfachen für 2 Servos! Ist zur Zeit für 4 Servos!
 */
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
    DIGITAL_TILT_PITCH_HIGH;
    OCR0A+= 250; // 1000 us
    state++;
  } else if (state == 3) {
    OCR0A+= atomicServo[0]; // 1 // 1000 + [0-1020] us
    state++;
  } else if (state == 4) {
    DIGITAL_TILT_PITCH_LOW;
    DIGITAL_TILT_ROLL_HIGH;
    state++;
    OCR0A+= 250; // 1000 us
  } else if (state == 5) {
    OCR0A+= atomicServo[1]; // 2 // 1000 + [0-1020] us
    state++;
  } else if (state == 6) {
    DIGITAL_TILT_ROLL_LOW;
    state++;
    OCR0A+= 250; // 1000 us
  } else if (state == 7) {
    OCR0A+= 250; //atomicServo[3]; // 1000 + [0-1020] us
    state++;
  } else if (state == 8) {
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
