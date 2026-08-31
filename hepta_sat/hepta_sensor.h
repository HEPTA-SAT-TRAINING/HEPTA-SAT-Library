/**
 * @file hepta_sensor.h
 * @brief HEPTA-SAT Sensor: BNO055 IMU + Pt100 bridge temperature circuit.
 */

#ifndef HEPTA_SENSOR_H
#define HEPTA_SENSOR_H

#include "../common/gps_fix.h"
#include "../common/hepta_sensor_base.h"
#include "../common/hepta_storage.h"
#include "../drv/camera_arducam_2mp.h"
#include "../drv/gps_gp1818mk.h"


class HeptaSensor : public HeptaSensorBase {
  public:
    HeptaSensor()
      : HeptaSensorBase(/*temp_pin=*/27),
        storage_(/*sd_cs=*/3, /*sd_tx=*/19, /*sd_rx=*/16, /*sd_sck=*/18),
        cam(/*cs=*/0) {}
    bool begin(void);
    float get_temperature(void);

    // GPS (GP-1818MK). Thin wrappers over the gps driver, mirroring how the
    // base class exposes the BNO055 IMU.
    bool gps_get_position(float* lat, float* lon, float* alt);
    bool gps_get_velocity(float* velocity, float* heading);
    bool gps_get_all(float* lat, float* lon, float* alt,
                     float* velocity, float* heading);
    bool gps_get_gpgga(GpggaData* out);
    bool gps_get_gprmc(GprmcData* out);
    bool gps_is_data_available(void);
    int  gps_read_byte(void);

    void gps_service(void);
    bool gps_get_latest(GpsFix* out);

    bool camera_snapshot(const char* filename = "picture.jpg",
                         ArducamJpegSize jpeg_size = ARDUCAM_JPEG_VGA);

    /**
     * @brief Invalidate the camera driver's cached setup so the next
     *        camera_snapshot() re-probes SPI / I2C and reloads JPEG tables.
     *
     * camera_snapshot() already does this automatically after a communication
     * failure. This method is retained for explicit hardware power-cycle flows.
     */
    void camera_invalidate(void);

  private:
    HeptaStorage storage_;
    CameraArducam2mp cam;
    Gps1818mk gps;
};


#endif /* HEPTA_SENSOR_H */
