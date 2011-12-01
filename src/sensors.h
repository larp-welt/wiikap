/*
 * sensors.h
 *
 *  Created on: 01.12.2011
 *      Author: tiggr
 */

#ifndef SENSORS_H_
#define SENSORS_H_

#include <WProgram.h>
#include "config.h"
#include "def.h"

// ************************************************************************************************************
// board orientation and setup
// ************************************************************************************************************
//default board orientation
#if !defined(ACC_ORIENTATION)
  #define ACC_ORIENTATION(X, Y, Z)  {accADC[ROLL]  = X; accADC[TILT]  = Y; accADC[PAN]  = Z;}
#endif
#if !defined(GYRO_ORIENTATION)
  #define GYRO_ORIENTATION(X, Y, Z) {gyroADC[ROLL] = X; gyroADC[TILT] = Y; gyroADC[PAN] = Z;}
#endif
#if !defined(MAG_ORIENTATION)
  #define MAG_ORIENTATION(X, Y, Z)  {magADC[ROLL]  = X; magADC[TILT]  = Y; magADC[PAN]  = Z;}
#endif

/*** I2C address ***/
#if !defined(ADXL345_ADDRESS)
  #define ADXL345_ADDRESS 0x3A
  //#define ADXL345_ADDRESS 0xA6   //WARNING: Conflicts with a Wii Motion plus!
#endif

#if !defined(BMA180_ADDRESS)
  #define BMA180_ADDRESS 0x80
  //#define BMA180_ADDRESS 0x82
#endif

#if !defined(ITG3200_ADDRESS)
  #define ITG3200_ADDRESS 0XD0
  //#define ITG3200_ADDRESS 0XD2
#endif

#if !defined(MS561101BA_ADDRESS)
  #define MS561101BA_ADDRESS 0xEE //CBR=0 0xEE I2C address when pin CSB is connected to LOW (GND)
  //#define MS561101BA_ADDRESS 0xEF //CBR=1 0xEF I2C address when pin CSB is connected to HIGH (VCC)
#endif

//ITG3200 and ITG3205 Gyro LPF setting
#if defined(ITG3200_LPF_256HZ) || defined(ITG3200_LPF_188HZ) || defined(ITG3200_LPF_98HZ) || defined(ITG3200_LPF_42HZ) || defined(ITG3200_LPF_20HZ) || defined(ITG3200_LPF_10HZ)
  #if defined(ITG3200_LPF_256HZ)
    #define ITG3200_SMPLRT_DIV 0  //8000Hz
    #define ITG3200_DLPF_CFG   0
  #endif
  #if defined(ITG3200_LPF_188HZ)
    #define ITG3200_SMPLRT_DIV 0  //1000Hz
    #define ITG3200_DLPF_CFG   1
  #endif
  #if defined(ITG3200_LPF_98HZ)
    #define ITG3200_SMPLRT_DIV 0
    #define ITG3200_DLPF_CFG   2
  #endif
  #if defined(ITG3200_LPF_42HZ)
    #define ITG3200_SMPLRT_DIV 0
    #define ITG3200_DLPF_CFG   3
  #endif
  #if defined(ITG3200_LPF_20HZ)
    #define ITG3200_SMPLRT_DIV 0
    #define ITG3200_DLPF_CFG   4
  #endif
  #if defined(ITG3200_LPF_10HZ)
    #define ITG3200_SMPLRT_DIV 0
    #define ITG3200_DLPF_CFG   5
  #endif
#else
    //Default settings LPF 256Hz/8000Hz sample
    #define ITG3200_SMPLRT_DIV 0  //8000Hz
    #define ITG3200_DLPF_CFG   0
#endif

extern uint8_t rawADC[6];
extern uint8_t nunchuk;
extern uint32_t neutralizeTime;
extern uint16_t acc_1G;             // this is the 1G measured acceleration
extern int16_t  acc_25deg;
extern int16_t  gyroADC[3],accADC[3];
extern int16_t  accTrim[2];
extern int16_t gyroZero[3];
extern int16_t accZero[3];
extern uint16_t calibratingA;  // the calibration is done is the main loop. Calibrating decreases at each cycle down to 0, then we enter in a normal mode.
extern uint16_t calibratingG;
extern int16_t  i2c_errors_count;

// ************************************************************************************************************
// I2C general functions
// ************************************************************************************************************

void i2c_init(void);
void i2c_rep_start(uint8_t address);
void i2c_stop(void);
void i2c_write(uint8_t data );
uint8_t i2c_readAck();
uint8_t i2c_readNak(void);
void waitTransmissionI2C();
void i2c_getSixRawADC(uint8_t add, uint8_t reg);
void i2c_writeReg(uint8_t add, uint8_t reg, uint8_t val);
uint8_t i2c_readReg(uint8_t add, uint8_t reg);

// ****************
// GYRO common part
// ****************
void GYRO_Common();

// ****************
// ACC common part
// ****************
void ACC_Common();

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
void ACC_init ();
void ACC_getADC ();
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
void ACC_init ();
void ACC_getADC ();
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
void ACC_init();
void ACC_getADC();
#endif

// ************************************************************************************************************
// standalone I2C Nunchuk
// ************************************************************************************************************
#if defined(NUNCHACK)
void ACC_init();
void ACC_getADC();
#endif

// ************************************************************************
// LIS3LV02 I2C Accelerometer
//contribution from adver (http://multiwii.com/forum/viewtopic.php?f=8&t=451)
// ************************************************************************
#if defined(LIS3LV02)
#define LIS3A  0x3A // I2C adress: 0x3A (8bit)

void i2c_ACC_init();
void i2c_ACC_getADC();
#endif

// ************************************************************************************************************
// I2C Accelerometer LSM303DLx
// contribution from wektorx (http://www.multiwii.com/forum/viewtopic.php?f=8&t=863)
// ************************************************************************************************************
#if defined(LSM303DLx_ACC)
void ACC_init ();
void ACC_getADC ();
#endif

// ************************************************************************************************************
// ADC ACC
// ************************************************************************************************************
#if defined(ADCACC)
void ACC_init();
void ACC_getADC();
#endif

// ************************************************************************************************************
// contribution from Ciskje
// I2C Gyroscope L3G4200D
// ************************************************************************************************************
#if defined(L3G4200D)
void Gyro_init();
void Gyro_getADC ();
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
void Gyro_init();
void Gyro_getADC ();
#endif

#if !GYRO
// ************************************************************************************************************
// I2C Wii Motion Plus + optional Nunchuk
// ************************************************************************************************************
// I2C adress 1: 0xA6 (8bit)    0x53 (7bit)
// I2C adress 2: 0xA4 (8bit)    0x52 (7bit)
// ************************************************************************************************************
void WMP_init();
uint8_t WMP_getRawADC();
#endif

void initSensors();

#endif /* SENSORS_H_ */
