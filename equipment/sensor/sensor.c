#include "sensor.h"
#include "util.h"

extern SensorClass sensor_classes[];
extern const int SENSOR_CLASS_COUNT;


// class_id로 SensorClass 검색
SensorClass* find_sensor_class(uint8_t class_id)
{
    for(int i = 0; i < SENSOR_CLASS_COUNT; i++)
    {
        if(sensor_classes[i].class_id == class_id)
            return &sensor_classes[i];
    }
    return NULL;  // 없으면 NULL 반환
}


static void add_sensor(Sensor sensors[], int *i, Module module, SensorType type, uint8_t index, int period)
{
    sensors[*i].sensor_id = MAKE_SENSOR_ID(module,type,index);
    sensors[*i].period_ms = period;
    sensors[*i].next_time_ms = now_ms() + period;
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

// 센서 생성 값 생성 함수 
float generate_sensor_value(uint16_t sensor_id)
{
    // 센서 아이디로 class id 가저옴 
    uint8_t class_id = GET_CLASS_ID(sensor_id);

    // class_id로 정상값 범주가 저장된 센서 클래스 인스턴스 가져옴
    SensorClass* cls = find_sensor_class(class_id);

    if(cls == NULL)
        return 0.0f;

    // 센서 타입에 맞는 정상값들을 넣어줌
    float range = cls->normal_max - cls->normal_min;
    float r = (float)rand() / RAND_MAX; // RAND_MAX rand가 될 수 있는 최대값 => 최종적으로 0~1사이 

    float value = cls->normal_min + r * range;

    // 5% 확률로 이상값 발생
    if(rand() % 100 < 5)
        value = cls->normal_max + ((float)(rand() % 20));

    return value;
}

// 패킷에 붙일 센서 데이터 값 생성하는 함수 
// 센서 데이터 생성 함수 (시간 외부 주입)
int generate_sensor_packet(Sensor sensors[], int sensor_count,
                           SensorData packet_data[], int max_count,
                           uint64_t now)
{
    int count = 0;

    for(int i = 0; i < sensor_count; i++)
    {
        // 주기 도달 여부 확인
        if(now >= sensors[i].next_time_ms)
        {
            // SensorData 생성
            packet_data[count].sensor_id = sensors[i].sensor_id;
            packet_data[count].value = generate_sensor_value(sensors[i].sensor_id);
            
            count++;

            // 다음 주기 업데이트
            sensors[i].next_time_ms = now + sensors[i].period_ms;

            if(count >= max_count)
                break;
        }
    }

    return count;
}