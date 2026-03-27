#include "anomaly_detection.h"

int detect_anomaly(uint16_t sensor_id, float value)
{
    SensorClass *cls = find_sensor_class(sensor_id);

    if (cls == NULL) {
        printf("⚠️ No class for sensor 0x%04X\n", sensor_id);
        return 0;
    }

    if (value < cls->normal_min || value > cls->normal_max)
        return 1;

    return 0;
}