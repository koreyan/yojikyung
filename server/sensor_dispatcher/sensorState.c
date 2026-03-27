#define MAX_SENSORS 256

typedef struct {
    float last_value;

    // 이동 평균
    float window[10];
    int   w_idx;
    int   w_count;
    float sum;

} SensorState;

SensorState sensor_table[MAX_SENSORS];