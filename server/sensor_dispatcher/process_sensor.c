#include "process_sensor.h"

#define DEBUG 0



void process_sensor(SensorData *data)
{
    SensorState *state = get_sensor_state(data->sensor_id);

    float avg = update_moving_average(state, data->value);

    int is_anomaly = detect_anomaly(data->sensor_id, data->value);

    int m, t, i;
    decode_sensor_id(data->sensor_id, &m, &t, &i);

    if(DEBUG){
        printf("M=%d T=%d I=%d | val=%.3f | avg=%.3f | %s\n",
           m, t, i,
           data->value,
           avg,
           is_anomaly ? "🚨 ANOMALY" : "OK");
    }
   
}