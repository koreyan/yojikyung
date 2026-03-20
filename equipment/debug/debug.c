#include "debug.h"

#define EXPECTED_SENSOR_COUNT 128

#define GET_MODULE(id)   (((id) >> 12) & 0x0F)
#define GET_TYPE(id)     (((id) >> 8)  & 0x0F)
#define GET_INDEX(id)    ((id) & 0xFF)

void validate_sensors(Sensor sensors[], int count)
{
    printf("==== Sensor Validation ====\n");

    // 1. 개수 검증
    if(count != EXPECTED_SENSOR_COUNT)
    {
        printf("❌ 센서 개수 이상: %d (expected: %d)\n", count, EXPECTED_SENSOR_COUNT);
    }
    else
    {
        printf("✅ 센서 개수 정상: %d\n", count);
    }

    // 2. ID 중복 체크
    for(int i = 0; i < count; i++)
    {
        for(int j = i + 1; j < count; j++)
        {
            if(sensors[i].sensor_id == sensors[j].sensor_id)
            {
                printf("❌ 중복 ID 발견: 0x%04X (index %d, %d)\n",
                    sensors[i].sensor_id, i, j);
            }
        }
    }

    // 3. 값 검증 + 출력
    for(int i = 0; i < count; i++)
    {
        uint16_t id = sensors[i].sensor_id;

        int m = GET_MODULE(id);
        int t = GET_TYPE(id);
        int idx = GET_INDEX(id);

        printf("[%03d] ID=0x%04X | M=%d T=%d I=%d | period=%u\n",
            i, id, m, t, idx, sensors[i].period_ms);

        // 유효성 체크
        if(m > MODULE_POWER)
            printf("   ❌ INVALID MODULE!\n");

        if(t > SENSOR_POWER_CONSUMPTION)
            printf("   ❌ INVALID TYPE!\n");

        if(idx > 20) // 네 설계 기준에 맞게 조정
            printf("   ⚠️ INDEX 비정상 (idx=%d)\n", idx);
    }
}