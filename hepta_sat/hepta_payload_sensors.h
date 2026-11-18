#ifndef HEPTA_PAYLOAD_SENSORS_H
#define HEPTA_PAYLOAD_SENSORS_H

#include "hepta_payload_pins.h"
#include "../drv/air_quality_mp503.h"
#include "../drv/hc_sr04.h"
#include "../drv/light_temt6000.h"
#include "../drv/uv_guva_s12sd.h"

class HeptaLightTemt6000 : public LightTemt6000 {
  public:
    bool begin(HeptaAnalogInput input) {
      return LightTemt6000::begin(hepta_payload::adc_cs_pin, hepta_payload::adc_channel(input));
    }
};

class HeptaUvGuvaS12sd : public UvGuvaS12sd {
  public:
    bool begin(HeptaAnalogInput input) {
      return UvGuvaS12sd::begin(hepta_payload::adc_cs_pin, hepta_payload::adc_channel(input));
    }
};

class HeptaAirQualityMp503 : public AirQualityMp503 {
  public:
    bool begin(HeptaAnalogInput input) {
      return AirQualityMp503::begin(hepta_payload::adc_cs_pin, hepta_payload::adc_channel(input));
    }

    // Use after the powered sensor has already warmed up for 20 seconds.
    bool begin_after_warmup(HeptaAnalogInput input) {
      return AirQualityMp503::begin(hepta_payload::adc_cs_pin, hepta_payload::adc_channel(input),
                                    3.3f, /*skip_warmup=*/true);
    }
};

class HeptaHcSr04 : public HcSr04 {
  public:
    bool begin(HeptaDigitalIo trig, HeptaDigitalIo echo) {
      return trig != echo && HcSr04::begin(hepta_payload::gpio(trig), hepta_payload::gpio(echo));
    }
};

#endif
