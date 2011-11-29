/*
 * chdk.h
 *
 *  Created on: 28.11.2011
 *      Author: tiggr
 *
 *  After an idea of CHDKlover
 *  http://forum.chdk-treff.de/viewtopic.php?f=20&t=356
 */

#define CHDK_ZOOM_IN		"S.1.0.0"
#define CHDK_ZOOM_OUT		"S.0.1.0"
#define CHDK_SHOOT			"S.1.1.0"
#define CHDK_EXP_MINUS		"S.0.0.1"
#define CHDK_EXP_ZERO		"S.1.0.1"
#define CHDK_EXP_PLUS		"S.0.1.1"
#define CHDK_EMERGENCY		"S.1.1.1"
#define CHDK_NULL			"S.0.0.0"

#define S_PAUSE 30
#define S_START 70
#define S_LBIT 10
#define S_HBIT 40

#define S_SEND_BITS 	7
#define MAX_COMMANDS	20
#define DEADTIME		250  // min. time between to commands

#define TIMER_CLOCK_FREQ 2000000.0 //2MHz for /8 prescale from 16MHz


volatile char cmdQueque[MAX_COMMANDS][S_SEND_BITS] = {};
volatile int quequeLen = 0;
volatile int timerLoadValue;
volatile int latency;


void chdkSend(const char command[])
{
	static unsigned int last = 0;

	if (quequeLen < MAX_COMMANDS && micros()-DEADTIME > last)
	{
		for (int i=0; i<S_SEND_BITS; i++) cmdQueque[quequeLen][i] = command[i];
		quequeLen++;
	}
	last = micros();
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
	static int bitPos = 0;
	static int wait = 0;


	if (!wait)
	{
		if (quequeLen>0)
		{
			char bit = cmdQueque[0][bitPos];
			switch (bit)
			{
			case 'S':
				wait = S_START;
				digitalWrite(CAMPIN, HIGH);
				break;
			case '.':
				wait = S_PAUSE;
				digitalWrite(CAMPIN, LOW);
				break;
			case '0':
				wait = S_LBIT;
				digitalWrite(CAMPIN, HIGH);
				break;
			case '1':
				wait = S_HBIT;
				digitalWrite(CAMPIN, HIGH);
				break;
			}
			bitPos++;
		}

		if (quequeLen>0 && bitPos == S_SEND_BITS)
		{
			digitalWrite(CAMPIN, LOW);

			for (int i=1; i<quequeLen; i++)
			{
				for (int j=0; j<S_SEND_BITS; j++)
				{
					cmdQueque[i-1][j] = cmdQueque[i][j];
				}
			}
			quequeLen--;
			bitPos = 0;
		}
	} else {
		wait--;
	}
	latency=TCNT2;
	TCNT2=latency+timerLoadValue;
}
