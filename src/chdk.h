/*
 * chdk.h
 *
 *  Created on: 28.11.2011
 *      Author: tiggr
 *
 *  After an idea of CHDKlover
 *  http://forum.chdk-treff.de/viewtopic.php?f=20&t=356
 */

#define CHDK_ZOOM_IN		"S.1.0.0-"  // 1
#define CHDK_ZOOM_OUT		"S.0.1.0-"  // 2
#define CHDK_SHOOT			"S.1.1.0-"  // 3
#define CHDK_EXP_MINUS		"S.0.0.1-"  // 4
#define CHDK_EXP_ZERO		"S.1.0.1-"  // 5
#define CHDK_EXP_PLUS		"S.0.1.1-"  // 6
#define CHDK_EMERGENCY		"S.1.1.1-"  // 7
#define CHDK_NULL			"S.0.0.0-"  // 8

#define S_PAUSE 300
#define S_START 700
#define S_LBIT 100
#define S_HBIT 400

#define S_SEND_BITS 	7
#define MAX_COMMANDS	128
#define DEADTIME		500  // min. time between to commands

#define TIMER_CLOCK_FREQ 2000000.0 //2MHz for /8 prescale from 16MHz


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
   

void chdkSend(const char command[])
{
    static unsigned int last = 0;

	if (micros()-DEADTIME > last)
	{
		for (int unsigned i=0; i<strlen(command); i++)
		{
		   if (command[i] == 'S')
		   {
			   cmdQueque[quequeWrite][0] = S_START;
			   cmdQueque[quequeWrite][1] = 1;
		   }
		   if (command[i] == '.')
		   {
			   cmdQueque[quequeWrite][0] = S_PAUSE;
			   cmdQueque[quequeWrite][1] = 0;
		   }
		   if (command[i] == '1')
		   {
			   cmdQueque[quequeWrite][0] = S_HBIT;
			   cmdQueque[quequeWrite][1] = 1;
		   }
		   if (command[i] == '0')
		   {
			   cmdQueque[quequeWrite][0] = S_LBIT;
			   cmdQueque[quequeWrite][1] = 1;
		   }
		   if (command[i] == '-')
		   {
			   cmdQueque[quequeWrite][0] = S_START;
			   cmdQueque[quequeWrite][1] = 0;
		   }

		   if (++quequeWrite >= MAX_COMMANDS) quequeWrite = 0;
		}
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
