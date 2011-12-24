/*
 * chdk.h
 *
 *  Created on: 28.11.2011
 *      Author: tiggr
 *
 *  After an idea of CHDKlover
 *  http://forum.chdk-treff.de/viewtopic.php?f=20&t=356
 */

#include <string.h>
#include <Arduino.h>
#include "config.h"
#include "chdk.h"


volatile int cmdQueque[MAX_COMMANDS][2];
volatile int quequeWrite = 0;
volatile int quequeRead = 0;
volatile int timerLoadValue;
volatile int latency;


void resetCmdQueque()
{
   quequeWrite = 0;
   quequeRead = 0;
}


void chdkSend(const int command)
{
    static unsigned int last = 0;

	if (micros()-DEADTIME > last)
	{
		   cmdQueque[quequeWrite][0] = command;
		   cmdQueque[quequeWrite][1] = 1;
		   if (++quequeWrite >= MAX_COMMANDS) quequeWrite = 0;

		   cmdQueque[quequeWrite][0] = S_PAUSE;
		   cmdQueque[quequeWrite][1] = 0;
		   if (++quequeWrite >= MAX_COMMANDS) quequeWrite = 0;

		   last = micros();
	}
}


unsigned char SetupTimer2(float timeoutFrequency){
	//http://www.uchobby.com/index.php/2007/11/24/arduino-interrupts/
	unsigned char result; //The timer load value.

	//Calculate the timer load value
	result=(int)((257.0-(TIMER_CLOCK_FREQ/timeoutFrequency))+0.5);
	//The 257 really should be 256 but I get better results with 257.

	//Timer2 Settings: Timer Prescaler /8, mode 0
	//Timer clock = 16MHz/8 = 2Mhz or 0.5us
	//The /8 prescale gives us a good range to work with
	//so we just hard code this for now.
	TCCR2A = 0;
	TCCR2B = 0<<CS22 | 1<<CS21 | 0<<CS20;

	//Timer2 Overflow Interrupt Enable
	TIMSK2 = 1<<TOIE2;

	//load the timer for its first cycle
	TCNT2=result;

	return(result);
}


ISR(TIMER2_OVF_vect) {
	static int wait = 0;
	static int state = 1;

	if (!wait)
	{
		if (state) digitalWrite(CAMPIN, LOW);
		if (quequeRead != quequeWrite)
		{
			wait = cmdQueque[quequeRead][0];
			state = cmdQueque[quequeRead][1];
			if (state) digitalWrite(CAMPIN, HIGH);
			if (++quequeRead >= MAX_COMMANDS) quequeRead = 0;
		}
	} else {
		wait--;
	}
	latency=TCNT2;
	TCNT2=latency+timerLoadValue;
}
