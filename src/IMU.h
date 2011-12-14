#include <WProgram.h>
#include "def.h"
#include "sensors.h"
#include "RX.h"
#include "output.h"

// **************************************************
// Simplified IMU based on "Complementary Filter"
// Inspired by http://starlino.com/imu_guide.html
//
// adapted by ziss_dm : http://wbb.multiwii.com/viewtopic.php?f=8&t=198
//
// The following ideas was used in this project:
// 1) Rotation matrix: http://en.wikipedia.org/wiki/Rotation_matrix
// 2) Small-angle approximation: http://en.wikipedia.org/wiki/Small-angle_approximation
// 3) C. Hastings approximation for atan2()
// 4) Optimization tricks: http://www.hackersdelight.org/
//
// Currently Magnetometer uses separate CF which is used only
// for heading approximation.
//
// Modified: 19/04/2011  by ziss_dm
// Version: V1.1
//
// code size deduction and tmp vector intermediate step for vector rotation computation: October 2011 by Alex
// **************************************************

//******  advanced users settings *******************
/* Set the Low Pass Filter factor for ACC */
/* Increasing this value would reduce ACC noise (visible in GUI), but would increase ACC lag time*/
/* Comment this if  you do not want filter at all.*/
/* Default WMC value: 8*/
#define ACC_LPF_FACTOR 8

/* Set the Gyro Weight for Gyro/Acc complementary filter */
/* Increasing this value would reduce and delay Acc influence on the output of the filter*/
/* Default WMC value: 300*/
#define GYR_CMPF_FACTOR 50.0f   //310.0f

//****** end of advanced users settings *************

#define INV_GYR_CMPF_FACTOR   (1.0f / (GYR_CMPF_FACTOR  + 1.0f))
#define GYRO_SCALE (1.0f/200e6f)
// empirical, depends on WMP on IDG datasheet, tied of deg/ms sensibility
// !!!!should be adjusted to the rad/sec

// Small angle approximation
#define ssin(val) (val)
#define scos(val) 1.0f

typedef struct fp_vector {
  float X;
  float Y;
  float Z;
} t_fp_vector_def;

typedef union {
  float   A[3];
  t_fp_vector_def V;
} t_fp_vector;


static uint8_t  calibratedACC = 0;
static uint32_t currentTime = 0;


int16_t _atan2(float y, float x){
  #define fp_is_neg(val) ((((byte*)&val)[3] & 0x80) != 0)
  float z = y / x;
  int16_t zi = abs(int16_t(z * 100)); 
  int8_t y_neg = fp_is_neg(y);
  if ( zi < 100 ){
    if (zi > 10) 
     z = z / (1.0f + 0.28f * z * z);
   if (fp_is_neg(x)) {
     if (y_neg) z -= PI;
     else z += PI;
   }
  } else {
   z = (PI / 2.0f) - z / (z * z + 0.28f);
   if (y_neg) z -= PI;
  }
  z *= (180.0f / PI * 10); 
  return z;
}


// Rotate Estimated vector(s) with small angle approximation, according to the gyro data
void rotateV(struct fp_vector *v,float* delta) {
  fp_vector v_tmp = *v;
  v->Z -= delta[ROLLAXIS]  * v_tmp.X + delta[TILTAXIS] * v_tmp.Y;
  v->X += delta[ROLLAXIS]  * v_tmp.Z - delta[PANAXIS]   * v_tmp.Y;
  v->Y += delta[TILTAXIS] * v_tmp.Z + delta[PANAXIS]   * v_tmp.X;
}


void getEstimatedAttitude(){
  uint8_t axis;
  int16_t accMag = 0;
  static t_fp_vector EstG;
  static int16_t accTemp[3];  //projection of smoothed and normalized magnetic vector on x/y/z axis, as measured by magnetometer
  static uint16_t previousT;
  uint16_t currentT = micros();
  float scale, deltaGyroAngle[3];

  scale = (currentT - previousT) * GYRO_SCALE;
  previousT = currentT;

  // Initialization
  for (axis = 0; axis < 3; axis++) {
    deltaGyroAngle[axis] = gyroADC[axis]  * scale;
    accTemp[axis] = (accTemp[axis] - (accTemp[axis] >>4)) + accADC[axis];
    accSmooth[axis] = accTemp[axis]>>4;
    accMag += (accSmooth[axis] * 10 / (int16_t)acc_1G) * (accSmooth[axis] * 10 / (int16_t)acc_1G);
  }

  rotateV(&EstG.V,deltaGyroAngle);

  if ( abs(accSmooth[ROLLAXIS])<acc_25deg && abs(accSmooth[TILTAXIS])<acc_25deg && accSmooth[PANAXIS]>0)
    smallAngle25 = 1;
  else
    smallAngle25 = 0;

  // Apply complimentary filter (Gyro drift correction)
  // If accel magnitude >1.4G or <0.6G and ACC vector outside of the limit range => we neutralize the effect of accelerometers in the angle estimation.
  // To do that, we just skip filter, as EstV already rotated by Gyro
  if ( ( 36 < accMag && accMag < 196 ) || smallAngle25 )
    for (axis = 0; axis < 3; axis++) {
      int16_t acc = accSmooth[axis];
      #if not defined(TRUSTED_ACCZ)
        if (smallAngle25 && axis == PANAXIS)
          //We consider ACCZ = acc_1G when the acc on other axis is small.
          //It's a tweak to deal with some configs where ACC_Z tends to a value < acc_1G when high throttle is applied.
          //This tweak applies only when the multi is not in inverted position
          acc = acc_1G;      
      #endif
      EstG.A[axis] = (EstG.A[axis] * GYR_CMPF_FACTOR + acc) * INV_GYR_CMPF_FACTOR;
    }
  
  // Attitude of the estimated vector
  angle[ROLLAXIS]  =  _atan2(EstG.V.X , EstG.V.Z) ;
  angle[TILTAXIS] =  _atan2(EstG.V.Y , EstG.V.Z) ;
}


float InvSqrt (float x){ 
  union{  
    int32_t i;  
    float   f; 
  } conv; 
  conv.f = x; 
  conv.i = 0x5f3759df - (conv.i >> 1); 
  return 0.5f * conv.f * (3.0f - x * conv.f * conv.f);
} 


int32_t isq(int32_t x){return x * x;}


void annexCode() { //this code is excetuted at each loop and won't interfere with control loop if it lasts less than 650 microseconds
  static uint32_t calibratedAccTime;
  uint8_t axis;

  // mapping rcData to a value from -500 to 500 with the center = 0
  for(axis=0;axis<2;axis++) {
    rcCommand[axis] = min(abs(rcData[axis]-MIDRC),500);
    if (rcData[axis]<MIDRC) rcCommand[axis] = -rcCommand[axis];
  }

  if ( calibratingA>0 ) {  // Calibration phasis
    if (calibratingA % 5 == 0) { LEDPIN_TOGGLE };
  } else {
	LEDPIN_ON;
  }

  if ( currentTime > calibratedAccTime ) {
    if (smallAngle25 == 0) {
      calibratedACC = 0; //the multi uses ACC and is not calibrated or is too much inclinated
      LEDPIN_TOGGLE;
      calibratedAccTime = currentTime + 500000;
    } else
      calibratedACC = 1;
  }
}


#define UPDATE_INTERVAL 25000    // 40hz update rate (20hz LPF on acc)
#define INIT_DELAY      4000000  // 4 sec initialization delay
#define Kp1 0.55f                // PI observer velocity gain 
#define Kp2 1.0f                 // PI observer position gain
#define Ki  0.001f               // PI observer integral gain (bias cancellation)
#define dt  (UPDATE_INTERVAL / 1000000.0f)


void computeIMU()
{
	uint8_t axis;
	static int16_t gyroADCprevious[3] = {0,0,0};
	int16_t gyroADCp[3];
	int16_t gyroADCinter[3];
	static uint32_t timeInterleave = 0;

	ACC_getADC();
	getEstimatedAttitude();
	WMP_getRawADC();

	for (axis = 0; axis < 3; axis++) gyroADCp[axis] =  gyroADC[axis];

	timeInterleave=micros();

	annexCode();
	while((micros()-timeInterleave)<650) ; //empirical, interleaving delay between 2 consecutive reads

	WMP_getRawADC();

	for (axis = 0; axis < 3; axis++)
	{
		gyroADCinter[axis] =  gyroADC[axis]+gyroADCp[axis];
		// empirical, we take a weighted value of the current and the previous values
		gyroData[axis] = (gyroADCinter[axis]+gyroADCprevious[axis]+1)/3;
		gyroADCprevious[axis] = gyroADCinter[axis]/2;
	}
}
