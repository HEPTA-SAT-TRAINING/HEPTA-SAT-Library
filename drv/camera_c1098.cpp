/**
 * @file camera_c1098.cpp
 * @author Masaki Naito
 * @brief 
 * @version 0.1
 * @date 2024-08-22
 * 
 * @copyright UNISEC all rights reserved.
 * 
 */

#include "camera_c1098.h"

#include <SPI.h>
#include <SD.h>


bool CameraC1098::begin(C1098_BAUD_RATE baud_rate, C1098_JPEG_SIZE size) {
  // Skip full re-initialization if the camera is already set up with the same config.
  // If baud_rate or size differ from the stored values, invalidate and re-sync so
  // the new parameters are actually applied (the camera must be re-INITIALized).
  if (_is_setup_fin) {
    if (baud_rate != _baud_rate || size != _jpeg_size) {
      Serial.println("begin: config changed — re-syncing camera");
      invalidate();  // _is_setup_fin = false; fall through to full init below
    } else {
      CAM_SERIAL.begin(115200);
      while (CAM_SERIAL.available()) CAM_SERIAL.read();
      return true;
    }
  }

  // --- Step 1: Sync ---
  // The camera boots at 14400 baud, so start communication at that rate.
  CAM_SERIAL.begin(14400);

  // Flush any leftover bytes that arrived during the reset interval
  // (garbage from baud-rate mismatch or camera boot messages).
  delay(50);
  while (CAM_SERIAL.available()) CAM_SERIAL.read();

  bool sync_ok = false;
  Serial.println("Sync Start");
  for (uint8_t i = 0; i < SYNC_TRY_MAX; i++) {
    if (_sync()) {
      Serial.println("Sync ACK received");
      delay(10);
      if (_is_sync_ok()) {
        _send_ack();
        Serial.println("Sync OK");
        sync_ok = true;
        break;
      }
    }
  }
  if (!sync_ok) {
    Serial.println("Sync failed");
    return false;
  }

  // --- Step 2: Initialize ---
  // Send the INITIAL command to set the baud rate and JPEG resolution.
  bool init_ok = _initial(baud_rate, size);
  // According to the datasheet, a 50 ms delay is required after INITIAL.
  delay(50);
  if (init_ok) {
    Serial.println("Init OK");
  } else {
    Serial.println("Init failed");
    return false;
  }

  // --- Step 3: Set packet size ---
  // Switch to the higher baud rate negotiated in Step 2, then configure
  // the packet size used for image transfer.
  CAM_SERIAL.begin(115200);
  bool packet_set_ok = _set_package_size(PACKET_LEN);
  if (!packet_set_ok) {
    Serial.println("Set Package Size failed");
    return false;
  }
  Serial.println("Set Package Size OK");

  _is_setup_fin = true;
  _baud_rate    = baud_rate;
  _jpeg_size    = size;
  return true;
}

uint32_t CameraC1098::take_picture(void) {
  if(!_snapshot()) {
    Serial.println("Snapshot failed");
    return 0;
  }
  Serial.println("Snapshot OK");

  if(!_get_picture()) {
    Serial.println("Get Picture failed");
    return 0;
  }
  Serial.println("Get Picture OK");

  _data_len = _data_length();
  _pkg_counter = 0;  // Reset the package ID counter before reading packets
  Serial.print("Data len: ");
  Serial.println(_data_len);

  return _data_len;
}

uint16_t CameraC1098::get_packet_size(void) {
  return PACKET_LEN;
}

void CameraC1098::invalidate(void) {
  // Mark the driver as uninitialized so the next begin() call re-runs the full
  // SYNC + INITIAL sequence.  Required after a camera power-cycle because the
  // camera hardware resets to 14400 baud and must be re-synced.
  _is_setup_fin = false;
}

// Packet structure (fixed PACKET_LEN bytes total):
//   [ID_L][ID_H][DataSize_L][DataSize_H][ImageData (up to PACKET_LEN-6 bytes)][Verify_L][Verify_H]
// Returns: number of image-data bytes written (header and verify bytes excluded)
int CameraC1098::get_image_data_packet(uint8_t *buf, size_t max_size) {
  if (_data_len == 0) {
    Serial.println("No more data");
    return 0;
  }

  // Datasheet 8.1.2: request the next packet by sending an ACK with the package ID
  // AA 0E 00 00 [pkg_L] [pkg_H]  (package ID in little-endian order)
  {
    uint8_t param[MAX_PARAM_NUM] = {
      0x00, 0x00,
      (uint8_t)(_pkg_counter & 0xFF),
      (uint8_t)((_pkg_counter >> 8) & 0xFF)
    };
    _send_cmd(C1098_CMD_ACK, param);
  }

  const uint32_t timeout_ms = 500;

  // Step 1: Read the 4-byte header to determine DataSize.
  // Packet layout: [ID_L][ID_H][DataSize_L][DataSize_H][ImageData×DataSize][Verify×2]
  // Datasheet note: "the last packet size varies depending on the image size"
  //   Normal packet : DataSize = PACKET_LEN-6 = 506  -> total 512 bytes
  //   Last packet   : DataSize < 506                 -> total DataSize+6 bytes (< 512)
  uint8_t header[4];
  for (uint8_t i = 0; i < 4; i++) {
    uint32_t start = millis();
    while (!CAM_SERIAL.available()) {
      if (millis() - start > timeout_ms) {
        Serial.println("Timeout waiting for packet header");
        return -1;
      }
    }
    header[i] = CAM_SERIAL.read();
  }

  uint16_t data_size = (uint16_t)header[2] | ((uint16_t)header[3] << 8);

  if (data_size == 0 || data_size > PACKET_LEN - 6) {
    Serial.print("Invalid packet data_size: ");
    Serial.println(data_size);
    return -1;
  }
  if (max_size < data_size) {
    Serial.println("Output buffer too small");
    return -1;
  }

  // Clamp to remaining byte count for the last packet
  uint16_t bytes_to_write = (data_size > _data_len)
                              ? (uint16_t)_data_len
                              : data_size;

  // Step 2: Receive DataSize bytes of image data and copy them into the buffer
  for (uint16_t i = 0; i < data_size; i++) {
    uint32_t start = millis();
    while (!CAM_SERIAL.available()) {
      if (millis() - start > timeout_ms) {
        Serial.println("Timeout waiting for image data");
        return -1;
      }
    }
    uint8_t b = CAM_SERIAL.read();
    if (i < bytes_to_write) {
      buf[i] = b;
    }
  }

  // Step 3: Read and discard the 2 verify bytes
  for (uint8_t i = 0; i < 2; i++) {
    uint32_t start = millis();
    while (!CAM_SERIAL.available()) {
      if (millis() - start > timeout_ms) {
        Serial.println("Timeout waiting for verify bytes");
        return -1;
      }
    }
    CAM_SERIAL.read();
  }

  _pkg_counter++;
  _data_len -= bytes_to_write;

  // After all data has been received, send the final ACK 0xF0F0 (datasheet p.7)
  if (_data_len == 0) {
    uint8_t final_param[MAX_PARAM_NUM] = {0x00, 0x00, 0xF0, 0xF0};
    _send_cmd(C1098_CMD_ACK, final_param);
    Serial.println("Transfer complete, sent F0F0 ACK");
  }

  return (int)bytes_to_write;
}

/* ---------------------------------------------------------------
  Private functions
--------------------------------------------------------------- */

bool CameraC1098::_initial(C1098_BAUD_RATE baud_rate, C1098_JPEG_SIZE size) {
  uint8_t param[MAX_PARAM_NUM] = {0, 0x07, 0x00, 0};
  param[0] = baud_rate;
  param[3] = size;

  _send_cmd(C1098_CMD_INITIAL, param);

  // delay needed before reading ack
  delay(10);
  return _is_ack_ok();
}

bool CameraC1098::_get_picture(void) {
  uint8_t param[MAX_PARAM_NUM] = {0x01, 0, 0, 0};

  _send_cmd(C1098_CMD_GET_PICTURE, param);

  // delay needed before reading ack
  delay(10);
  return _is_ack_ok();
}

bool CameraC1098::_snapshot(void) {
  uint8_t param[MAX_PARAM_NUM] = {0};

  _send_cmd(C1098_CMD_SNAPSHOT, param);

  // delay needed before reading ack
  delay(10);
  return _is_ack_ok();
}

bool CameraC1098::_set_package_size(uint16_t size) {
  uint8_t param[MAX_PARAM_NUM] = {0x08, 0, 0, 0};

  param[1] = size & 0xFF;
  param[2] = size >> 8;

  _send_cmd(C1098_CMD_SET_PACKAGE_SIZE, param);

  // delay needed before reading ack
  delay(10);
  return _is_ack_ok();
}

bool CameraC1098::_reset(void) {
  uint8_t param[MAX_PARAM_NUM] = {0};

  _send_cmd(C1098_CMD_RESET, param);

  // delay needed before reading ack
  delay(20);
  if(!_is_ack_ok()) {
    Serial.println("Reset failed");
    return false;
  } else {
    Serial.println("Reset OK");
    return true;
  }
}

uint32_t CameraC1098::_data_length(void){
  const uint32_t timeout_ms = 500;
  uint32_t start = millis();
  while (CAM_SERIAL.available() < CMD_PACKET_LEN) {
    if (millis() - start > timeout_ms) {
      Serial.println("_data_length timeout");
      return 0;
    }
  }

  uint8_t buf[CMD_PACKET_LEN] = {0};
  for(uint8_t i = 0; i < CMD_PACKET_LEN; i++) {
    buf[i] = _get_data();
  }

  // Datasheet p.8: DATA LENGTH packet format
  //   AA  0A  01  [Byte0=LSB]  [Byte1]  [Byte2=MSB]
  //   [0] [1] [2]     [3]        [4]       [5]
  // Example: AA 0A 01 90 22 00 -> size = 0x90 | (0x22<<8) | (0x00<<16) = 0x002290 = 8848
  return (uint32_t)buf[3] | ((uint32_t)buf[4] << 8) | ((uint32_t)buf[5] << 16);
}

bool CameraC1098::_sync(void) {
  uint8_t param[MAX_PARAM_NUM] = {0};

  _send_cmd(C1098_CMD_SYNC, param);

  // delay needed before reading ack
  delay(10);
  return _is_ack_ok();
}

void CameraC1098::_send_ack(void) {
  uint8_t param[MAX_PARAM_NUM] = {0};

  _send_cmd(C1098_CMD_ACK, param);
}

bool CameraC1098::_is_ack_ok(void) {
  const uint32_t timeout_ms = 100;
  uint32_t start = millis();
  while (CAM_SERIAL.available() < CMD_PACKET_LEN) {
    if (millis() - start > timeout_ms) {
      Serial.print("[DBG] _is_ack_ok timeout, available=");
      Serial.println(CAM_SERIAL.available());
      // Discard partial / stale bytes so they don't pollute the next call.
      while (CAM_SERIAL.available()) CAM_SERIAL.read();
      return false;
    }
  }

  uint8_t buf[CMD_PACKET_LEN] = {0};
  for(uint8_t i = 0; i < CMD_PACKET_LEN; i++) {
    buf[i] = CAM_SERIAL.read();
  }
  if(buf[1] == C1098_CMD_ACK) {
    return true;
  }

  return false;
}

bool CameraC1098::_is_sync_ok(void) {
  const uint32_t timeout_ms = 100;
  uint32_t start = millis();
  while (CAM_SERIAL.available() < CMD_PACKET_LEN) {
    if (millis() - start > timeout_ms) {
      Serial.println("serial not available");
      while (CAM_SERIAL.available()) CAM_SERIAL.read();
      return false;
    }
  }

  uint8_t buf[CMD_PACKET_LEN] = {0};
  Serial.println("is_sync_ok");
  for(uint8_t i = 0; i < CMD_PACKET_LEN; i++) {
    buf[i] = CAM_SERIAL.read();
    Serial.print(buf[i], HEX);
  }
  if(buf[1] == C1098_CMD_SYNC) {
    return true;
  }

  return false;
}


void CameraC1098::_send_cmd(C1098_CMD cmd, uint8_t param[]) {
  CAM_SERIAL.write(CMD_START_BYTE);
  CAM_SERIAL.write((uint8_t)cmd);

  for(uint8_t i = 0; i < MAX_PARAM_NUM; i++) {
    CAM_SERIAL.write(param[i]);
  }
}

uint8_t CameraC1098::_get_data(void) {
  return CAM_SERIAL.read();
}
