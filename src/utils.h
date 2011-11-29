/*
 * utils.h
 *
 *  Created on: 28.11.2011
 *      Author: tiggr
 */

void signalLed(const char chrMorse[])
{
	for (unsigned int i = 0; i < strlen(chrMorse); i++)
	{
		digitalWrite(LEDPIN, HIGH);
		if (chrMorse[i] == '.') {
			delay(200);
		} else {
			delay(600);
		}
		digitalWrite(LEDPIN, LOW);
		delay(200);
	}
}
