/**
 * @file camera_c1098.h
 * @author Masaki Naito
 * @brief 
 * @version 0.1
 * @date 2024-08-22
 * 
 * @copyright UNISEC all rights reserved.
 * 
 */


#ifndef CAMERA_C1098
#define CAMERA_C1098

#include <Arduino.h>

typedef enum {
  C1098_BAUD_RATE_14400  = 0x07,
  C1098_BAUD_RATE_28800  = 0x06,
  C1098_BAUD_RATE_57600  = 0x05,
  C1098_BAUD_RATE_115200 = 0x04,
  C1098_BAUD_RATE_230400 = 0x03,
  C1098_BAUD_RATE_460800 = 0x02
} C1098_BAUD_RATE;

typedef enum {
  C1098_JPEG_SIZE_QVGA = 0x05,  // 320x240
  C1098_JPEG_SIZE_VGA = 0x07,   // 640x480
} C1098_JPEG_SIZE;

typedef enum {
  C1098_CMD_INITIAL = 0x01,
  C1098_CMD_GET_PICTURE = 0x04,
  C1098_CMD_SNAPSHOT = 0x05,
  C1098_CMD_SET_PACKAGE_SIZE = 0x06,
  C1098_CMD_RESET = 0x08,
  C1098_CMD_DATA_LEN= 0x0A,
  C1098_CMD_SYNC = 0x0D,
  C1098_CMD_ACK= 0x0E,
  C1098_CMD_NACK = 0x0F,
} C1098_CMD;

#define CAM_SERIAL Serial1

#define MAX_PARAM_NUM 4
#define CMD_PACKET_LEN 6
#define SYNC_TRY_MAX 60
#define CMD_START_BYTE 0xAA

class CameraC1098 {
  public:

    /**
     * @brief Initialize the camera with specified baud rate and JPEG size
     * @param baud_rate Baud rate for communication
     * @param size JPEG size to be used
     * @return true if initialization is successful, false otherwise
     */
    bool begin(C1098_BAUD_RATE baud_rate, C1098_JPEG_SIZE size);

    /**
     * @brief Take a picture with the camera
     * @return The length of the data received from the camera
     */
    uint32_t take_picture(void);


    /**
     * @brief Return the fixed packet size used for image transfer
     * @return Packet size in bytes (always PACKET_LEN = 512)
     */
    uint16_t get_packet_size(void);
  
    /**
     * @brief Retrieve one packet of image data from the camera
     * @param buf    Receive buffer (must be at least PACKET_LEN bytes)
     * @param max_size Size of the buffer in bytes
     * @return Number of image-data bytes written to buf
     *         (may be less than PACKET_LEN for the last packet),
     *         0 when all data has been received, or -1 on error
     */
    int get_image_data_packet(uint8_t *buf, size_t max_size);

    /**
     * @brief Invalidate the setup state so that the next begin() call performs
     *        a full SYNC + INITIAL sequence at 14400 baud.
     *
     * Call this after power-cycling the camera (e.g. via HeptaEps::switch_3V3_off()
     * followed by switch_3V3_on()) to ensure the driver re-syncs with the camera
     * hardware instead of assuming it is already initialized.
     */
    void invalidate(void);

  private:
    const uint16_t PACKET_LEN = 512;
    bool            _is_setup_fin = false;
    C1098_BAUD_RATE _baud_rate    = C1098_BAUD_RATE_115200;  // baud rate set by last successful begin()
    C1098_JPEG_SIZE _jpeg_size    = C1098_JPEG_SIZE_VGA;     // JPEG size set by last successful begin()
    uint32_t _data_len    = 0;
    uint16_t _pkg_counter = 0;   // Package ID counter incremented with each received packet

    // command
    bool _initial(C1098_BAUD_RATE baud_rate, C1098_JPEG_SIZE size);
    bool _get_picture(void);
    bool _snapshot(void);
    bool _set_package_size(uint16_t size);
    uint32_t _data_length(void);
    bool _sync(void);
    void _send_ack(void);        // Send a command ACK packet: AA 0E 00 00 00 00

    bool _is_ack_ok(void);
    bool _is_sync_ok(void);

    void _send_cmd(C1098_CMD cmd, uint8_t param[]);

    // Map an interface-speed code to the actual UART bit rate (bps).
    static long _baud_to_bps(C1098_BAUD_RATE baud);
    // Read a single byte with timeout. Returns false on timeout.
    bool _read_byte(uint8_t *out, uint32_t timeout_ms);
    // Read a 6-byte command frame, hunting for the 0xAA start byte first so a
    // stray/noise byte cannot permanently desync the 6-byte framing. Returns
    // false on timeout / no frame.
    bool _read_cmd_packet(uint8_t *buf, uint32_t timeout_ms);
};

#endif /* CAMERA_C1098 */
