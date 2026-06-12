#include "hepta_image_tx.h"

#include <string.h>


namespace {

constexpr uint8_t kMagic0 = 0x48;  // 'H'
constexpr uint8_t kMagic1 = 0x50;  // 'P'

constexpr uint8_t kPacketTypeStart = 0x01;
constexpr uint8_t kPacketTypeData  = 0x02;
constexpr uint8_t kPacketTypeEnd   = 0x03;
constexpr uint8_t kPacketTypeError = 0x04;

constexpr uint8_t kFormatJpeg = 0x01;

constexpr uint16_t kPayloadMax = 512;
constexpr size_t kHeaderSize   = 11;

// Minimal valid 1x1 grey JPEG for protocol testing without a camera.
static const uint8_t kTestJpeg[] = {
  0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46, 0x00, 0x01,
  0x01, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0xFF, 0xDB, 0x00, 0x43,
  0x00, 0x08, 0x06, 0x06, 0x07, 0x06, 0x05, 0x08, 0x07, 0x07, 0x07, 0x09,
  0x09, 0x08, 0x0A, 0x0C, 0x14, 0x0D, 0x0C, 0x0B, 0x0B, 0x0C, 0x19, 0x12,
  0x13, 0x0F, 0x14, 0x1D, 0x1A, 0x1F, 0x1E, 0x1D, 0x1A, 0x1C, 0x1C, 0x20,
  0x24, 0x2E, 0x27, 0x20, 0x22, 0x2C, 0x23, 0x1C, 0x1C, 0x28, 0x37, 0x29,
  0x2C, 0x30, 0x31, 0x34, 0x34, 0x34, 0x1F, 0x27, 0x39, 0x3D, 0x38, 0x32,
  0x3C, 0x2E, 0x33, 0x34, 0x32, 0xFF, 0xC0, 0x00, 0x0B, 0x08, 0x00, 0x01,
  0x00, 0x01, 0x01, 0x01, 0x11, 0x00, 0xFF, 0xC4, 0x00, 0x14, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x08, 0xFF, 0xC4, 0x00, 0x14, 0x10, 0x01, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xFF, 0xDA, 0x00, 0x08, 0x01, 0x01, 0x00, 0x00, 0x3F, 0x00,
  0x7F, 0xFF, 0xD9
};

void write_le16(uint8_t* buf, uint16_t value) {
  buf[0] = static_cast<uint8_t>(value & 0xFF);
  buf[1] = static_cast<uint8_t>((value >> 8) & 0xFF);
}

uint16_t update_crc16_ccitt_false(uint16_t crc, const uint8_t* data, size_t length) {
  for (size_t i = 0; i < length; i++) {
    crc ^= static_cast<uint16_t>(data[i]) << 8;
    for (uint8_t bit = 0; bit < 8; bit++) {
      if (crc & 0x8000) {
        crc = static_cast<uint16_t>((crc << 1) ^ 0x1021);
      } else {
        crc = static_cast<uint16_t>(crc << 1);
      }
    }
  }
  return crc;
}

uint16_t calc_packet_crc(uint8_t type,
                         uint16_t seq,
                         uint16_t total,
                         uint16_t len,
                         const uint8_t* payload) {
  uint8_t crc_buf[1 + 2 + 2 + 2 + kPayloadMax];
  size_t offset = 0;

  crc_buf[offset++] = type;
  write_le16(crc_buf + offset, seq);
  offset += 2;
  write_le16(crc_buf + offset, total);
  offset += 2;
  write_le16(crc_buf + offset, len);
  offset += 2;

  if (payload != nullptr && len > 0) {
    memcpy(crc_buf + offset, payload, len);
    offset += len;
  }

  return HeptaImageTx::crc16_ccitt_false(crc_buf, offset);
}

bool send_image_packet(HeptaComBase& com,
                       uint8_t type,
                       uint16_t seq,
                       uint16_t total,
                       const uint8_t* payload,
                       uint16_t len) {
  if (len > kPayloadMax) {
    return false;
  }

  uint16_t crc = calc_packet_crc(type, seq, total, len, payload);

  uint8_t header[kHeaderSize];
  header[0] = kMagic0;
  header[1] = kMagic1;
  header[2] = type;
  write_le16(header + 3, seq);
  write_le16(header + 5, total);
  write_le16(header + 7, len);
  write_le16(header + 9, crc);

  if (!com.send(header, sizeof(header))) {
    return false;
  }

  if (payload != nullptr && len > 0) {
    if (!com.send(payload, len)) {
      return false;
    }
  }

  return true;
}

bool send_start_packet(HeptaComBase& com,
                       uint16_t total_packet_count,
                       uint16_t image_id,
                       uint32_t image_size,
                       uint16_t image_crc) {
  uint8_t payload[9];
  payload[0] = kFormatJpeg;
  write_le16(payload + 1, image_id);
  payload[3] = static_cast<uint8_t>(image_size & 0xFF);
  payload[4] = static_cast<uint8_t>((image_size >> 8) & 0xFF);
  payload[5] = static_cast<uint8_t>((image_size >> 16) & 0xFF);
  payload[6] = static_cast<uint8_t>((image_size >> 24) & 0xFF);
  write_le16(payload + 7, image_crc);

  return send_image_packet(com,
                         kPacketTypeStart,
                         0,
                         total_packet_count,
                         payload,
                         sizeof(payload));
}

uint16_t data_packet_count_for_size(uint32_t image_size) {
  return static_cast<uint16_t>((image_size + kPayloadMax - 1) / kPayloadMax);
}

bool send_data_packets_from_buffer(HeptaComBase& com,
                                   const uint8_t* data,
                                   uint32_t image_size,
                                   uint16_t total_packet_count) {
  uint16_t data_packet_count = data_packet_count_for_size(image_size);

  for (uint16_t i = 0; i < data_packet_count; i++) {
    uint32_t offset = static_cast<uint32_t>(i) * kPayloadMax;
    uint16_t len = kPayloadMax;

    if (offset + len > image_size) {
      len = static_cast<uint16_t>(image_size - offset);
    }

    if (!send_image_packet(com,
                           kPacketTypeData,
                           i + 1,
                           total_packet_count,
                           data + offset,
                           len)) {
      return false;
    }
  }

  return true;
}

bool send_data_packets_from_file(HeptaComBase& com,
                                 File& file,
                                 uint32_t image_size,
                                 uint16_t total_packet_count) {
  uint8_t buf[kPayloadMax];
  uint16_t data_packet_count = data_packet_count_for_size(image_size);

  if (!file.seek(0)) {
    return false;
  }

  for (uint16_t i = 0; i < data_packet_count; i++) {
    uint32_t offset = static_cast<uint32_t>(i) * kPayloadMax;
    uint16_t len = kPayloadMax;

    if (offset + len > image_size) {
      len = static_cast<uint16_t>(image_size - offset);
    }

    size_t read_len = file.read(buf, len);
    if (read_len != len) {
      return false;
    }

    if (!send_image_packet(com,
                           kPacketTypeData,
                           i + 1,
                           total_packet_count,
                           buf,
                           len)) {
      return false;
    }
  }

  return true;
}

bool send_end_and_footer(HeptaComBase& com, uint16_t total_packet_count) {
  if (!send_image_packet(com,
                         kPacketTypeEnd,
                         total_packet_count - 1,
                         total_packet_count,
                         nullptr,
                         0)) {
    return false;
  }

  com.send("\nIMG_END\n");
  return true;
}

bool send_picture_from_buffer(HeptaComBase& com,
                              const uint8_t* data,
                              uint32_t image_size,
                              uint16_t image_crc,
                              uint16_t image_id) {
  uint16_t total_packet_count = data_packet_count_for_size(image_size) + 2;

  com.send("IMG_BEGIN\n");

  if (!send_start_packet(com,
                         total_packet_count,
                         image_id,
                         image_size,
                         image_crc)) {
    return false;
  }

  if (!send_data_packets_from_buffer(com, data, image_size, total_packet_count)) {
    return false;
  }

  return send_end_and_footer(com, total_packet_count);
}

bool send_picture_from_file(HeptaComBase& com,
                            File& file,
                            uint32_t image_size,
                            uint16_t image_crc,
                            uint16_t image_id) {
  uint16_t total_packet_count = data_packet_count_for_size(image_size) + 2;

  com.send("IMG_BEGIN\n");

  if (!send_start_packet(com,
                         total_packet_count,
                         image_id,
                         image_size,
                         image_crc)) {
    return false;
  }

  if (!send_data_packets_from_file(com, file, image_size, total_packet_count)) {
    return false;
  }

  return send_end_and_footer(com, total_packet_count);
}

bool crc16_file_contents(File& file, uint16_t* out_crc) {
  uint8_t buf[kPayloadMax];
  uint16_t crc = 0xFFFF;

  if (!file.seek(0)) {
    return false;
  }

  while (true) {
    int read_len = file.read(buf, sizeof(buf));
    if (read_len < 0) {
      return false;
    }
    if (read_len == 0) {
      break;
    }
    crc = update_crc16_ccitt_false(crc, buf, static_cast<size_t>(read_len));
  }

  *out_crc = crc;
  return true;
}

}  // namespace


uint16_t HeptaImageTx::crc16_ccitt_false(const uint8_t* data, size_t length) {
  return update_crc16_ccitt_false(0xFFFF, data, length);
}

bool HeptaImageTx::send_jpeg_buffer(HeptaComBase& com,
                                    const uint8_t* data,
                                    uint32_t size,
                                    uint16_t image_id) {
  if (data == nullptr || size == 0) {
    send_error(com, ERROR_IMAGE_NOT_AVAILABLE);
    return false;
  }

  uint16_t image_crc = crc16_ccitt_false(data, size);
  return send_picture_from_buffer(com, data, size, image_crc, image_id);
}

bool HeptaImageTx::send_jpeg_file(HeptaComBase& com, File& file, uint16_t image_id) {
  if (!file) {
    send_error(com, ERROR_IMAGE_NOT_AVAILABLE);
    return false;
  }

  uint32_t image_size = file.size();
  if (image_size == 0) {
    send_error(com, ERROR_IMAGE_NOT_AVAILABLE);
    return false;
  }

  uint16_t image_crc = 0;
  if (!crc16_file_contents(file, &image_crc)) {
    send_error(com, ERROR_INTERNAL_BUFFER);
    return false;
  }

  return send_picture_from_file(com, file, image_size, image_crc, image_id);
}

void HeptaImageTx::send_error(HeptaComBase& com, uint8_t error_code) {
  com.send("IMG_BEGIN\n");
  send_image_packet(com, kPacketTypeError, 0, 1, &error_code, 1);
  com.send("\nIMG_END\n");
}

bool HeptaImageTx::send_test_jpeg(HeptaComBase& com) {
  return send_jpeg_buffer(com, kTestJpeg, sizeof(kTestJpeg), 0);
}
