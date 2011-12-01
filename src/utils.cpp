/*
 * utils.cpp
 *
 *  Created on: 01.12.2011
 *      Author: tiggr
 */

#include <string.h>
#include <WProgram.h>
#include "config.h"
#include "def.h"
#include "utils.h"

void signalLed(const char chrMorse[])
{
	for (unsigned int i = 0; i < strlen(chrMorse); i++)
	{
		LEDPIN_ON
		if (chrMorse[i] == '.') {
			delay(200);
		} else {
			delay(600);
		}
		LEDPIN_OFF
		delay(200);
	}
}
