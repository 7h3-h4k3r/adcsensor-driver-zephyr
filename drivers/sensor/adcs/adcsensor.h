#ifndef ADCSENSOR_H_
#define ADCSENSOR_H_

#include <zephyr/drivers/sensor.h>

enum adcsensor_channel {
    SENSOR_GET_RAW_ADC = SENSOR_CHAN_PRIV_START,
    SENSOR_GET_PERCENTAGE,
};



#endif /* ADCSENSOR_H_ */