#include "data_generator.h"


extern SensorClass sensor_classes[];
extern const int SENSOR_CLASS_COUNT;



static void add_sensor(Sensor sensors[], int *i, Module module, SensorType type, uint8_t index, int period)
{
    sensors[*i].sensor_id = MAKE_SENSOR_ID(module,type,index);
    sensors[*i].period_ms = period;
    // 센서 생성 시 초기 타이밍을 랜덤하게 분산
    sensors[*i].next_time_ms = get_time_ms() + (rand() % period);
    (*i)++;
}

// 센서 초기화 
// 센서 초기화 (총 128개)
int init_sensors(Sensor sensors[])
{
    int i = 0;

    //---------------- Bond Head ----------------
    for(int k=0;k<6;k++) add_sensor(sensors,&i,MODULE_BOND_HEAD,SENSOR_TEMPERATURE,k,100);
    for(int k=0;k<6;k++) add_sensor(sensors,&i,MODULE_BOND_HEAD,SENSOR_FORCE,k,50);
    for(int k=0;k<4;k++) add_sensor(sensors,&i,MODULE_BOND_HEAD,SENSOR_ULTRASONIC_POWER,k,50);
    for(int k=0;k<4;k++) add_sensor(sensors,&i,MODULE_BOND_HEAD,SENSOR_VIBRATION,k,200);

    //---------------- Stage ----------------
    for(int k=0;k<14;k++) add_sensor(sensors,&i,MODULE_STAGE,SENSOR_POSITION_ENCODER,k,20);
    for(int k=0;k<6;k++) add_sensor(sensors,&i,MODULE_STAGE,SENSOR_MOTOR_CURRENT,k,50);
    for(int k=0;k<4;k++) add_sensor(sensors,&i,MODULE_STAGE,SENSOR_MOTOR_VOLTAGE,k,50);
    for(int k=0;k<4;k++) add_sensor(sensors,&i,MODULE_STAGE,SENSOR_MOTOR_SPEED,k,20);
    for(int k=0;k<4;k++) add_sensor(sensors,&i,MODULE_STAGE,SENSOR_VIBRATION,k,200);

    //---------------- Heater ----------------
    for(int k=0;k<8;k++) add_sensor(sensors,&i,MODULE_HEATER,SENSOR_TEMPERATURE,k,200);
    for(int k=0;k<4;k++) add_sensor(sensors,&i,MODULE_HEATER,SENSOR_POWER_CONSUMPTION,k,50);

    //---------------- Vacuum ----------------
    for(int k=0;k<8;k++) add_sensor(sensors,&i,MODULE_VACUUM,SENSOR_VACUUM_PRESSURE,k,100); 
    for(int k=0;k<4;k++) add_sensor(sensors,&i,MODULE_VACUUM,SENSOR_FLOW,k,200);

    //---------------- Motor / Drive ----------------
    for(int k=0;k<10;k++) add_sensor(sensors,&i,MODULE_MOTOR_DRIVE,SENSOR_MOTOR_CURRENT,k,50); 
    for(int k=0;k<6;k++) add_sensor(sensors,&i,MODULE_MOTOR_DRIVE,SENSOR_MOTOR_VOLTAGE,k,50);
    for(int k=0;k<6;k++) add_sensor(sensors,&i,MODULE_MOTOR_DRIVE,SENSOR_MOTOR_SPEED,k,20);
    for(int k=0;k<4;k++) add_sensor(sensors,&i,MODULE_MOTOR_DRIVE,SENSOR_VIBRATION,k,100);

    //---------------- Vision ----------------
    for(int k=0;k<5;k++) add_sensor(sensors,&i,MODULE_VISION,SENSOR_VISION_ALIGNMENT,k,100);
    for(int k=0;k<5;k++) add_sensor(sensors,&i,MODULE_VISION,SENSOR_DEFECT_DETECTION,k,200);

    //---------------- Environment ----------------
    for(int k=0;k<5;k++) add_sensor(sensors,&i,MODULE_ENVIRONMENT,SENSOR_TEMPERATURE,k,1000); 
    for(int k=0;k<2;k++) add_sensor(sensors,&i,MODULE_ENVIRONMENT,SENSOR_HUMIDITY,k,1000);
    for(int k=0;k<3;k++) add_sensor(sensors,&i,MODULE_ENVIRONMENT,SENSOR_AIRFLOW,k,1000);

    //---------------- Power ----------------
    for(int k=0;k<6;k++) add_sensor(sensors,&i,MODULE_POWER,SENSOR_POWER_CONSUMPTION,k,500);

    return i;
}




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

// 센서 초기값을 정상 범위 내에서 랜덤으로 설정
// → 시작부터 자연스러운 값 분포 생성
void init_sensor_states(SensorState* states, Sensor* sensors, int count)
{
    for (int i = 0; i < count; i++) {

        SensorClass* cls = find_sensor_class(sensors[i].sensor_id);

        states[i].sensor_id = sensors[i].sensor_id;

        if (cls) {
            // 정상 범위 내 랜덤 초기값
            states[i].value = rand_float(cls->normal_min, cls->normal_max);
        } else {
            states[i].value = 0.0f; // fallback
        }
    }
}



// 센서 값을 "이전 값 기반으로" 업데이트
// → 실제 장비처럼 값이 급격히 튀지 않고 자연스럽게 변함
float update_sensor_value(float current, SensorClass* cls)
{
    float r = (float)rand() / RAND_MAX;

    // 1. 매우 낮은 확률로 이상 값 생성
    if (r < ABNORMAL_PROB) {

        float range = cls->normal_max - cls->normal_min;

        // 정상 범위를 벗어나도록 크게 튀게 만듦
        if (rand() % 2) {
            // 상한 초과
            return cls->normal_max + range * rand_float(0.5f, 2.0f);
        } else {
            // 하한 미만
            return cls->normal_min - range * rand_float(0.5f, 2.0f);
        }
    }

    // 2. 정상 상태 → 미세한 변화
    float range = cls->normal_max - cls->normal_min;

    // 변화량 (범위 대비 1%)
    float delta = range * DELTA_RATIO;

    // -delta ~ +delta 범위의 랜덤 변화
    float change = rand_float(-delta, delta);

    float next = current + change;

    // 3. 정상 범위를 벗어나지 않도록 보정
    return clamp(next, cls->normal_min, cls->normal_max);
}



// 현재 상태(states)를 기반으로 SensorData 배열 생성
// → HSMS 전송 직전에 호출되는 핵심 함수
// 센서 주기(period_ms)를 기반으로 "지금 생성해야 할 센서만" 데이터 생성
// → 각 센서의 next_time_ms와 현재 시간(now)을 비교하여 선택적으로 생성
void generate_sensor_data_with_period(Sensor* sensors, SensorState* states, SensorData* out, int sensor_count, uint64_t now, int* out_count)
{
    int idx = 0; // 이번 패킷에 담을 데이터 개수

    for (int i = 0; i < sensor_count; i++)
    {
        // 아직 생성할 시간이 안 된 센서는 skip
        if (now < sensors[i].next_time_ms)
            continue;

        // 센서 클래스 (정상 범위 정보) 찾기
        SensorClass* cls = find_sensor_class(states[i].sensor_id);
        if (!cls)
            continue;

        // 현재 값을 기반으로 다음 값 생성 (미세 변화 + 이상치 포함)
        states[i].value = update_sensor_value(states[i].value, cls);

        // 전송용 데이터에 기록
        out[idx].sensor_id = states[i].sensor_id;
        out[idx].value     = states[i].value;

        idx++;

        // 다음 생성 시각 업데이트
        // → 현재 시간 기준으로 period를 더함
        sensors[i].next_time_ms = now + sensors[i].period_ms;
    }

    // 실제 생성된 센서 개수 반환
    *out_count = idx;
}