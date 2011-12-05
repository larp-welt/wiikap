/*
 * Direct copy form MultiWiiCopter
 *
 * - removed all parts for compass
 * - removed all parts for barometer
 */

#include <WProgram.h>
#include "config.h"
#include "def.h"
#include "sensors.h"

uint8_t rawADC[6];
uint8_t  nunchuk = 0;
uint32_t neutralizeTime = 0;
uint16_t acc_1G;             // this is the 1G measured acceleration
int16_t  acc_25deg;
int16_t  gyroADC[3],accADC[3];
int16_t  accTrim[2] = {0, 0};
int16_t gyroZero[3] = {0,0,0};
int16_t accZero[3]  = {0,0,0};
uint16_t calibratingA = 0;  // the calibration is done is the main loop. Calibrating decreases at each cycle down to 0, then we enter in a normal mode.
uint16_t calibratingG;
int16_t  i2c_errors_count = 0;
  
// ************************************************************************************************************
// I2C general functions
// ************************************************************************************************************

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

uint8_t i2c_readAck() {
  TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
  waitTransmissionI2C();
  return TWDR;
}

uint8_t i2c_readNak(void) {
  TWCR = (1<<TWINT) | (1<<TWEN);
  waitTransmissionI2C();
  uint8_t r = TWDR;
  i2c_stop();
  return r;
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

void i2c_getSixRawADC(uint8_t add, uint8_t reg) {
  i2c_rep_start(add);
  i2c_write(reg);         // Start multiple read at the reg register
  i2c_rep_start(add +1);  // I2C read direction => I2C address + 1
  for(uint8_t i = 0; i < 5; i++)
    rawADC[i]=i2c_readAck();
  rawADC[5]= i2c_readNak();
}

void i2c_writeReg(uint8_t add, uint8_t reg, uint8_t val) {
  i2c_rep_start(add+0);  // I2C write direction
  i2c_write(reg);        // register selection
  i2c_write(val);        // value to write in register
  i2c_stop();
}

uint8_t i2c_readReg(uint8_t add, uint8_t reg) {
  i2c_rep_start(add+0);  // I2C write direction
  i2c_write(reg);        // register selection
  i2c_rep_start(add+1);  // I2C read direction
  return i2c_readNak();  // Read single register and return value
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
      if (calibratingG == 1) {
        gyroZero[axis]=g[axis]/400;
      }
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
    // Calculate average, shift Z down by acc_1G and store values in EEPROM at end of calibration
    if (calibratingA == 1) {
      accZero[PAN] = a[PAN]/400;
      accZero[TILT]   = a[TILT]/400-acc_1G; // for nunchuk 200=1G
      accTrim[PAN]  = 0;
      // writeParams(); // XXX write accZero in EEPROM
    }
    calibratingA--;
  }
  accADC[PAN] -=  accZero[PAN];
  accADC[TILT]   -=  accZero[TILT] ;
}


// ************************************************************************************************************
// I2C Accelerometer ADXL345 
// ************************************************************************************************************
// I2C adress: 0x3A (8bit)    0x1D (7bit)
// Resolution: 10bit (Full range - 14bit, but this is autoscaling 10bit ADC to the range +- 16g)
// principle:
//  1) CS PIN must be linked to VCC to select the I2C mode
//  2) SD0 PIN must be linked to VCC to select the right I2C adress
//  3) bit  b00000100 must be set on register 0x2D to read data (only once at the initialization)
//  4) bits b00001011 must be set on register 0x31 to select the data format (only once at the initialization)
// ************************************************************************************************************
#if defined(ADXL345)
void ACC_init () {
  delay(10);
  i2c_writeReg(ADXL345_ADDRESS,0x2D,1<<3); //  register: Power CTRL  -- value: Set measure bit 3 on
  i2c_writeReg(ADXL345_ADDRESS,0x31,0x0B); //  register: DATA_FORMAT -- value: Set bits 3(full range) and 1 0 on (+/- 16g-range)
  i2c_writeReg(ADXL345_ADDRESS,0x2C,8+2+1); // register: BW_RATE     -- value: 200Hz sampling (see table 5 of the spec)
  acc_1G = 256;
}

void ACC_getADC () {
  TWBR = ((16000000L / 400000L) - 16) / 2; // change the I2C clock rate to 400kHz, ADXL435 is ok with this speed
  i2c_getSixRawADC(ADXL345_ADDRESS,0x32);

  ACC_ORIENTATION( - ((rawADC[3]<<8) | rawADC[2]) ,
                     ((rawADC[1]<<8) | rawADC[0]) ,
                     ((rawADC[5]<<8) | rawADC[4]) );
  ACC_Common();
}
#endif

// ************************************************************************************************************
// contribution initially from opie11 (rc-groups)
// adaptation from C2po (may 2011)
// contribution from ziss_dm (June 2011)
// contribution from ToLuSe (Jully 2011)
// I2C Accelerometer BMA180
// ************************************************************************************************************
// I2C adress: 0x80 (8bit)    0x40 (7bit) (SDO connection to VCC) 
// I2C adress: 0x82 (8bit)    0x41 (7bit) (SDO connection to VDDIO)
// Resolution: 14bit
//
// Control registers:
//
// 0x20    bw_tcs:   |                                           bw<3:0> |                        tcs<3:0> |
//                   |                                             150Hz |                 !!Calibration!! |
// ************************************************************************************************************
#if defined(BMA180)
void ACC_init () {
  delay(10);
  //default range 2G: 1G = 4096 unit.
  i2c_writeReg(BMA180_ADDRESS,0x0D,1<<4); // register: ctrl_reg0  -- value: set bit ee_w to 1 to enable writing
  delay(5);
  uint8_t control = i2c_readReg(BMA180_ADDRESS, 0x20);
  control = control & 0x0F; // register: bw_tcs reg: bits 4-7 to set bw -- value: set low pass filter to 10Hz (bits value = 0000xxxx)
  control = control | 0x00; 
  i2c_writeReg(BMA180_ADDRESS, 0x20, control);
  delay(5); 
  control = i2c_readReg(BMA180_ADDRESS, 0x30);
  control = control & 0xFC; 
  control = control | 0x02; 
  i2c_writeReg(BMA180_ADDRESS, 0x30, control);
  delay(5); 
  acc_1G = 512;
}

void ACC_getADC () {
  TWBR = ((16000000L / 400000L) - 16) / 2;  // Optional line.  Sensor is good for it in the spec.
  i2c_getSixRawADC(BMA180_ADDRESS,0x02);
  //usefull info is on the 14 bits  [2-15] bits  /4 => [0-13] bits  /8 => 11 bit resolution
  ACC_ORIENTATION(  - ((rawADC[1]<<8) | rawADC[0])/32 ,
                    - ((rawADC[3]<<8) | rawADC[2])/32 ,
                      ((rawADC[5]<<8) | rawADC[4])/32 );
  ACC_Common();
}
#endif

// ************************************************************************************************************
// contribution from Point65 and mgros (rc-groups)
// contribution from ziss_dm (June 2011)
// contribution from ToLuSe (Jully 2011)
// I2C Accelerometer BMA020
// ************************************************************************************************************
// I2C adress: 0x70 (8bit)
// Resolution: 10bit
// Control registers:
//
// Datasheet: After power on reset or soft reset it is recommended to set the SPI4-bit to the correct value.
//            0x80 = SPI four-wire = Default setting
// | 0x15: | SPI4 | enable_adv_INT | new_data_INT | latch_INT | shadow_dis | wake_up_pause<1:0> | wake_up |
// |       |    1 |              0 |            0 |         0 |          0 |                 00 |       0 |
//
// | 0x14: |                       reserved <2:0> |            range <1:0> |               bandwith <2:0> |
// |       |                      !!Calibration!! |                     2g |                         25Hz |
//
// ************************************************************************************************************
#ifdef BMA020
void ACC_init(){
  i2c_writeReg(0x70,0x15,0x80);
  uint8_t control = i2c_readReg(0x70, 0x14);
  control = control & 0xE0;
  control = control | (0x00 << 3); //Range 2G 00
  control = control | 0x00;        //Bandwidth 25 Hz 000
  i2c_writeReg(0x70,0x14,control); 
  acc_1G = 255;
}

void ACC_getADC(){
  TWBR = ((16000000L / 400000L) - 16) / 2;
  i2c_getSixRawADC(0x70,0x02);
  ACC_ORIENTATION(    ((rawADC[1]<<8) | rawADC[0])/64 ,
                      ((rawADC[3]<<8) | rawADC[2])/64 ,
                      ((rawADC[5]<<8) | rawADC[4])/64 );
  ACC_Common();
}
#endif

// ************************************************************************************************************
// standalone I2C Nunchuk
// ************************************************************************************************************
#if defined(NUNCHACK)
void ACC_init() {
  i2c_writeReg(0xA4 ,0xF0 ,0x55 );
  i2c_writeReg(0xA4 ,0xFB ,0x00 );
  delay(250);
  acc_1G = 200;
}

void ACC_getADC() {
  TWBR = ((16000000L / I2C_SPEED) - 16) / 2; // change the I2C clock rate. !! you must check if the nunchuk is ok with this freq
  i2c_getSixRawADC(0xA4,0x00);

  ACC_ORIENTATION(  ( (rawADC[3]<<2)        + ((rawADC[5]>>4)&0x2) ) ,
                  - ( (rawADC[2]<<2)        + ((rawADC[5]>>3)&0x2) ) ,
                    ( ((rawADC[4]&0xFE)<<2) + ((rawADC[5]>>5)&0x6) ));
  ACC_Common();
}
#endif

// ************************************************************************
// LIS3LV02 I2C Accelerometer
//contribution from adver (http://multiwii.com/forum/viewtopic.php?f=8&t=451)
// ************************************************************************
#if defined(LIS3LV02)
#define LIS3A  0x3A // I2C adress: 0x3A (8bit)

void i2c_ACC_init(){
  i2c_writeReg(LIS3A ,0x20 ,0xD7 ); // CTRL_REG1   1101 0111 Pwr on, 160Hz 
  i2c_writeReg(LIS3A ,0x21 ,0x50 ); // CTRL_REG2   0100 0000 Littl endian, 12 Bit, Boot
  acc_1G = 256;
}

void i2c_ACC_getADC(){
  TWBR = ((16000000L / 400000L) - 16) / 2; // change the I2C clock rate to 400kHz
  i2c_getSixRawADC(LIS3A,0x28+0x80);
  ACC_ORIENTATION(  (rawADC[3]<<8 | rawADC[2])/4 ,
                   -(rawADC[1]<<8 | rawADC[0])/4 ,
                   -(rawADC[5]<<8 | rawADC[4])/4);
  ACC_Common();
}
#endif

// ************************************************************************************************************
// I2C Accelerometer LSM303DLx
// contribution from wektorx (http://www.multiwii.com/forum/viewtopic.php?f=8&t=863)
// ************************************************************************************************************
#if defined(LSM303DLx_ACC)
void ACC_init () {
  delay(10);
  i2c_writeReg(0x30,0x20,0x27);
  i2c_writeReg(0x30,0x23,0x30);
  i2c_writeReg(0x30,0x21,0x00);

  acc_1G = 256;
}

  void ACC_getADC () {
  TWBR = ((16000000L / 400000L) - 16) / 2;
  i2c_getSixRawADC(0x30,0xA8);

  ACC_ORIENTATION( - ((rawADC[3]<<8) | rawADC[2])/16 ,
                     ((rawADC[1]<<8) | rawADC[0])/16 ,
                     ((rawADC[5]<<8) | rawADC[4])/16 );
  ACC_Common();
}
#endif

// ************************************************************************************************************
// ADC ACC
// ************************************************************************************************************
#if defined(ADCACC)
void ACC_init(){
  pinMode(A1,INPUT);
  pinMode(A2,INPUT);
  pinMode(A3,INPUT);
  acc_1G = 75;
}

void ACC_getADC() {
  ACC_ORIENTATION( -analogRead(A1) ,
                   -analogRead(A2) ,
                    analogRead(A3) );
  ACC_Common();
}
#endif

// ************************************************************************************************************
// contribution from Ciskje
// I2C Gyroscope L3G4200D 
// ************************************************************************************************************
#if defined(L3G4200D)
void Gyro_init() {
  delay(100);
  i2c_writeReg(0XD2+0 ,0x20 ,0x8F ); // CTRL_REG1   400Hz ODR, 20hz filter, run!
  delay(5);
  i2c_writeReg(0XD2+0 ,0x24 ,0x02 ); // CTRL_REG5   low pass filter enable
}

void Gyro_getADC () {
  TWBR = ((16000000L / 400000L) - 16) / 2; // change the I2C clock rate to 400kHz
  i2c_getSixRawADC(0XD2,0x80|0x28);

  GYRO_ORIENTATION(  ((rawADC[1]<<8) | rawADC[0])/20  ,
                     ((rawADC[3]<<8) | rawADC[2])/20  ,
                    -((rawADC[5]<<8) | rawADC[4])/20  );
  GYRO_Common();
}
#endif

// ************************************************************************************************************
// I2C Gyroscope ITG3200 
// ************************************************************************************************************
// I2C adress: 0xD2 (8bit)   0x69 (7bit)
// I2C adress: 0xD0 (8bit)   0x68 (7bit)
// principle:
// 1) VIO is connected to VDD
// 2) I2C adress is set to 0x69 (AD0 PIN connected to VDD)
// or 2) I2C adress is set to 0x68 (AD0 PIN connected to GND)
// 3) sample rate = 1000Hz ( 1kHz/(div+1) )
// ************************************************************************************************************
#if defined(ITG3200)
void Gyro_init() {
  delay(100);
  i2c_writeReg(ITG3200_ADDRESS, 0x3E, 0x80); //register: Power Management  --  value: reset device
//  delay(5);
//  i2c_writeReg(ITG3200_ADDRESS, 0x15, ITG3200_SMPLRT_DIV); //register: Sample Rate Divider  -- default value = 0: OK
  delay(5);
  i2c_writeReg(ITG3200_ADDRESS, 0x16, 0x18 + ITG3200_DLPF_CFG); //register: DLPF_CFG - low pass filter configuration
  delay(5);
  i2c_writeReg(ITG3200_ADDRESS, 0x3E, 0x03); //register: Power Management  --  value: PLL with Z Gyro reference
  delay(100);
}

void Gyro_getADC () {
  TWBR = ((16000000L / 400000L) - 16) / 2; // change the I2C clock rate to 400kHz
  i2c_getSixRawADC(ITG3200_ADDRESS,0X1D);
  GYRO_ORIENTATION(  + ( ((rawADC[2]<<8) | rawADC[3])/4) , // range: +/- 8192; +/- 2000 deg/sec
                     - ( ((rawADC[0]<<8) | rawADC[1])/4 ) ,
                     - ( ((rawADC[4]<<8) | rawADC[5])/4 ) );
  GYRO_Common();
}
#endif

#if !GYRO
// ************************************************************************************************************
// I2C Wii Motion Plus + optional Nunchuk
// ************************************************************************************************************
// I2C adress 1: 0xA6 (8bit)    0x53 (7bit)
// I2C adress 2: 0xA4 (8bit)    0x52 (7bit)
// ************************************************************************************************************
void WMP_init() {
  delay(250);
  i2c_writeReg(0xA6, 0xF0, 0x55); // Initialize Extension
  delay(250);
  i2c_writeReg(0xA6, 0xFE, 0x05); // Activate Nunchuck pass-through mode
  delay(250);

  // We need to set acc_1G for the Nunchuk beforehand; It's used in WMP_getRawADC() and ACC_Common()
  // If a different accelerometer is used, it will be overwritten by its ACC_init() later.
  acc_1G = 200;
  acc_25deg = acc_1G * 0.423;
  uint8_t numberAccRead = 0;
  // Read from WMP 100 times, this should return alternating WMP and Nunchuk data
  for(uint8_t i=0;i<100;i++) {
    delay(4);
    if (WMP_getRawADC() == 0) numberAccRead++; // Count number of times we read from the Nunchuk extension
  }
  // If we got at least 25 Nunchuck reads, we assume the Nunchuk is present
  if (numberAccRead>25)
    nunchuk = 1;
  delay(10);
}

uint8_t WMP_getRawADC() {
  uint8_t axis;
  TWBR = ((16000000L / I2C_SPEED) - 16) / 2; // change the I2C clock rate
  i2c_getSixRawADC(0xA4,0x00);

  if (micros() < (neutralizeTime + NEUTRALIZE_DELAY)) {//we neutralize data in case of blocking+hard reset state
    for (axis = 0; axis < 3; axis++) {gyroADC[axis]=0;accADC[axis]=0;}
    accADC[TILT] = acc_1G;
    return 1;
  } 

  // Wii Motion Plus Data
  if ( (rawADC[5]&0x03) == 0x02 ) {
    // Assemble 14bit data 
    gyroADC[PAN] = - ( ((rawADC[4]>>2)<<8) | rawADC[1] );
    gyroADC[TILT]  =  - ( ((rawADC[3]>>2)<<8) | rawADC[0] );
    GYRO_Common();
    // Check if slow bit is set and normalize to fast mode range
    gyroADC[PAN] = (rawADC[4]&0x02)>>1  ? gyroADC[PAN]/5 : gyroADC[PAN]; //we detect here the slow of fast mode WMP gyros values (see wiibrew for more details)
    gyroADC[TILT]   = (rawADC[3]&0x02)>>1  ? gyroADC[TILT]/5   : gyroADC[TILT];   // this step must be done after zero compensation
    return 1;
  } else if ( (rawADC[5]&0x03) == 0x00 ) { // Nunchuk Data
    ACC_ORIENTATION(  ( (rawADC[3]<<2)      | ((rawADC[5]>>4)&0x02) ) ,
                    - ( (rawADC[2]<<2)      | ((rawADC[5]>>3)&0x02) ) ,
                      ( ((rawADC[4]>>1)<<3) | ((rawADC[5]>>5)&0x06) ) );
    ACC_Common();
    return 0;
  } else
    return 2;
}
#endif

void initSensors() {
  delay(200);
  POWERPIN_ON;
  delay(100);
  i2c_init();
  delay(100);
  #if (GYRO)
	  Gyro_init();
  #else
	  WMP_init();
  #endif
  #if (ACC)
	  ACC_init();
	  acc_25deg = acc_1G * 0.423;
  #endif
}
