#include "sensor_id_util.h"


extern SensorClass sensor_classes[];
extern const int SENSOR_CLASS_COUNT;

// sensor_id → class_id 추출 후 해당 SensorClass 찾기
// → 센서마다 정상 범위(min/max)를 얻기 위해 필요
SensorClass* find_sensor_class(uint16_t sensor_id)
{
    uint8_t class_id = GET_CLASS_ID(sensor_id); // 상위 8비트: module + type

    for (int i = 0; i < SENSOR_CLASS_COUNT; i++) {
        if (sensor_classes[i].class_id == class_id) {
            return &sensor_classes[i]; // 해당 클래스 반환
        }
    }
    return NULL; // 못 찾으면 NULL
}

void decode_sensor_id(uint16_t id, int *module, int *type, int *index)
{
    *module = (id >> 12) & 0x0F;
    *type   = (id >> 8)  & 0x0F;
    *index  = id & 0xFF;
}