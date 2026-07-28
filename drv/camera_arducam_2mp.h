/**
 * @file camera_arducam_2mp.h
 * @brief Minimal driver for Arducam Mini 2MP Plus (OV2640 + ArduChip).
 *
 * SPI (ArduChip FIFO / commands) + I2C (OV2640 sensor). No external
 * ArduCAM library — only Arduino Wire and SPI.
 */

#ifndef CAMERA_ARDUCAM_2MP_H
#define CAMERA_ARDUCAM_2MP_H

#include <Arduino.h>

#include "ov2640_regs.h"

enum ArducamJpegSize : uint8_t {
  ARDUCAM_JPEG_QQVGA = 0,  // 160x120
  ARDUCAM_JPEG_QVGA  = 1,  // 320x240
  ARDUCAM_JPEG_VGA   = 2,  // 640x480 (default)
  ARDUCAM_JPEG_SVGA  = 3,  // 800x600
  ARDUCAM_JPEG_UXGA  = 4,  // 1600x1200
};

class CameraArducam2mp {
  public:
    explicit CameraArducam2mp(uint8_t cs_pin = 0);

    /**
     * @brief Probe ArduChip + OV2640 and load JPEG register tables.
     * @param jpeg_size One of QQVGA / QVGA / VGA / SVGA / UXGA (default VGA).
     * @return true on success.
     *
     * SPI must already be configured (e.g. by HeptaCdh / HeptaStorage).
     */
    bool begin(ArducamJpegSize jpeg_size = ARDUCAM_JPEG_VGA);

    /**
     * @brief Capture one JPEG into the ArduChip FIFO.
     * @return FIFO byte length, or 0 on failure.
     */
    uint32_t take_picture(void);

    /**
     * @brief Read the next chunk of FIFO JPEG data.
     * @return Bytes written to buf (>0), 0 when done, -1 on error.
     *
     * Call repeatedly after a successful take_picture() until 0.
     * CS is released between calls so the shared SPI bus can talk to the SD card.
     */
    int get_image_data_packet(uint8_t* buf, size_t max_size);

    /** @brief Drop cached setup so the next begin() re-probes the camera. */
    void invalidate(void);

  private:
    static constexpr uint8_t ARDUCHIP_TEST1  = 0x00;
    static constexpr uint8_t ARDUCHIP_FIFO   = 0x04;
    static constexpr uint8_t ARDUCHIP_GPIO   = 0x06;
    static constexpr uint8_t CPLD_RESET_REG  = 0x07;
    static constexpr uint8_t BURST_FIFO_READ = 0x3C;
    static constexpr uint8_t ARDUCHIP_TRIG   = 0x41;
    static constexpr uint8_t FIFO_SIZE1      = 0x42;
    static constexpr uint8_t FIFO_SIZE2      = 0x43;
    static constexpr uint8_t FIFO_SIZE3      = 0x44;

    static constexpr uint8_t FIFO_CLEAR_MASK = 0x01;
    static constexpr uint8_t FIFO_START_MASK = 0x02;
    static constexpr uint8_t CAP_DONE_MASK   = 0x08;
    static constexpr uint8_t GPIO_RESET_MASK = 0x01;  // 1 = sensor normal
    static constexpr uint8_t GPIO_PWDN_MASK  = 0x02;  // 1 = sensor standby
    static constexpr uint8_t GPIO_PWREN_MASK = 0x04;  // 1 = sensor LDO enable

    static constexpr uint32_t MAX_FIFO_SIZE      = 0x7FFFF;  // 512 KiB (2MP Plus)
    static constexpr uint32_t CAPTURE_TIMEOUT_MS = 3000;
    static constexpr uint32_t SPI_HZ             = 4000000;

    uint8_t _cs_pin;
    bool _initialized = false;
    ArducamJpegSize _jpeg_size = ARDUCAM_JPEG_VGA;

    uint32_t _fifo_len = 0;
    uint32_t _fifo_remaining = 0;

    void _cs_low(void);
    void _cs_high(void);
    void _spi_write_reg(uint8_t addr, uint8_t data);
    uint8_t _spi_read_reg(uint8_t addr);

    bool _sensor_write(uint8_t reg, uint8_t val);
    bool _sensor_read(uint8_t reg, uint8_t* val);
    bool _sensor_write_list(const Ov2640Reg* list);

    bool _probe_spi(void);
    bool _probe_sensor(void);
    bool _load_jpeg_tables(ArducamJpegSize jpeg_size);
    bool _wait_capture_done(void);
    uint32_t _read_fifo_length(void);
};

#endif /* CAMERA_ARDUCAM_2MP_H */
