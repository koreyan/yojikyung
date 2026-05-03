#include "sensor_state.h"

static SensorState states[MAX_SENSOR];
static int state_count = 0;

/* sensor_id 기반으로 상태 가져오기 */
SensorState* get_sensor_state(uint16_t sensor_id)
{
    for (int i = 0; i < state_count; i++) {
        if (states[i].sensor_id == sensor_id)
            return &states[i];
    }

    // 신규 생성
    SensorState *s = &states[state_count++];
    memset(s, 0, sizeof(SensorState));
    s->sensor_id = sensor_id;

    return s;
}

void get_all_sensor_states(SensorState** out_states, int* out_count) {
    *out_states = states;
    *out_count = state_count;
}