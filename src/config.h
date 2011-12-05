#ifndef CONFIG_H_
#define CONFIG_H_

#define  VERSION  "0.1"

// Hardware
#define SERIAL_COM_SPEED 9600
#define I2C_SPEED 100000L     //100kHz normal mode, this value must be used for a genuine WMP
//#define I2C_SPEED 400000L   //400kHz fast mode, it works only with some WMP clones

//enable internal I2C pull ups
#define INTERNAL_I2C_PULLUPS

//if you use independent sensors
/* I2C gyroscope */
//#define ITG3200
//#define L3G4200D

/* I2C accelerometer */
//#define ADXL345
#define BMA020
//#define BMA180
//#define NUNCHACK  // if you want to use the nunckuk as a standalone I2C ACC without WMP
//#define LIS3LV02

/* interleaving delay in micro seconds between 2 readings WMP/NK in a WMP+NK config
   if the ACC calibration time is very long (20 or 30s), try to increase this delay up to 4000
   it is relevent only for a conf with NK */
#define INTERLEAVING_DELAY 3000

/* when there is an error on I2C bus, we neutralize the values during a short time. expressed in microseconds
   it is relevent only for a conf with at least a WMP */
#define NEUTRALIZE_DELAY 100000



// define to use stabilisation
//#define STABI

// define to enable AuRiCo
//#define AURICO

// RC channels
#define  PAN          0
#define  TILT         1
#define  SHOOT        2
#define  ZOOM         3
#define  EXPOSURE     4

// pins for interrupts
#define  PANPIN       2
#define  TILTPIN      4
#define  SHOOTPIN     5
#define  ZOOMPIN      6
#define  EXPOSUREPIN  7

// output pins
#define  PANSERVO     A0
#define  TILTSERVO    A1
#define  CAMPIN       10
#define  LEDPIN		  13

// pause
#define WAIT_SHOOT    50
#define WAIT_ZOOM	  25

#endif /* CONFIG_H_ */
