#ifndef DATA_GENERATOR_H
#define DATA_GENERATOR_H
#include "common.h"
#include "sensor_type.h"
#include "sensor_class.h"
#include "sensor_state.h"
#include "sensor_data.h"
#include "sensor_id.h"
#include "sensor_id_utill.h"

#include "util.h"

#define ABNORMAL_PROB 0.0001f   // 0.01% 확률로 이상치 발생
#define DELTA_RATIO   0.01f     // 변화 폭 (전체 범위의 1%)


//------------------------------------------------------------
// 센서 구성 생성
//------------------------------------------------------------
int init_sensors(Sensor sensors[]);

// 센서 초기값을 정상 범위 내에서 랜덤으로 설정
// → 시작부터 자연스러운 값 분포 생성
void init_sensor_states(SensorState* states, Sensor* sensors, int count);

// 센서 값을 "이전 값 기반으로" 업데이트
// → 실제 장비처럼 값이 급격히 튀지 않고 자연스럽게 변함
float update_sensor_value(float current, SensorClass* cls);

// 현재 상태(states)를 기반으로 SensorData 배열 생성
// → HSMS 전송 직전에 호출되는 핵심 함수
// 센서 주기(period_ms)를 기반으로 "지금 생성해야 할 센서만" 데이터 생성
// → 각 센서의 next_time_ms와 현재 시간(now)을 비교하여 선택적으로 생성
void generate_sensor_data_with_period(Sensor* sensors, SensorState* states, SensorData* out, int sensor_count, uint64_t now, int* out_count);



#endif 