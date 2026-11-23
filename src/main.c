#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>

#define SENSOR_CHAN_MQ135_PERCENT (SENSOR_CHAN_PRIV_START + 1)

int main(void)
{
    const struct device *mq135 = DEVICE_DT_GET(DT_NODELABEL(mq135_sensor));
    const struct device *water = DEVICE_DT_GET(DT_NODELABEL(water_level_depth_sensor));
    const struct device *soil  = DEVICE_DT_GET(DT_NODELABEL(soil_moisture_sensor));
    struct sensor_value raw_val, pct_val;
    char json_payload[128];

    if (!device_is_ready(mq135)) {
        printk("MQ-135 device not ready!\n");
        return -1;
    }

    while (1) {
        if (adcsensor_sample_fetch(mq135) == 0) {
            adcsensor_channel_get(mq135, SENSOR_CHAN_GAS_RES, &raw_val);
            adcsensor_channel_get(mq135, SENSOR_CHAN_MQ135_PERCENT, &pct_val);

            double pct = sensor_value_to_double(&pct_val);

            snprintf(json_payload, sizeof(json_payload),
                     "{\"sensor\":\"MQ-135\",\"raw_adc\":%d,\"pollution_percent\":%.1f}",
                     raw_val.val1, pct);

            printk("%s\n", json_payload);
        }
        k_sleep(K_SECONDS(2));
    }
    return 0;
}