#ifndef ITG3200_H_GUARD
#define ITG3200_H_GUARD

#include <Wire.h> // I2C library, gyroscope

#define GYRO_ADDR 0x69 // gyro address, binary = 11101001 when AD0 is connected to Vcc (see schematics of your breakout board)
#define SMPLRT_DIV 0x15
#define DLPF_FS 0x16
#define INT_CFG 0x17
#define PWR_MGM 0x3E

#define TO_READ 6 // 2 bytes for each axis x, y, z


//initializes the gyroscope
void initGyro()
{
  /*****************************************
  * ITG 3200
  * power management set to:
  * clock select = internal oscillator
  *     no reset, no sleep mode
  *   no standby mode
  * sample rate to = 3Hz
  * parameter to +/- 2000 degrees/sec
  * low pass filter = 5Hz
  * no interrupt
  ******************************************/

  writeTo(GYRO_ADDR, PWR_MGM, 0x00);
//  writeTo(GYRO_ADDR, SMPLRT_DIV, 0xFF); // EB, 50, 80, 7F, DE, 23, 20, FF
  writeTo(GYRO_ADDR, SMPLRT_DIV, (100-1) ); // EB, 50, 80, 7F, DE, 23, 20, FF
//  writeTo(GYRO_ADDR, DLPF_FS, 0x1E); // +/- 2000 dgrs/sec, 1KHz, 1E, 19
//  writeTo(GYRO_ADDR, DLPF_FS, B11110); // +/- 2000 dgrs/sec, 1KHz, 1E, 19
  writeTo(GYRO_ADDR, DLPF_FS, 0x19); // +/- 2000 dgrs/sec, 1KHz, 1E, 19
//  writeTo(GYRO_ADDR, INT_CFG, 0x00);
  writeTo(GYRO_ADDR, INT_CFG, 0x05);

}


//void getGyroscopeData(double *x, double *y, double *z)
bool getGyroscopeData(int *x, int *y, int *z)
{
  /**************************************
  Gyro ITG-3200 I2C
  registers:
  x axis MSB = 1D, x axis LSB = 1E
  y axis MSB = 1F, y axis LSB = 20
  z axis MSB = 21, z axis LSB = 22
  *************************************/

  // read the interrupt register
  // if not set dont read and bomb out early..  
  byte intreg;
  readFrom(GYRO_ADDR, 0x1A, 1, &intreg);
  if((intreg & 1) != 1){
    return false; 
  }

  int regAddress = 0x1D;
  byte buff[TO_READ];

  readFrom(GYRO_ADDR, regAddress, TO_READ, buff); //read the gyro data from the ITG3200

  *x = ((buff[0] << 8) | buff[1]);
  *y = ((buff[2] << 8) | buff[3]);
  *z = ((buff[4] << 8) | buff[5]);

  return true;
}

//convert rotations to degrees
double gyroRotationsToDeg(long *rotations){
  double deg = *rotations / 143.75; //ITG3200's hardware scale factor
  if(deg > 360){ //deal with wrap around..
    deg -= 360;
  }else
  if(deg < 0){
    deg += 360;
  }
  return deg;
}
// accumulates (or deccumulate) total rotations since power on
long accumulateRotations(int gyro, long *rotations, int *offset){
  if(gyro!=0){
    if(*offset==0){ //auto offset
      *offset = gyro;
    }
    gyro -= *offset;

    if((gyro > 10) || (gyro < -10)){ //remove tiny movements
      *rotations += gyro;
    }
  }
  return *rotations;
}

#endif
