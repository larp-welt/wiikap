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
#define MAX_COMMANDS	255
#define DEADTIME		250  // min. time between to commands

#define TIMER_CLOCK_FREQ 2000000.0 //2MHz for /8 prescale from 16MHz


volatile char cmdQueque[MAX_COMMANDS][2];
volatile int quequeWrite = 0;
volatile int quequeRead = 0;
volatile int last = 0;
volatile int timerLoadValue;
volatile int latency;


void initCmdQueque()
{
   for (int i=0; i<MAX_COMMANDS; i++) cmdQueque[i][0] = 0;
   quequeWrite = 0;
   quequeRead = 0;
}
   

void chdkSend(const char command[])
{
	if (micros()-DEADTIME > last)
	{
		for (int i=0; i<strlen(command); i++)
    {
       case command[i]
       {
       case 'S':
          cmdQueque[quequeWrite][0] = S_START;
          cmdQueque[quequeWrite][1] = 1;
          break;
       case '.':
          cmdQueque[quequeWrite][0] = S_PAUSE;
          cmdQueque[quequeWrite][1] = 0;
          break;
       case '1':
          cmdQueque[quequeWrite][0] = S_HBIT;
          cmdQueque[quequeWrite][1] = 1;
          break;
       case '0':
          cmdQueque[quequeWrite][0] = S_LBIT;
          cmdQueque[quequeWrite][1] = 1;
          break;
       }
       if (++quequeWrite = MAX_COMMANDS) quequeWrite = 0;
       cmdQueque[quequeWrite][0] = 0;
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


	if (!wait)
	{
    digitalWrite(CAMPIN, LOW);
    if (cmdQueque[quequeRead][0] != 0)
    {
       wait = cmdQueque[quequeRead][0];
       if (cmdQueque[quequeRead][1]) digitalWrite(CAMPIN, cmdQueque[quequeRead][1]);
       if (++quequeRead = MAX_COMMANDS) quequeRead = 0;
    }
  } else {
		wait--;
	}
	latency=TCNT2;
	TCNT2=latency+timerLoadValue;
}
