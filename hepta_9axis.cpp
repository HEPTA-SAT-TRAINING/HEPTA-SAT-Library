/**
 * @file hepta_9axis.cpp
 * @author Masaki Naito
 * @brief 
 * @version 0.1
 * @date 2024-12-09
 * 
 * @copyright UNISEC all rights reserved.
 * 
 */

#include "hepta_9axis.h"

#include <Wire.h>

void Hepta9Axis::begin(void) {
  Wire.begin();

  //
  // ACC setting
  //
  Wire.beginTransmission(I2C_ADDR_ACC);
  Wire.write(0x14);  //softreset
  Wire.write(0xB6);  //triggers a reset 
  Wire.endTransmission();

  Wire.beginTransmission(I2C_ADDR_ACC);
  Wire.write(0x0F);  //acceleration measurement range 
  Wire.write(0x05);  //+-4g
  Wire.endTransmission();

  Wire.beginTransmission(I2C_ADDR_ACC);
  Wire.write(0x11);  //Selection of the main power modes and low power sleep period
  Wire.write(0x00);  //NORMAL mode, Sleep duration = 0.5ms
  Wire.endTransmission();

  //
  // GYRO setting
  //
  Wire.beginTransmission(I2C_ADDR_GYRO);
  Wire.write(0x0F);  // gyro range
  Wire.write(0x04);  // +-125dps
  Wire.endTransmission();

  Wire.beginTransmission(I2C_ADDR_GYRO);
  Wire.write(0x10);  // rate date filter bandwidth
  Wire.write(0x07);  // Output data rate 100Hz
  Wire.endTransmission();

  Wire.beginTransmission(I2C_ADDR_GYRO);
  Wire.write(0x11);  // main power mode
  Wire.write(0x02);  // 2ms
  Wire.endTransmission();

  //
  // MAG setting
  //
  Wire.beginTransmission(I2C_ADDR_MAG);
  Wire.write(0x4B);  // power, softreset, spi
  Wire.write(0x01);  // softreset
  Wire.endTransmission();

  Wire.beginTransmission(I2C_ADDR_MAG);
  Wire.write(0x4C);  // operation mode
  Wire.write(0x00);  // Output data rate 10Hz
  Wire.endTransmission();

  Wire.beginTransmission(I2C_ADDR_MAG);
  Wire.write(0x4E);  // interrupt and axes enable
  Wire.write(0x84);  // xyz enable
  Wire.endTransmission();

  Wire.beginTransmission(I2C_ADDR_MAG);
  Wire.write(0x51);  // repetition number(xy)
  Wire.write(0x04);  // 9 times
  Wire.endTransmission();

  Wire.beginTransmission(I2C_ADDR_MAG);
  Wire.write(0x52);  // repetition number(z)
  Wire.write(0x16);  // 15 times
  Wire.endTransmission();
}

void Hepta9Axis::sen_acc(float *ax, float *ay, float *az) {
  float accel[3];

  for(uint8_t i = 0; i < 6; i++) {
    Wire.beginTransmission(I2C_ADDR_ACC);
    Wire.write(2 + i);
    Wire.endTransmission();

    Wire.requestFrom(I2C_ADDR_ACC, 1);
    if (Wire.available() == 1) {
      data[i] = Wire.read();
    }
  }

  for(uint8_t i = 0; i < 3; i++) {
    accel[i] = (data[i*2+1] << 4) | (data[i*2] >> 4);
    if(accel[i] > 2047) {
      accel[i] -= 4096;
    }
    // 1.95 mg/LSB when range is +-4G
    // datasheet page 28 (rev1.1)
    accel[i] *= 0.00195 * 9.81;
  }

  *ax = accel[0];
  *ay = accel[1];
  *az = accel[2];
}

void Hepta9Axis::sen_gyro(float *gx, float *gy, float *gz) {
  float gyro[3];

  for(uint8_t i = 0; i < 6; i++) {
    Wire.beginTransmission(I2C_ADDR_GYRO);
    Wire.write(2 + i);
    Wire.endTransmission();

    Wire.requestFrom(I2C_ADDR_GYRO, 1);
    if (Wire.available() == 1) {
      data[i] = Wire.read();
    }
  }

  for(uint8_t i = 0; i < 3; i++) {
    gyro[i] = data[i*2+1] << 8 | data[i*2];

    if(gyro[i] > 32767) {
      gyro[i] -= 65536;
    }
    // 3.8mdeg/s/LSB when full scale is +-125dps
    // datasheet page 99 (rev1.1)
    gyro[i] = gyro[i] * 0.0038;
  }

  *gx = gyro[0];
  *gy = gyro[1];
  *gz = gyro[2];
}

void Hepta9Axis::sen_mag(float *mx,float *my,float *mz) {
  for(uint8_t i = 0; i < 6; i++) {
    Wire.beginTransmission(I2C_ADDR_MAG);
    Wire.write(0x42 + i);
    Wire.endTransmission();

    Wire.requestFrom(I2C_ADDR_MAG, 1);
    if (Wire.available() == 1) {
      data[i] = Wire.read();
    }
  }

  *mx = data[1] << 5 | data[0] >> 3;
  if(*mx > 4095) *mx -= 8192;

  *my = data[3] << 5 | data[2] >> 3;
  if(*my > 4095) *my -= 8192;

  *mz = data[5] << 7 | data[4] >> 1;
  if(*mz > 16383) *mz -= 32768;
}

void Hepta9Axis::print_acc(void) {
  float ax, ay, az;

  sen_acc(&ax, &ay, &az);

  Serial.print("ax: ");
  Serial.print(ax);
  Serial.print(", ay: ");
  Serial.print(ay);
  Serial.print(", az: ");
  Serial.println(az);
}

void Hepta9Axis::print_gyro(void) {
  float gx, gy, gz;

  sen_gyro(&gx, &gy, &gz);

  Serial.print("gx: ");
  Serial.print(gx);
  Serial.print(", gy: ");
  Serial.print(gy);
  Serial.print(", gz: ");
  Serial.println(gz);
}

void Hepta9Axis::print_mag(void) {
  float mx, my, mz;

  sen_mag(&mx, &my, &mz);

  Serial.print("mx: ");
  Serial.print(mx);
  Serial.print(", my: ");
  Serial.print(my);
  Serial.print(", mz: ");
  Serial.println(mz);
}
