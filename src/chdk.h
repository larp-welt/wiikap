/*
 * chdk.h
 *
 *  Created on: 28.11.2011
 *      Author: tiggr
 *
 *  After an idea of CHDKlover
 *  http://forum.chdk-treff.de/viewtopic.php?f=20&t=356
 */

#ifndef CHDK_H_
#define CHDK_H_

#define CHDK_ZOOM_IN		2000
#define CHDK_ZOOM_OUT		2500
#define CHDK_SHOOT			500
#define CHDK_EXP_MINUS		4000
#define CHDK_EXP_ZERO		4500
#define CHDK_EXP_PLUS	    5000

#define S_PAUSE 300

#define MAX_COMMANDS	32
#define DEADTIME		250  // min. time between to commands

#define TIMER_CLOCK_FREQ 2000000.0 //2MHz for /8 prescale from 16MHz


extern volatile int cmdQueque[MAX_COMMANDS][2];
extern volatile int quequeWrite;
extern volatile int quequeRead;
extern volatile int timerLoadValue;
extern volatile int latency;


void resetCmdQueque();
void chdkSend(const int command);
unsigned char SetupTimer2(float timeoutFrequency);

#endif /* CHDK_H_ */
