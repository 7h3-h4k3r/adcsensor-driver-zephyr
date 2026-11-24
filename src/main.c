#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>
#include "adcsensor.h"

static void sensors_stop(struct k_timer *timer_id);

#ifdef CONFIG_MQ135
const struct device *mq135 = DEVICE_DT_GET(DT_NODELABEL(mq135_sensor));
#endif

#ifdef CONFIG_WATER
const struct device *water = DEVICE_DT_GET(DT_NODELABEL(water_level_depth_sensor));
#endif

#ifdef CONFIG_SOIL
const struct device *soil = DEVICE_DT_GET(DT_NODELABEL(soil_moisture_sensor));
#endif

static void mqtt_publish_work_handler(struct k_work *work){
    struct sensor_value raw_val, pct_val;
    char json_payload[128];

    while (1) {
    #ifdef CONFIG_MQ135
    if (sensor_sample_fetch(mq135) == 0) {
        sensor_channel_get(mq135, SENSOR_GET_RAW_ADC, &raw_val);
        sensor_channel_get(mq135, SENSOR_GET_PERCENTAGE, &pct_val);

        double pct = sensor_value_to_double(&pct_val);

        snprintf(json_payload, sizeof(json_payload),
                    "{\"sensor\":\"MQ-135\",\"raw_adc\":%d,\"pollution_percent\":%.1f}",
                    raw_val.val1, pct);

        printk("%s\n", json_payload);
    }
    #endif

    #ifdef CONFIG_WATER
    if (sensor_sample_fetch(water) == 0){
        sensor_channel_get(water,SENSOR_GET_RAW_ADC,&raw_val);
        sensor_channel_get(water,SENSOR_GET_PERCENTAGE,&pct_val);

        double pct = sensor_value_to_double(&pct_val);

        snprintf(json_payload, sizeof(json_payload),
                    "{\"sensor\":\"HW-038\",\"raw_adc\":%d,\"water depth\":%.1f}",
                    raw_val.val1, pct);

        printk("%s\n", json_payload);
    }
    #endif

    #ifdef CONFIG_SOIL
    if (sensor_sample_fetch(soil) == 0){
        sensor_channel_get(soil,SENSOR_GET_RAW_ADC,&raw_val);
        sensor_channel_get(soil,SENSOR_GET_PERCENTAGE,&pct_val);

        double pct = sensor_value_to_double(&pct_val);

        snprintf(json_payload, sizeof(json_payload),
                    "{\"sensor\":\"SOIL-MS\",\"raw_adc\":%d,\"soil state\":%.1f}",
                    raw_val.val1, pct);

        printk("%s\n", json_payload);
    }
    #endif
    }
   

    return;
}
K_WORK_DEFINE(mqtt_publish_work,mqtt_publish_work_handler);

static void sensors_start(struct k_timer *timer_id){
    k_work_submit(&mqtt_publish_work);
}   




K_TIMER_DEFINE(sensors_timer,sensors_start,sensors_stop);

static void sensors_stop(struct k_timer *timer_id){
    k_timer_stop(&sensors_timer);
}


int main(void)
{
    k_timer_start(&sensors_timer, K_MSEC(1000), K_MSEC(5000));
    return 0;
}