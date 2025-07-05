/**
 * @file gps_gp1818mk.cpp
 * @author Masaki Naito
 * @brief 
 * @version 0.1
 * @date 2025-02-26
 * 
 * @copyright UNISEC all rights reserved.
 * 
 */

#include "gps_gp1818mk.h"


Gps1818mk::Gps1818mk()
  : GpsSerial(_rx_pin, _tx_pin) {
}

void Gps1818mk::begin(void) {
  GpsSerial.begin(9600);
}

void Gps1818mk::read_raw(void) {
  char buf;
  uint8_t timeout = 0;
  while(1) {
    if(GpsSerial.available()) {
      buf = GpsSerial.read();
      Serial.print(buf);
      timeout++;
      if(timeout >= UINT8_MAX - 1) {
        Serial.println("Timeout");
        break;
      }
    }
  }
}

bool Gps1818mk::get_position(float* lat, float* lon, float* alt) {
  if (!wait_serial()) {
    return false;
  }

  char gpgga[6] = {'$', 'G', 'P', 'G', 'G', 'A'};
  if(!get_header(gpgga)) {
    Serial.println("Header error");
    return 0;
  }

  // とりあえずそんなにいかないだろうという仮定
  const uint16_t GPGGA_LEN_MAX = 256;
  char rawdata[256] = {0};
  uint8_t actual_len = 0;
  char buf = 0;

  for(uint16_t i = 0; i < GPGGA_LEN_MAX; i++) {
    buf = read_byte();
    // 最初のカンマは読み飛ばす
    if(i > 0) {
      rawdata[i - 1] = buf;
      // ターミネータを見つけるまで
      //!@todo i-1なのわかりにくいからやめたい
      if(rawdata[i-2] == '\r' && rawdata[i-1] == '\n') {
        actual_len = i;
        break;
      }
    }
    if(i >= GPGGA_LEN_MAX - 1) {
      Serial.println("Packet error");
      return 0;
    }
  }

  // for(uint16_t i = 0; i < actual_len; i++) {
  //   Serial.print(rawdata[i]);
  // }
  // Serial.println();

  float utc_time,lat_deg,lon_deg,msl_alt;
  int fix;
  char lat_direc,lon_direc;
  sscanf(rawdata,"%f,%f,%c,%f,%c,%d,%d,%f,%f,%c,%f,%c",
         &utc_time,&lat_deg,&lat_direc,&lon_deg,&lon_direc,
         &fix,NULL,NULL,&msl_alt,NULL,NULL,NULL);

  if(fix == 0) {
    return 0;
  }

  int lat_d = (int)(lat_deg/100.0f);
  float lat_f = (lat_deg - (float)lat_d*100.0f);
  *lat = (float)lat_d + lat_f/60.0f;

  int lon_d = (int)(lon_deg/100.0f);
  float lon_f = (lon_deg - (float)lon_d*100.0f);
  *lon = (float)lon_d + lon_f/60.0f;

  *alt = msl_alt;

  return 1;
}

char Gps1818mk::read_byte(void) {
  for(uint32_t i = 0; i < UINT32_MAX; i++) {
    if(GpsSerial.available()) {
      break;
    }
    if(i >= UINT32_MAX - 1) {
      Serial.println("Serial unvailable");
      return 0;
    }
  }
  return GpsSerial.read();
}

bool Gps1818mk::wait_serial(void) {
  for(uint8_t i = 0; i < UINT8_MAX; i++) {
    if(GpsSerial.available()) {
      return true;
    }
    if(i >= UINT8_MAX - 1) {
      Serial.println("Serial unvailable");
      Serial.println("Please check the GPS connection.");
      return false;
    }
    delay(1);
  }
  return false;
}

bool Gps1818mk::get_header(char array[]) {
  char header[6] = {0};
  for(uint16_t i = 0; i < UINT16_MAX; i++) {
    header[5] = read_byte();

    for(uint8_t j = 0; j < 6; j++) {
      if(header[j] != array[j]) {
        break;
      }
      if(j >= 5) {
        return 1;
      }
    }

    for(uint8_t j = 0; j < 5; j++) {
      header[j] = header[j+1];
    }

    if(i >= UINT16_MAX - 1) {
      return 0;
    }
  }
  return 0;
}

bool Gps1818mk::is_data_available(void) {
  return GpsSerial.available();
}
