#ifndef CONFIG_H_
#define CONFIG_H_

#define  VERSION  1  // Byte!

// Hardware
#define SERIAL_COM_SPEED 115200
#define I2C_SPEED 100000L     //100kHz normal mode, this value must be used for a genuine WMP
//#define I2C_SPEED 400000L   //400kHz fast mode, it works only with some WMP clones

//enable internal I2C pull ups
#define INTERNAL_I2C_PULLUPS

#define BMA020

// define to use stabilisation
#define STABI
//#define ROLLSTABI
#define NORC

#ifdef ROLLSTABI
	#define RCROLL	rcCommand[ROLL]
#else
	#define RCROLL	1500
#endif

// define to enable AuRiCo
//#define AURICO

// RC channels
#define  PAN          0
#define  TILT         1
#define  SHOOT        2
#define  ZOOM         3
#define  EXPOSURE     4
//#define  ROLL		  5

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
#define TILT_MIN    1020    //servo travel min, don't set it below 1020
#define TILT_MAX    2000    //servo travel max, max value=2000
#define TILT_MIDDLE 1500    //servo neutral value
#define TILT_PROP   10      //servo proportional (tied to angle) ; can be negative to invert movement

#define ROLL_MIN     1020
#define ROLL_MAX     2000
#define ROLL_MIDDLE  1500
#define ROLL_PROP    10

#define PAN_MIN     1020
#define PAN_MAX     2000
#define PAN_MIDDLE  1500
#define PAN_PROP    2

// some radios have not a neutral point centered on 1500. can be changed here
#define MIDRC 1500

// Aurico
#define AURICO_MODE_TILT 1
#define AURICO_MODE_SHOOT 2
#define AURICO_MODE_ROTATE 3
#define AURICO_MODE_RESET 4
#define AURICO_MODE_PAUSE 5
#define AURICO_PROGRAMM {AURICO_MODE_RESET, AURICO_MODE_SHOOT, AURICO_MODE_TILT, AURICO_MODE_SHOOT, AURICO_MODE_TILT, AURICO_MODE_SHOOT, AURICO_MODE_TILT, AURICO_MODE_SHOOT, AURICO_MODE_ROTATE};

#endif /* CONFIG_H_ */
