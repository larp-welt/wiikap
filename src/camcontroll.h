/*
 * camcontroll.h
 *
 *  Created on: 14.12.2011
 *      Author: tiggr
 */

#ifndef CAMCONTROLL_H_
#define CAMCONTROLL_H_

#include "RX.h"
#include "chdk.h"

void camcontroll()
{
		static int exposure = 0;
		static int shootWait = 0;
		static int zoomWait = 0;

		// Exporsure Mode
		if (rcData[EXPOSURE] < 1300 && exposure != -1)
		{
			chdkSend(CHDK_EXP_MINUS);
			exposure = -1;
		}
		if (rcData[EXPOSURE] > 1600 && exposure != 1)
		{
			chdkSend(CHDK_EXP_PLUS);
			exposure = 1;
		}
		if (rcData[EXPOSURE] > 1300 && rcData[EXPOSURE] < 1600 && exposure != 0)
		{
			chdkSend(CHDK_EXP_ZERO);
			exposure = 0;
		}

		// Zoom
		if (rcData[ZOOM] < 1300 && zoomWait == 0)
		{
			chdkSend(CHDK_ZOOM_OUT);
			zoomWait = WAIT_ZOOM;
		}
		if (rcData[ZOOM] > 1600 && zoomWait == 0)
		{
			chdkSend(CHDK_ZOOM_IN);
			zoomWait = WAIT_ZOOM;
		}


		// Shoot
		if (rcData[SHOOT] < 1400 || rcData[SHOOT] > 1600)
		{
			if (shootWait == 0)
			{
				chdkSend(CHDK_SHOOT);
				shootWait = WAIT_SHOOT;
			}
		}

		if (shootWait > 0) shootWait--;
		if (zoomWait > 0) zoomWait--;
}

#endif /* CAMCONTROLL_H_ */
