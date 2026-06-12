/**
 * @file hepta_image_tx.h
 * @brief HP binary image downlink protocol over HeptaComBase.
 *
 * Implements the HEPTA-SAT image serial protocol (IMG_BEGIN, HP packets,
 * IMG_END) defined for command 'p' downlink. Sketches do not include this
 * header directly; use HeptaComBase::downlink_image_file().
 */

#ifndef HEPTA_IMAGE_TX_H
#define HEPTA_IMAGE_TX_H

#include <Arduino.h>
#include <SD.h>

#include "hepta_com_base.h"


class HeptaImageTx {
  public:
    static constexpr uint8_t ERROR_IMAGE_NOT_AVAILABLE   = 0x01;
    static constexpr uint8_t ERROR_CAMERA_CAPTURE_FAILED = 0x02;
    static constexpr uint8_t ERROR_IMAGE_SIZE_TOO_LARGE  = 0x03;
    static constexpr uint8_t ERROR_INTERNAL_BUFFER       = 0x04;

    static uint16_t crc16_ccitt_false(const uint8_t* data, size_t length);

    static bool send_jpeg_file(HeptaComBase& com, File& file, uint16_t image_id = 0);
    static bool send_jpeg_buffer(HeptaComBase& com,
                                 const uint8_t* data,
                                 uint32_t size,
                                 uint16_t image_id = 0);
    static void send_error(HeptaComBase& com, uint8_t error_code);
    static bool send_test_jpeg(HeptaComBase& com);
};

#endif /* HEPTA_IMAGE_TX_H */
