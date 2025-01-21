/**
 * @file icm20948.cpp
 * @author Masaki Naito
 * @brief Arduino Library for ICM-20948
 * @version 0.1
 * @date 2025-01-16
 * 
 * @copyright UNISEC all rights reserved.
 */

/**
 * @note 2025/01/20
 * このコードを実行した後にサンプルコードを実行すると，失敗してしまう
 * ケーブルを抜き差ししたらうまくいくので，変なアドレスにアクセスしてしまっている？
 * 
 */

#include "icm20948.h"

#include <Wire.h>

void Icm20948::begin(void) {
  uint8_t ack;

  Wire.begin();

  _reset();
  delay(10);
  _wakeup();

  ack = _reg_read(ICM20948_WHO_AM_I);
  Serial.print("ack: ");
  Serial.println(ack, HEX);

  _init_mag();

  set_accel_scale(ACCEL_FS_2G);
  set_gyro_scale(GYRO_FS_500dps);
}

void Icm20948::get_accel(float *ax, float *ay, float *az) {
  uint8_t buf[6] = {0};

  _select_bank(BANK0);
  _reg_read(ICM20948_ACCEL_XOUT_H, buf, 6);

  *ax = (int16_t)(buf[0] << 8 | buf[1]) / _accel_sensitivity;
  *ay = (int16_t)(buf[2] << 8 | buf[3]) / _accel_sensitivity;
  *az = (int16_t)(buf[4] << 8 | buf[5]) / _accel_sensitivity;
}

void Icm20948::get_gyro(float *gx, float *gy, float *gz) {
  uint8_t buf[6] = {0};

  _select_bank(BANK0);
  _reg_read(ICM20948_GYRO_XOUT_H, buf, 6);

  *gx = (int16_t)(buf[0] << 8 | buf[1]) / _gyro_sensitivity;
  *gy = (int16_t)(buf[2] << 8 | buf[3]) / _gyro_sensitivity;
  *gz = (int16_t)(buf[4] << 8 | buf[5]) / _gyro_sensitivity;
}

void Icm20948::get_mag(float *mx, float *my, float *mz) {
  uint8_t buf[6] = {0};

  Wire.beginTransmission(AK09916_I2C_ADDR);
  Wire.write(AK09916_WHO_AM_I);
  Wire.endTransmission();

  Wire.requestFrom(AK09916_I2C_ADDR, 1);
  if(Wire.available()) {
    Serial.print("WAI: ");
    Serial.println(Wire.read(), HEX);
  }

  Wire.beginTransmission(AK09916_I2C_ADDR);
  Wire.write(AK09916_XAXIS_HIGH);
  Wire.endTransmission();

  Wire.requestFrom(AK09916_I2C_ADDR, 6);
  if(Wire.available()) {
    for(uint8_t i = 0; i < 6; i++) {
      buf[i] = Wire.read();
    }
  }

  Wire.beginTransmission(AK09916_I2C_ADDR);
  Wire.write(AK09916_CNTL_2);
  // Wire.write((uint8_t)count%3);
  Wire.write(1);
  Wire.endTransmission();

  Wire.beginTransmission(AK09916_I2C_ADDR);
  Wire.write(AK09916_CNTL_2);
  Wire.endTransmission();

  Wire.requestFrom(AK09916_I2C_ADDR, 1);
  if(Wire.available()) {
    Serial.print("CNTL2: ");
    Serial.println(Wire.read(), HEX);
  }
  // _select_bank(BANK0);
  // _reg_read(ICM20948_EXT_SLV_SENS_DATA_00, buf, 6);

  // uint16_t wai = _who_am_i_mag();
  // Serial.print("mag who am i: ");
  // Serial.println(wai);

  int16_t mx_int = (int16_t)(buf[1] << 8 | buf[0]);
  int16_t my_int = (int16_t)(buf[3] << 8 | buf[2]);
  int16_t mz_int = (int16_t)(buf[5] << 8 | buf[4]);

  *mx = mx_int * AK09916_MAG_LSB;
  *my = my_int * AK09916_MAG_LSB;
  *mz = mz_int * AK09916_MAG_LSB;
}

void Icm20948::print_accel(void) {
  float ax, ay, az;
  get_accel(&ax, &ay, &az);

  Serial.print("ax: ");   Serial.print(ax);
  Serial.print(", ay: "); Serial.print(ay);
  Serial.print(", az: "); Serial.println(az);
}

void Icm20948::print_gyro(void) {
  float gx, gy, gz;
  get_gyro(&gx, &gy, &gz);

  Serial.print("gx: ");   Serial.print(gx);
  Serial.print(", gy: "); Serial.print(gy);
  Serial.print(", gz: "); Serial.println(gz);
}

void Icm20948::print_mag(void) {
  float mx, my, mz;
  get_mag(&mx, &my, &mz);

  Serial.print("mx: ");   Serial.print(mx);
  Serial.print(", my: "); Serial.print(my);
  Serial.print(", mz: "); Serial.println(mz);
}

void Icm20948::set_accel_scale(ACCEL_FS_SEL scale) {
  _accel_scale = scale;
  _accel_sensitivity = ACCEL_SCALE_FACTOR[scale];

  _select_bank(BANK2);
  _reg_write(ICM20948_ACCEL_CONFIG, ACCEL_DLPF << 3 | scale << 1 | 1);
  _reg_write(ICM20948_ACCEL_SMPLRT_DIV_1, 10);
  _reg_write(ICM20948_ODR_ALIGN_EN, 1);
}

void Icm20948::set_gyro_scale(GYRO_FS_SEL scale) {
  _gyro_scale = scale;
  _gyro_sensitivity = GYRO_SCALE_FACTOR[scale];

  _select_bank(BANK2);
  _reg_write(ICM20948_GYRO_CONFIG_1, scale << 1 | 1);
}

/* ----------------------
  private functions
------------------------- */
void Icm20948::_reset(void) {
  _select_bank(BANK0);
  _reg_write(ICM20948_PWR_MGMT_1, 0x81);
  delay(10);
}

void Icm20948::_wakeup(void) {
  // SLEEP disable
  _select_bank(BANK0);
  _reg_write(ICM20948_PWR_MGMT_1, 0x01);
}

void Icm20948::_select_bank(ICM20948_BANK bank) {
  if(bank > 4) {
    return;
  }

  _reg_write(REG_BANK_SEL, (uint8_t)bank << 4);
}

void Icm20948::_reg_write(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(ICM20948_I2C_ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

uint8_t Icm20948::_reg_read(uint8_t reg) {
  uint8_t ret;

  Wire.beginTransmission(ICM20948_I2C_ADDR);
  Wire.write(reg);
  Wire.endTransmission();

  Wire.requestFrom(ICM20948_I2C_ADDR, 1);
  if(Wire.available()) {
    ret = Wire.read();
  }

  return ret;
}

void Icm20948::_reg_read(uint8_t reg, uint8_t val[], uint8_t len) {
  Wire.beginTransmission(ICM20948_I2C_ADDR);
  Wire.write(reg);
  Wire.endTransmission();

  Wire.requestFrom(ICM20948_I2C_ADDR, len);
  if(Wire.available()) {
    for(uint8_t i = 0; i < len; i++) {
      val[i] = Wire.read();
    }
  }
}

void Icm20948::_init_mag(void) {
  // bool is_success = false;
  // _enable_i2c_master();

  // _mag_reset();
  // _reset();
  // _wakeup();

  // _select_bank(BANK2);
  // _reg_write(ICM20948_ODR_ALIGN_EN, 1);

  // for(uint8_t i = 0; i < 10; i++) {
  //   delay(10);
  //   _enable_i2c_master();
  //   delay(10);

  //   uint16_t wai = _who_am_i_mag();
  //   Serial.print("mag who am i: ");
  //   Serial.println(wai);

  //   if(wai == AK09916_DEVICE_ID) {
  //     is_success = true;
  //     break;
  //   } else {
  //     _i2c_master_reset();
  //   }
  // }
  // if(is_success) {
  //   _set_mag_mode(AK09916_CONT_MODE_20HZ);
  // }
  _select_bank(BANK0);
  _reg_write(ICM20948_USER_CTRL, 0<<5);
  _reg_write(0x0F, 1<<1);

  Wire.beginTransmission(AK09916_I2C_ADDR);
  Wire.write(AK09916_WHO_AM_I);
  Wire.endTransmission();

  Wire.requestFrom(AK09916_I2C_ADDR, 1);
  if(Wire.available()) {
    Serial.print("WAI: ");
    Serial.println(Wire.read(), HEX);
  }

  Wire.beginTransmission(AK09916_I2C_ADDR);
  Wire.write(AK09916_CNTL_3);
  Wire.write(1);
  Wire.endTransmission();
  delay(10);

  Wire.beginTransmission(AK09916_I2C_ADDR);
  Wire.write(AK09916_CNTL_2);
  Wire.write(4);
  Wire.endTransmission();

  _select_bank(BANK2);
  _reg_write(ICM20948_ODR_ALIGN_EN, 1);
}

uint8_t Icm20948::_who_am_i_mag(void) {
  return _ak09916_reg_read(AK09916_WHO_AM_I);
}

void Icm20948::_enable_i2c_master(void) {
  // enable I2C master
  _select_bank(BANK0);
  _reg_write(ICM20948_USER_CTRL, 1<<5);

  // set I2C clock(345.6kHz)
  _select_bank(BANK3);
  _reg_write(ICM20948_I2C_MST_CTRL, 7);
  delay(10);
}

void Icm20948::_i2c_master_reset(void) {
  _select_bank(BANK0);
  uint8_t val = _reg_read(ICM20948_USER_CTRL);
  val |= ICM20948_I2C_MST_RST;
  _reg_write(val, ICM20948_USER_CTRL);
}

void Icm20948::_mag_reset(void) {
  _ak09916_reg_write(AK09916_CNTL_3, 0x01);
  delay(100);
}

void Icm20948::_set_mag_mode(AK09916_OP_MODE mode) {
  _ak09916_reg_write(AK09916_CNTL_2, mode);
  delay(10);
  if(mode!=AK09916_PWR_DOWN){
    _enable_mag_data_read(AK09916_XAXIS_HIGH, 0x08);
  }
}

void Icm20948::_enable_mag_data_read(uint8_t reg, uint8_t len) {
  _select_bank(BANK3);
  _reg_write(ICM20948_I2C_SLV0_ADDR, AK09916_I2C_ADDR | AK09916_READ);
  _reg_write(ICM20948_I2C_SLV0_REG, reg);
  _reg_write(ICM20948_I2C_SLV0_CTRL, ICM20948_I2C_SLVX_EN | len);
  delay(10);
}

void Icm20948::_ak09916_reg_write(uint8_t reg, uint8_t val) {
  _select_bank(BANK3);
  _reg_write(ICM20948_I2C_SLV4_ADDR, AK09916_I2C_ADDR);
  _reg_write(ICM20948_I2C_SLV4_DO, val);
  _reg_write(ICM20948_I2C_SLV4_REG, reg);
  _reg_write(ICM20948_I2C_SLV4_CTRL, ICM20948_I2C_SLVX_EN);
  while(_reg_read(ICM20948_I2C_SLV4_CTRL) & ICM20948_I2C_SLVX_EN);
}

uint8_t Icm20948::_ak09916_reg_read(uint8_t reg) {
  _select_bank(BANK3);
  _reg_write(ICM20948_I2C_SLV4_ADDR, AK09916_I2C_ADDR | AK09916_READ); // read AK09916
  _reg_write(ICM20948_I2C_SLV4_REG, reg); // define AK09916 register to be read
  _reg_write(ICM20948_I2C_SLV4_CTRL, ICM20948_I2C_SLVX_EN);
  while(_reg_read(ICM20948_I2C_SLV4_CTRL) & ICM20948_I2C_SLVX_EN){;}
  return _reg_read(ICM20948_I2C_SLV4_DI);
}
