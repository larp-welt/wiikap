/*
 * stolen code from MultiWii
 *
 * - removed all parts for sum signal
 * - removed all parts about failsafe
 * - removed all parts about Arduino Mega
 */

#define MAX_CHAN 8

static int16_t rcData[MAX_CHAN];    // interval [1000;2000]
static uint8_t pinRcChannel[MAX_CHAN]  = {TILTPIN, PANPIN, SHOOTPIN, ZOOMPIN, EXPOSUREPIN};
volatile uint16_t rcPinValue[MAX_CHAN] = {1500,1500,1500,1500,1500}; // interval [1000;2000]
static int16_t rcData4Values[MAX_CHAN][4];
static int16_t rcDataMean[MAX_CHAN] ;


// Configure each rc pin for PCINT
void configureReceiver() {
  for (uint8_t chan = 0; chan < MAX_CHAN; chan++)
    for (uint8_t a = 0; a < 4; a++)
      rcData4Values[chan][a] = 1500; //we initiate the default value of each channel. If there is no RC receiver connected, we will see those values
    // PCINT activated only for specific pin inside [D0-D7]  , [D2 D4 D5 D6 D7] for this multicopter
  PORTD   = (1<<2) | (1<<4) | (1<<5) | (1<<6) | (1<<7); //enable internal pull ups on the PINs of PORTD (no high impedence PINs)
  PCMSK2 |= (1<<2) | (1<<4) | (1<<5) | (1<<6) | (1<<7);
  PCICR   = 1<<2; // PCINT activated only for the port dealing with [D0-D7] PINs
}


ISR(PCINT2_vect) { //this ISR is common to every receiver channel, it is call everytime a change state occurs on a digital pin [D2-D7]
  uint8_t mask;
  uint8_t pin;
  uint16_t cTime,dTime;
  static uint16_t edgeTime[MAX_CHAN];
  static uint8_t PCintLast;

  pin = PIND;               // PIND indicates the state of each PIN for the arduino port dealing with [D0-D7] digital pins (8 bits variable)
  mask = pin ^ PCintLast;   // doing a ^ between the current interruption and the last one indicates wich pin changed
  sei();                    // re enable other interrupts at this point, the rest of this interrupt is not so time critical and can be interrupted safely
  PCintLast = pin;          // we memorize the current state of all PINs [D0-D7]

  cTime = micros();         // micros() return a uint32_t, but it is not usefull to keep the whole bits => we keep only 16 bits

  // mask is pins [D0-D7] that have changed
  // the principle is the same on the MEGA for PORTK and [A8-A15] PINs
  // chan = pin sequence of the port. chan begins at D2 and ends at D7
  if (mask & 1<<2) //indicates the bit 2 of the arduino port [D0-D7], that is to say digital pin 2, if 1 => this pin has just changed
	  {
	  if (!(pin & 1<<2)) {     // indicates if the bit 2 of the arduino port [D0-D7] is not at a high state (so that we match here only descending PPM pulse)
		  dTime = cTime-edgeTime[2]; if (900<dTime && dTime<2200) rcPinValue[2] = dTime; // just a verification: the value must be in the range [1000;2000] + some margin
	  } else {
		  edgeTime[2] = cTime; // if the bit 2 of the arduino port [D0-D7] is at a high state (ascending PPM pulse), we memorize the time
	  }
  }
  if (mask & 1<<4)
	  {
	  if (!(pin & 1<<4)) {
		  dTime = cTime-edgeTime[4]; if (900<dTime && dTime<2200) rcPinValue[4] = dTime; // just a verification: the value must be in the range [1000;2000] + some margin
	  } else {
		  edgeTime[4] = cTime;
	  }
  }
  if (mask & 1<<5)
	  {
	  if (!(pin & 1<<5)) {
		  dTime = cTime-edgeTime[5]; if (900<dTime && dTime<2200) rcPinValue[5] = dTime; // just a verification: the value must be in the range [1000;2000] + some margin
	  } else {
		  edgeTime[5] = cTime;
	  }
  }
  if (mask & 1<<6)
	  {
	  if (!(pin & 1<<6)) {
		  dTime = cTime-edgeTime[6]; if (900<dTime && dTime<2200) rcPinValue[6] = dTime; // just a verification: the value must be in the range [1000;2000] + some margin
	  } else {
		  edgeTime[6] = cTime;
	  }
  }
  if (mask & 1<<7)
	  {
	  if (!(pin & 1<<7)) {
		  dTime = cTime-edgeTime[7]; if (900<dTime && dTime<2200) rcPinValue[7] = dTime; // just a verification: the value must be in the range [1000;2000] + some margin
	  } else {
		  edgeTime[7] = cTime;
	  }
  }

}


uint16_t readRawRC(uint8_t chan) {
  uint16_t data;
  uint8_t oldSREG;

  oldSREG = SREG;
  cli(); // Let's disable interrupts
  data = rcPinValue[pinRcChannel[chan]]; // Let's copy the data Atomically
  SREG = oldSREG;
  sei(); // Let's enable the interrupts
  if (chan>4) return 1500; // XXX ???
  return data; // We return the value correctly copied when the IRQ's where disabled
}


void computeRC() {
  static uint8_t rc4ValuesIndex = 0;
  uint8_t chan,a;

  rc4ValuesIndex++;
  for (chan = 0; chan < MAX_CHAN; chan++) {
    rcData4Values[chan][rc4ValuesIndex%4] = readRawRC(chan);
    rcDataMean[chan] = 0;
    for (a=0;a<4;a++) rcDataMean[chan] += rcData4Values[chan][a];
    rcDataMean[chan]= (rcDataMean[chan]+2)/4;
    if ( rcDataMean[chan] < rcData[chan] -3)  rcData[chan] = rcDataMean[chan]+2;
    if ( rcDataMean[chan] > rcData[chan] +3)  rcData[chan] = rcDataMean[chan]-2;
  }
}
