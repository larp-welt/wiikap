#ifndef _serial_h_
#define _serial_h_

#include <WProgram.h>

static uint8_t point;
static uint8_t s[200];

void serialize16(int16_t a) {s[point++]  = a; s[point++]  = a>>8&0xff;}
void serialize8(uint8_t a)  {s[point++]  = a;}

// ***********************************
// Interrupt driven UART transmitter for MIS_OSD
// ***********************************
static uint8_t tx_ptr;
static uint8_t tx_busy = 0;

void ISR_UART() {
  UDR0 = s[tx_ptr++];           /* Transmit next byte */
  if ( tx_ptr == point ) {      /* Check if all data is transmitted */
    UCSR0B &= ~(1<<UDRIE0);     /* Disable transmitter UDRE interrupt */
    tx_busy = 0;
  }
}

void UartSendData() {          // start of the data block transmission
  cli();
  tx_ptr = 0;
  UCSR0A |= (1<<UDRE0);        /* Clear UDRE interrupt flag */
  UCSR0B |= (1<<UDRIE0);       /* Enable transmitter UDRE interrupt */
  UDR0 = s[tx_ptr++];          /* Start transmission */
  tx_busy = 1;
  sei();
}

void serialCom() {
  uint8_t i;

  if ((!tx_busy) && Serial.available()) {
    switch (Serial.read()) {
    case 'M': // Multiwii @ arduino to GUI all data
      point=0;
      serialize8('M');
      serialize8(VERSION);  // MultiWii Firmware version
      for(i=0;i<3;i++) serialize16(accSmooth[i]);
      for(i=0;i<3;i++) serialize16(gyroData[i]/8);

      for(i=0;i<3;i++) serialize16(servo[i]);
      for(i=0;i<8;i++) serialize16(rcData[i]);

      for(i=0;i<2;i++) serialize16(angle[i]/10);

      serialize8(P8);
      serialize8(I8);
      serialize8(D8);

      serialize8('M');
      UartSendData(); // Serial.write(s,point);
      break;
    case 'W': //GUI write params to eeprom @ arduino
      while (Serial.available()<3) {}

      P8 = Serial.read();
      I8 = Serial.read();
      D8 = Serial.read();

      writeParams();
      break;
    case 'S': //GUI to arduino ACC calibration request
      calibratingA=400;
      break;
    }
  }
}

#endif // _serial_h_
