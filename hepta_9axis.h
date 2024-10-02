/**
 * @file hepta_9axis.h
 * @author Masaki Naito
 * @brief 
 * @version 0.1
 * @date 2024-08-22
 * 
 * @copyright UNISEC all rights reserved.
 * 
 */


#ifndef HEPTA_9AXIS_H
#define HEPTA_9AXIS_H

#include <Arduino.h>
#include <stdint.h>

class Hepta9Axis {
  public:
    Hepta9Axis();

    int addr_accel;
    int addr_gyro;
    int addr_compus;

    void sen_acc(float *ax,float *ay,float *az);
    void sen_gyro(float *gx,float *gy,float *gz);
    void sen_mag(float *mx,float *my,float *mz);

  private:
    char g1[8],g2[8];
    char a1[8],a2[8];
    char m1[8],m2[8];
    short int st2;
    short int xl,xh,yl,yh,zl,zh;
    short int gxl,gxh,gyl,gyh,gzl,gzh;
    short int mxl,mxh,myl,myh,mzl,mzh;
    
    float accel[3];
    float magnet[3];
    float gyroscope[3];
    char cmd[2];
    uint8_t data[8];
    char send[1], get[1];
    char temp;
};


#endif /* HEPTA_9AXIS_H */
