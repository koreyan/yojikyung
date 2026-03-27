#include "run_equipment.h"
#include "sensor_id.h"


//==========================================================디버깅용=======================================================
// sensor_id → module / type / index 분리
void decode_sensor_id(uint16_t id,
                      uint8_t* module,
                      uint8_t* type,
                      uint8_t* index)
{
    *module = (id >> 12) & 0x0F;
    *type   = (id >> 8)  & 0x0F;
    *index  = id & 0xFF;
}


// Module 이름 반환
const char* module_to_str(uint8_t m)
{
    switch (m) {
        case MODULE_BOND_HEAD: return "BOND_HEAD";
        case MODULE_STAGE: return "STAGE";
        case MODULE_HEATER: return "HEATER";
        case MODULE_VACUUM: return "VACUUM";
        case MODULE_MOTOR_DRIVE: return "MOTOR";
        case MODULE_VISION: return "VISION";
        case MODULE_ENVIRONMENT: return "ENV";
        case MODULE_POWER: return "POWER";
        default: return "UNKNOWN";
    }
}

// SensorType 이름 반환
const char* type_to_str(uint8_t t)
{
    switch (t) {
        case SENSOR_TEMPERATURE: return "TEMP";
        case SENSOR_PRESSURE: return "PRESS";
        case SENSOR_FORCE: return "FORCE";
        case SENSOR_ULTRASONIC_POWER: return "US_POWER";
        case SENSOR_POSITION_ENCODER: return "POS";
        case SENSOR_MOTOR_CURRENT: return "CURR";
        case SENSOR_MOTOR_VOLTAGE: return "VOLT";
        case SENSOR_MOTOR_SPEED: return "SPEED";
        case SENSOR_VIBRATION: return "VIB";
        case SENSOR_VACUUM_PRESSURE: return "VAC";
        case SENSOR_FLOW: return "FLOW";
        case SENSOR_VISION_ALIGNMENT: return "ALIGN";
        case SENSOR_DEFECT_DETECTION: return "DEFECT";
        case SENSOR_HUMIDITY: return "HUM";
        case SENSOR_AIRFLOW: return "AIR";
        case SENSOR_POWER_CONSUMPTION: return "POWER";
        default: return "UNKNOWN";
    }
}


// 전송 직전에 센서 데이터 출력
void debug_print_packet(uint64_t timestamp,
                        SensorData* data,
                        int count)
{
    printf("\n========== PACKET ==========\n");
    printf("Time: %llu ms\n", (unsigned long long)timestamp);
    printf("Sensor Count: %d\n", count);

    for (int i = 0; i < count; i++) {

        uint8_t m, t, idx;

        decode_sensor_id(data[i].sensor_id, &m, &t, &idx);

        printf(" [%3d] ID=0x%04X (%s / %s / %d)  Value=%.3f\n",
               i,
               data[i].sensor_id,
               module_to_str(m),
               type_to_str(t),
               idx,
               data[i].value);
    }

    printf("============================\n");
}
//========================================================================================================================







int run(){
    // 소켓 연결
    int sock = connect_server();

    // 센서 초기화 
    Sensor sensors[SENSOR_COUNT];
    SensorState states[SENSOR_COUNT];
    SensorData data[SENSOR_COUNT];


    // 센서 인스턴스 생성
    init_sensors(sensors);


    // 초기 상태 설정 (정상 범위 내)
    init_sensor_states(states, sensors, SENSOR_COUNT);


    //------------------------------------------------------------
    // 3. 메인 루프 (시뮬레이터)
    //------------------------------------------------------------
    while (1){
        uint64_t now = get_time_ms();

       int count = 0;

        // 센서 주기 기반으로 필요한 데이터만 생성
        generate_sensor_data_with_period(sensors, states, data, SENSOR_COUNT, now, &count);

        // 생성된 데이터가 있을 때만 전송
        if (count > 0) {

            // 🔥 디버깅 출력
            debug_print_packet(now, data, count);
            send_sensor_packet(sock, now, data, count);
        }



        // 로그 (디버깅용)
            // printf("Sent packet - time=%llu, sensors=%d\n",
            //     (unsigned long long)now,
            //     SENSOR_COUNT);

            // 
            usleep(1000000);
    }

    close(sock);
    
    return -1;
}