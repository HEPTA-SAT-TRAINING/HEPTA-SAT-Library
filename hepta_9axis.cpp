/**
 * @file hepta_9axis.cpp
 * @author Masaki Naito
 * @brief 
 * @version 0.1
 * @date 2024-08-22
 * 
 * @copyright UNISEC all rights reserved.
 * 
 */

#include"hepta_9axis.h"

#include <Wire.h>

#define ACC 0x19<<1 //addr_accel
#define MAG 0x13<<1 //addr_compus
#define GYRO 0x69<<1 //addr_gyro

Hepta9Axis::Hepta9Axis()
{
  Wire.begin();
  Wire.beginTransmission(ACC);

  cmd[0]=0x14; //softreset
  cmd[1]=0xB6; //triggers a reset 
  Wire.write(cmd, 2); 

  cmd[0]=0x0F; //acceleration measurement range 
  cmd[1]=0x05; //+-4g
  Wire.write(cmd, 2);

  cmd[0]=0x11; //Selection of the main power modes and low power sleep period
  cmd[1]=0x00; //NORMAL mode, Sleep duration = 0.5ms
  Wire.write(cmd, 2);

  Wire.endTransmission();
}

void Hepta9Axis::sen_acc(float *ax,float *ay,float *az)
{
  Wire.beginTransmission(ACC);
  send[0]=(char)(2);
  Wire.write(send, 1);
  Wire.endTransmission();

  Wire.requestFrom(ACC, 6);
  for(int i = 0; i < 6; i++) {
    data[i] = Wire.read();
  }

  for(int i = 0; i < 3; i++){
      accel[i]=(int16_t)(((int16_t)data[i*2+1]<<8) | data[i*2]) >> 4;
      if(accel[i]>2047)accel[i]-=4096;
      accel[i]= accel[i]/512*9.8;
  }

  *ax = accel[0];
  *ay = accel[1];
  *az = accel[2];
}

void Hepta9Axis::sen_gyro(float *gx,float *gy,float *gz)
{
  Wire.beginTransmission(GYRO);
  cmd[0]=0x0F; 
  cmd[1]=0x04;
  Wire.write(cmd, 2);

  cmd[0]=0x10; 
  cmd[1]=0x07;
  Wire.write(cmd, 2);

  cmd[0]=0x11; 
  cmd[1]=0x00;
  Wire.write(cmd, 2); 
  
  for(int i = 0; i < 6; i++) {
    send[0]=(char)(2+i);
    Wire.write(send, 1);
    data[i] = Wire.read();  //　あってる？
  }

  for(int i = 0; i < 3; i++) {
    gyroscope[i]=(int16_t)(((int16_t)data[i*2+1]<<8) | data[i*2]) >> 4;
    if(gyroscope[i]>32767)gyroscope[i]-=65536;
    gyroscope[i]=gyroscope[i]*125/2048;
  }

  //printf("gx = %2.4f, gy = %2.4f, gz = %2.4f\r\n",gyroscope[0],gyroscope[1],gyroscope[2]);
  *gx = gyroscope[0];
  *gy = gyroscope[1];
  *gz = gyroscope[2];

  Wire.endTransmission();
}

void Hepta9Axis::sen_mag(float *mx,float *my,float *mz)
{
  Wire.beginTransmission(MAG);

  cmd[0]=0x4B; 
  cmd[1]=0x01;
  Wire.write(cmd, 2);
  cmd[0]=0x4C; 
  cmd[1]=0x00;
  Wire.write(cmd, 2);
  cmd[0]=0x4E; 
  cmd[1]=0x84;
  Wire.write(cmd, 2);
  cmd[0]=0x51; 
  cmd[1]=0x04;
  Wire.write(cmd, 2);
  cmd[0]=0x52; 
  cmd[1]=0x16;
  Wire.write(cmd, 2);
  cmd[0]=0x00;
  Wire.write(cmd, 1);
  Wire.read(); //　あってる？読み飛ばすだけなのか？

  for(int i=0;i<8;i++){
    send[0]=(char)(0x42+i);
    Wire.write(send, 1);
    data[i] = Wire.read();
  }

  for(int i=0;i<3;i++){
      if(i!=2)magnet[i]=(int16_t)(((int16_t)data[i*2+1]<<8) | data[i*2]) >> 3;
      else magnet[i]=(int16_t)(((int16_t)data[i*2+1]<<8) | data[i*2]) >> 1;
      if(i==2 && magnet[i]>16383)magnet[i]-=32768;
      else if(i!=2 && magnet[i]>4095)magnet[i]-=8092;
  }

  //printf("mx = %2.4f, my = %2.4f, mz = %2.4f\r\n\n",magnet[0],magnet[1],magnet[2]);
  *mx = magnet[0];
  *my = magnet[1];
  *mz = magnet[2];
}
