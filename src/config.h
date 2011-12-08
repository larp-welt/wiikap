#ifndef CONFIG_H_
#define CONFIG_H_

#define  VERSION  "0.1"

// Hardware
#define SERIAL_COM_SPEED 9600
#define I2C_SPEED 100000L     //100kHz normal mode, this value must be used for a genuine WMP
//#define I2C_SPEED 400000L   //400kHz fast mode, it works only with some WMP clones

//enable internal I2C pull ups
#define INTERNAL_I2C_PULLUPS

#define BMA020

// define to use stabilisation
#define STABI

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

// servo const
#define TILT_PITCH_MIN    1020    //servo travel min, don't set it below 1020
#define TILT_PITCH_MAX    2000    //servo travel max, max value=2000
#define TILT_PITCH_MIDDLE 1500    //servo neutral value
#define TILT_PITCH_PROP   10      //servo proportional (tied to angle) ; can be negative to invert movement
#define TILT_ROLL_MIN     1020
#define TILT_ROLL_MAX     2000
#define TILT_ROLL_MIDDLE  1500
#define TILT_ROLL_PROP    10

/* some radios have not a neutral point centered on 1500. can be changed here */
#define MIDRC 1500

#endif /* CONFIG_H_ */
