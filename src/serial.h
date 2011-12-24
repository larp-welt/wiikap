#ifndef _serial_h_
#define _serial_h_

#include <Arduino.h>

static uint8_t writePosUART;
static uint8_t s[200];

void serialize16(int16_t a) {s[writePosUART++]  = a; s[writePosUART++]  = a>>8&0xff;}
void serialize8(uint8_t a)  {s[writePosUART++]  = a;}


void serialCom() {
  uint8_t i;

  if (Serial.available()) {
    switch (Serial.read()) {
    case 'M': // Multiwii @ arduino to GUI all data
      LEDPIN_TOGGLE;
      writePosUART=0;
      serialize8('M');
      serialize8(VERSION);  // WiiKAP Firmware version
      for(i=0;i<3;i++) serialize16(accSmooth[i]);
      for(i=0;i<3;i++) serialize16(gyroData[i]/8);

      for(i=0;i<3;i++) serialize16(servo[i]);
      for(i=0;i<8;i++) serialize16(rcData[i]);
      for(i=0;i<2;i++) serialize16(angle[i]/10);

      serialize8(P8);
      serialize8(I8);
      serialize8(D8);

      serialize8('M');
      Serial.write(s, writePosUART);
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
    case 'T':
      LEDPIN_TOGGLE;
      serialize8('T');
      serialize8('E');
      serialize8('S');
      serialize8('T');
      Serial.write(s, writePosUART);
      break;
    }
  }
}

#endif // _serial_h_
