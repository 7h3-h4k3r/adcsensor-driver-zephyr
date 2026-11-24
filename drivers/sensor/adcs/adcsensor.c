
#define DT_DRV_COMPAT custom_sensoradc


#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include "adcsensor.h"

LOG_MODULE_REGISTER(adc,CONFIG_SENSOR_LOG_LEVEL);

#define SAMPLE_COUNT 10

struct adcsensor_config{
    struct adc_dt_spec adc;
    int32_t min_adc;
    int32_t max_adc;
};


struct adcsensor_data{
    int32_t raw_avg;
    float percentage;
};


static int adcsensor_sample_fetch(const struct device *dev ,enum sensor_channel chan)

{

    ARG_UNUSED(chan);
    const struct adcsensor_config *cfg = dev->config;
    struct adcsensor_data *data = dev->data;
    int16_t raw_buf;
    int32_t sum = 0;

    struct adc_sequence sequence = {
        .buffer = &raw_buf,
        .buffer_size = sizeof(raw_buf),
    };

    for (int i = 0;i < SAMPLE_COUNT ;i++)
    {
        adc_sequence_init_dt(&cfg->adc,&sequence);
        int err = adc_read_dt(&cfg->adc,&sequence);

        if (err < 0) {
            LOG_ERR("ADC read failed %d ",err);
            return err;
        }

        sum += raw_buf;
        k_msleep(5);
    }

    data->raw_avg = sum / SAMPLE_COUNT;

    float pct = ((float)(data->raw_avg - cfg->min_adc) /
             (float)(cfg->max_adc - cfg->min_adc)) * 100.0f;

    if(pct < 0.0f){
        pct = 0.0f;
    }else if (pct > 100.0f){
        pct = 100.0f;
    }
    
    data->percentage = pct;

    return 0;
}




static int adcsensor_channel_get(const struct device *dev, 
                            enum sensor_channel chan,
                            struct sensor_value *val)
{
    struct adcsensor_data  *data = dev->data;

    switch ((int)chan){
        case SENSOR_GET_RAW_ADC:
            val->val1 = data->raw_avg;
            val->val2 = 0;
            return 0;

        case SENSOR_GET_PERCENTAGE:
            sensor_value_from_float(val,data->percentage);
            return 0;
        default:
            return -ENOTSUP;
    }


}

static const struct sensor_driver_api adcsensor_driver_api = {
    .sample_fetch = adcsensor_sample_fetch,
    .channel_get = adcsensor_channel_get,
};


static int adcsensor_init(const struct device *dev)
{
    const struct adcsensor_config *cfg = dev->config;

    if(!adc_is_ready_dt(&cfg->adc)){
        LOG_ERR("ADC device %s not ready",cfg->adc.dev->name);
        return -ENODEV;
    }

    int err = adc_channel_setup_dt(&cfg->adc);

    if(err < 0){
        LOG_ERR("ADC channel setup failed %d ",err);
        return err;
    }
    LOG_INF("ADC device %s is ready to send a Data ",cfg->adc.dev->name);
    return 0;
}



#define SENSORADC_DEFINE(inst)                                              \
    static struct adcsensor_data adcsensor_data_##inst;                         \
    static const struct adcsensor_config adcsensor_config_##inst = {            \
        .adc = ADC_DT_SPEC_INST_GET(inst),                              \
        .min_adc = DT_INST_PROP(inst, min_adc),             \
        .max_adc = DT_INST_PROP(inst, max_adc),                 \
    };                                                                  \
    SENSOR_DEVICE_DT_INST_DEFINE(inst,                                  \
                                 adcsensor_init,                            \
                                 NULL,                                  \
                                 &adcsensor_data_##inst,                    \
                                 &adcsensor_config_##inst,                  \
                                 POST_KERNEL,                           \
                                 CONFIG_SENSOR_INIT_PRIORITY,           \
                                 &adcsensor_driver_api);

DT_INST_FOREACH_STATUS_OKAY(SENSORADC_DEFINE)