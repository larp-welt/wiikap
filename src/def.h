#ifndef _DEF_H_
#define _DEF_H_

#include "config.h"

#define MODE_AURICO 0
#define MODE_RC 1

#define PROMINI

#if defined(PROMINI)
	#define LEDPIN_PINMODE             pinMode (13, OUTPUT);
	#define LEDPIN_TOGGLE              PINB |= 1<<5;     //switch LEDPIN state (digital PIN 13)
	#define LEDPIN_OFF                 PORTB &= ~(1<<5);
	#define LEDPIN_ON                  PORTB |= (1<<5);
	#define BUZZERPIN_PINMODE          pinMode (8, OUTPUT);
	#define BUZZERPIN_ON               PORTB |= 1;
	#define BUZZERPIN_OFF              PORTB &= ~1;
	#define POWERPIN_PINMODE           pinMode (12, OUTPUT);
	#define POWERPIN_ON                PORTB |= 1<<4;
	#define POWERPIN_OFF               PORTB &= ~(1<<4); //switch OFF WMP, digital PIN 12
	#define I2C_PULLUPS_ENABLE         PORTC |= 1<<4; PORTC |= 1<<5;   // PIN A4&A5 (SDA&SCL)
	#define I2C_PULLUPS_DISABLE        PORTC &= ~(1<<4); PORTC &= ~(1<<5);
	#define PAN_PINMODE 			   pinMode(A0,OUTPUT);
	#define PAN_HIGH    			   PORTC |= 1<<0;
	#define PAN_LOW     			   PORTC &= ~(1<<0);
	#define TILT_PINMODE  			   pinMode(A1,OUTPUT);
	#define TILT_HIGH     			   PORTC |= 1<<1;
	#define TILT_LOW      			   PORTC &= ~(1<<1);
	#ifdef ROLLSTABI
		#define ROLL_PINMODE		   ;
		#define ROLL_HIGHT			   ;
		#define ROLL_LOW			   ;
	#else
		#define ROLL_PINMODE           ;
		#define ROLL_HIGHT			   ;
		#define ROLL_LOW			   ;
	#endif
#endif

#endif /* _DEF_H_ */
