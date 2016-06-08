//
// title:     MPU9250.h
// author:    Taylor, Brian R.
// email:     brian.taylor@bolderflight.com
// date:      2016-06-08 
// license: 
//

#ifndef MPU9250_h
#define MPU9250_h

#include "Arduino.h"

#define ACCEL_OUT			0x3B
#define GYRO_OUT			0x43

#define ACCEL_CONFIG		0x1C
#define ACCEL_FS_SEL_2G		0x00
#define	ACCEL_FS_SEL_4G		0x08
#define ACCEL_FS_SEL_8G 	0x10
#define ACCEL_FS_SEL_16G	0x18

#define ACCEL_CONFIG2		0x1D

#define ACCEL_DLPF_184		0x01
#define ACCEL_DLPF_92 		0x02
#define ACCEL_DLPF_41 		0x03
#define ACCEL_DLPF_20 		0x04
#define ACCEL_DLPF_10 		0x05
#define ACCEL_DLPF_5 		0x06

#define GYRO_CONFIG			0x1B
#define GYRO_FS_SEL_250DPS	0x00
#define GYRO_FS_SEL_500DPS	0x08
#define GYRO_FS_SEL_1000DPS	0x10
#define GYRO_FS_SEL_2000DPS	0x18

#define CONFIG 				0x1A

#define GYRO_DLPF_184		0x01
#define GYRO_DLPF_92		0x02
#define GYRO_DLPF_41 		0x03
#define GYRO_DLPF_20 		0x04
#define GYRO_DLPF_10 		0x05
#define GYRO_DLPF_5 		0x06

#define SMPDIV				0x19

#define INT_PIN_CFG         0x37
#define INT_ENABLE          0x38

#define PWR_MGMNT_1			0x6B
#define CLOCK_SEL_PLL		0x01

#define G					9.807
#define D2R					180.0/PI						

class MPU9250{
  public:
    MPU9250(int address);
    void begin(String accelRange, String gyroRange);
    void setFilt(String bandwidth, uint8_t frequency);
    void getAccel(double* ax, double* ay, double* az);
    void getAccelCounts(uint16_t* ax, uint16_t* ay, uint16_t* az);
    void getGyro(double* gx, double* gy, double* gz);
    void getGyroCounts(uint16_t* gx, uint16_t* gy, uint16_t* gz);
    //void getMag(double* hx, double* hy, double* hz);
    void getMotion6(double* ax, double* ay, double* az, double* gx, double* gy, double* gz);
    void getMotion6Counts(uint16_t* ax, uint16_t* ay, uint16_t* az, uint16_t* gx, uint16_t* gy, uint16_t* gz);
  private:
    int _address;
    double _accelScale;
    double _gyroScale;
    void writeRegister(uint8_t subAddress, uint8_t data);
    void readRegisters(uint8_t subAddress, int count, uint8_t* dest);
};

#endif