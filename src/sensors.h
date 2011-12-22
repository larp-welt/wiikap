/*
 * Direct copy form MultiWiiCopter
 *
 * - removed all parts for compass
 * - removed all parts for barometer
 */

#ifndef _SENSORS_H_
#define _SENSORS_H_

#include <WProgram.h>
#include "def.h"

#define ROLLAXIS 0
#define TILTAXIS 1		// TILT
#define PANAXIS 2		// PAN

#define NEUTRALIZE_DELAY 100000

#if !defined(ACC_ORIENTATION)
  #define ACC_ORIENTATION(X, Y, Z)  {accADC[ROLLAXIS]  = X; accADC[TILTAXIS]  = Y; accADC[PANAXIS]  = Z;}
#endif
#if !defined(GYRO_ORIENTATION)
  #define GYRO_ORIENTATION(X, Y, Z) {gyroADC[ROLLAXIS] = X; gyroADC[TILTAXIS] = Y; gyroADC[PANAXIS] = Z;}
#endif

static uint16_t calibratingA = 0;  // the calibration is done is the main loop. Calibrating decreases at each cycle down to 0, then we enter in a normal mode.
static uint16_t calibratingG = 0;

static int16_t  acc_25deg;
static uint16_t acc_1G;
static int16_t  gyroADC[3],accADC[3];
static int16_t  accTrim[2] = {0, 0};
static int16_t gyroZero[3] = {0,0,0};
static int16_t accZero[3]  = {0,0,0};
static int16_t gyroData[3] = {0,0,0};
static int16_t angle[2]    = {0,0};  // absolute angle inclination in multiple of 0.1 degree    180 deg = 1800
static int8_t  smallAngle25 = 1;
static int16_t  accSmooth[3];       // projection of smoothed and normalized gravitation force vector on x/y/z axis, as measured by accelerometer

static uint32_t neutralizeTime = 0;
static int16_t  i2c_errors_count = 0;

uint8_t rawADC[6];


void i2c_init(void) {
  #if defined(INTERNAL_I2C_PULLUPS)
    I2C_PULLUPS_ENABLE
  #else
    I2C_PULLUPS_DISABLE
  #endif
  TWSR = 0;                                    // no prescaler => prescaler = 1
  TWBR = ((16000000L / I2C_SPEED) - 16) / 2;   // change the I2C clock rate
  TWCR = 1<<TWEN;                              // enable twi module, no interrupt
}


void waitTransmissionI2C() {
  uint16_t count = 255;
  while (!(TWCR & (1<<TWINT))) {
    count--;
    if (count==0) {              //we are in a blocking state => we don't insist
      TWCR = 0;                  //and we force a reset on TWINT register
      neutralizeTime = micros(); //we take a timestamp here to neutralize the value during a short delay
      i2c_errors_count++;
      break;
    }
  }
}


void i2c_rep_start(uint8_t address) {
  TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) ; // send REPEAT START condition
  waitTransmissionI2C();                       // wait until transmission completed
  TWDR = address;                              // send device address
  TWCR = (1<<TWINT) | (1<<TWEN);
  waitTransmissionI2C();                       // wail until transmission completed
}


void i2c_stop(void) {
  TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
  //  while(TWCR & (1<<TWSTO));                // <- can produce a blocking state with some WMP clones
}


void i2c_write(uint8_t data ) {
  TWDR = data;                                 // send data to the previously addressed device
  TWCR = (1<<TWINT) | (1<<TWEN);
  waitTransmissionI2C();
}


void i2c_writeReg(uint8_t add, uint8_t reg, uint8_t val) {
  i2c_rep_start(add+0);  // I2C write direction
  i2c_write(reg);        // register selection
  i2c_write(val);        // value to write in register
  i2c_stop();
}


uint8_t i2c_readNak(void) {
  TWCR = (1<<TWINT) | (1<<TWEN);
  waitTransmissionI2C();
  uint8_t r = TWDR;
  i2c_stop();
  return r;
}


uint8_t i2c_readAck() {
  TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
  waitTransmissionI2C();
  return TWDR;
}


uint8_t i2c_readReg(uint8_t add, uint8_t reg) {
  i2c_rep_start(add+0);  // I2C write direction
  i2c_write(reg);        // register selection
  i2c_rep_start(add+1);  // I2C read direction
  return i2c_readNak();  // Read single register and return value
}


void i2c_getSixRawADC(uint8_t add, uint8_t reg) {
  i2c_rep_start(add);
  i2c_write(reg);         // Start multiple read at the reg register
  i2c_rep_start(add +1);  // I2C read direction => I2C address + 1
  for(uint8_t i = 0; i < 5; i++)
    rawADC[i]=i2c_readAck();
  rawADC[5]= i2c_readNak();
}


// ****************
// GYRO common part
// ****************
void GYRO_Common() {
  static int16_t previousGyroADC[3] = {0,0,0};
  static int32_t g[3];
  uint8_t axis;

  if (calibratingG>0) {
    for (axis = 0; axis < 3; axis++) {
      // Reset g[axis] at start of calibration
      if (calibratingG == 400) g[axis]=0;
      // Sum up 400 readings
      g[axis] +=gyroADC[axis];
      // Clear global variables for next reading
      gyroADC[axis]=0;
      gyroZero[axis]=0;
      if (calibratingG == 1) gyroZero[axis]=g[axis]/400;
    }
    calibratingG--;
  }
  for (axis = 0; axis < 3; axis++) {
    gyroADC[axis]  -= gyroZero[axis];
    //anti gyro glitch, limit the variation between two consecutive readings
    gyroADC[axis] = constrain(gyroADC[axis],previousGyroADC[axis]-800,previousGyroADC[axis]+800);
    previousGyroADC[axis] = gyroADC[axis];
  }
}

// ****************
// ACC common part
// ****************
void ACC_Common() {
  static int32_t a[3];

  if (calibratingA>0) {
    for (uint8_t axis = 0; axis < 3; axis++) {
      // Reset a[axis] at start of calibration
      if (calibratingA == 400) a[axis]=0;
      // Sum up 400 readings
      a[axis] +=accADC[axis];
      // Clear global variables for next reading
      accADC[axis]=0;
      accZero[axis]=0;
    }
    // Calculate average, shift Z down by acc_1G
    if (calibratingA == 1) {
      accZero[ROLLAXIS]  = a[ROLLAXIS]/400;
      accZero[TILTAXIS] = a[TILTAXIS]/400;
      accZero[PANAXIS]   = a[PANAXIS]/400-acc_1G; // for nunchuk 200=1G
      accTrim[ROLLAXIS]   = 0;
      accTrim[TILTAXIS]  = 0;
    }
    calibratingA--;
  }
  accADC[ROLLAXIS]  -=  accZero[ROLLAXIS] ;
  accADC[TILTAXIS] -=  accZero[TILTAXIS];
  accADC[PANAXIS]   -=  accZero[PANAXIS] ;
}


void ACC_getADC(){
  TWBR = ((16000000L / 400000L) - 16) / 2;
  i2c_getSixRawADC(0x70,0x02);
  ACC_ORIENTATION(    ((rawADC[1]<<8) | rawADC[0])/64 ,
                      ((rawADC[3]<<8) | rawADC[2])/64 ,
                      ((rawADC[5]<<8) | rawADC[4])/64 );
  ACC_Common();
}


uint8_t WMP_getRawADC() {
  uint8_t axis;
  TWBR = ((16000000L / I2C_SPEED) - 16) / 2; // change the I2C clock rate
  i2c_getSixRawADC(0xA4,0x00);

  if (micros() < (neutralizeTime + NEUTRALIZE_DELAY)) {//we neutralize data in case of blocking+hard reset state
    for (axis = 0; axis < 3; axis++) {gyroADC[axis]=0;accADC[axis]=0;}
    accADC[PANAXIS] = acc_1G;
    return 1;
  }

  // Wii Motion Plus Data
  if ( (rawADC[5]&0x03) == 0x02 ) {
    // Assemble 14bit data
    gyroADC[ROLLAXIS]  = - ( ((rawADC[5]>>2)<<8) | rawADC[2] ); //range: +/- 8192
    gyroADC[TILTAXIS] = - ( ((rawADC[4]>>2)<<8) | rawADC[1] );
    gyroADC[PANAXIS]  =  - ( ((rawADC[3]>>2)<<8) | rawADC[0] );
    GYRO_Common();
    // Check if slow bit is set and normalize to fast mode range
    gyroADC[ROLLAXIS]  = (rawADC[3]&0x01)    ? gyroADC[ROLLAXIS]/5 : gyroADC[ROLLAXIS];  //the ratio 1/5 is not exactly the IDG600 or ISZ650 specification
    gyroADC[TILTAXIS] = (rawADC[4]&0x02)>>1  ? gyroADC[TILTAXIS]/5 : gyroADC[TILTAXIS];  //we detect here the slow of fast mode WMP gyros values (see wiibrew for more details)
    gyroADC[PANAXIS]   = (rawADC[3]&0x02)>>1 ? gyroADC[PANAXIS]/5  : gyroADC[PANAXIS];   // this step must be done after zero compensation
    return 1;
  } else if ( (rawADC[5]&0x03) == 0x00 ) { // Nunchuk Data
    ACC_ORIENTATION(  ( (rawADC[3]<<2)      | ((rawADC[5]>>4)&0x02) ) ,
                    - ( (rawADC[2]<<2)      | ((rawADC[5]>>3)&0x02) ) ,
                      ( ((rawADC[4]>>1)<<3) | ((rawADC[5]>>5)&0x06) ) );
    ACC_Common();
    return 0;
  } else return 2;
}


void WMP_init() {
  delay(250);
  i2c_writeReg(0xA6, 0xF0, 0x55); // Initialize Extension
  delay(250);
  i2c_writeReg(0xA6, 0xFE, 0x05); // Activate Nunchuck pass-through mode
  delay(250);
}


// BMA020
void ACC_init(){
  i2c_writeReg(0x70,0x15,0x80);
  uint8_t control = i2c_readReg(0x70, 0x14);
  control = control & 0xE0;
  control = control | (0x00 << 3); //Range 2G 00
  control = control | 0x00;        //Bandwidth 25 Hz 000
  i2c_writeReg(0x70,0x14,control);
  acc_1G = 255;
}


void initSensors() {
  delay(200);
  POWERPIN_ON;
  delay(100);
  i2c_init();
  delay(100);
  WMP_init();
  ACC_init();
  acc_25deg = acc_1G * 0.423;
}

#endif // _SENSORS_H_
