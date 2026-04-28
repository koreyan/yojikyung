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
// --- 데이터 송신 전 디버깅 출력 함수 ---
// 이 함수는 뮤텍스 내부에서 호출되어야 출력 결과가 스레드 간에 섞이지 않습니다.
void debug_print_packet(uint64_t timestamp, SensorData* data, int count) {
    printf("\n[TX PACKET] ------------------------------------------\n");
    printf(" Timestamp  : %llu ms\n", (unsigned long long)timestamp);
    printf(" Data Count : %d\n", count);
    printf(" -----------------------------------------------------\n");

    for (int i = 0; i < count; i++) {
        uint8_t m, t, idx;
        // 기존 정의된 유틸리티를 사용하여 ID 해석
        decode_sensor_id(data[i].sensor_id, &m, &t, &idx);

        // [형식] 인덱스: ID (모듈/타입/번호) -> 현재 측정값
        printf(" [%3d] 0x%04X (Mod:%d, Type:%d, Idx:%d) -> Value: %.3f\n",
               i, data[i].sensor_id, m, t, idx, data[i].value);
    }
    printf("------------------------------------------------------\n");
}
//========================================================================================================================

/**
 * [PRD 요구사항 반영] 
 * 1. 128개 센서 데이터를 8개 모듈 스레드로 분산 처리
 * 2. 공유 소켓 전송 시 Mutex를 사용하여 데이터 깨짐 방지
 */

#define MAX_SENSORS 128        // 시스템 내 최대 센서 수
#define MODULE_COUNT 8         // 0x0 ~ 0x7 모듈 개수

// --- 전역 자원 ---
pthread_mutex_t socket_mutex = PTHREAD_MUTEX_INITIALIZER; // 소켓 전송 동기화를 위한 뮤텍스
int global_sock = -1;                                     // 서버와 연결된 전역 소켓 핸들











// --- 스레드 전달용 구조체 ---
typedef struct {
    Module module_id;          // 해당 스레드가 담당할 모듈 (0~7)
    Sensor* sensors;           // 전체 센서 설정 배열 (주기 등 정보)
    SensorState* states;       // 전체 센서 상태 배열 (현재 값 정보)
    int total_sensor_count;    // 초기화된 총 센서 개수
} ModuleThreadArgs;

/**
 * @brief 모듈별 독립 스레드 루틴
 * 각 모듈 스레드는 전체 센서 리스트를 순회하며 자기 담당 센서의 데이터만 생성/전송합니다.
 */
void* module_worker(void* arg) {
    ModuleThreadArgs* args = (ModuleThreadArgs*)arg;
    SensorData out_data[MAX_SENSORS]; // 주기가 도래한 데이터를 임시 저장할 배열
    int out_count = 0;

    printf("[Module Thread 0x%X] 독립 실행 시작\n", args->module_id);

    while (1) {
        uint64_t now = get_time_ms();
        out_count = 0;

        // 1. 내 담당 모듈의 센서 데이터 갱신 로직
        for (int i = 0; i < args->total_sensor_count; i++) {
            uint8_t m, t, idx;
            // Sensor ID에서 모듈 정보를 추출하여 내 스레드 담당인지 확인
            decode_sensor_id(args->sensors[i].sensor_id, &m, &t, &idx);

            if (m == args->module_id) {
                // 데이터 생성 주기가 되었는지 확인
                if (now >= args->sensors[i].next_time_ms) {
                    SensorClass* cls = find_sensor_class(args->states[i].sensor_id);
                    if (cls) {
                        // 센서 값 업데이트 및 전송 목록 추가
                        args->states[i].value = update_sensor_value(args->states[i].value, cls);
                        out_data[out_count].sensor_id = args->states[i].sensor_id;
                        out_data[out_count].value = args->states[i].value;
                        out_count++;
                        
                        // 다음 실행 시간 갱신
                        args->sensors[i].next_time_ms = now + args->sensors[i].period_ms;
                    }
                }
            }
        }

        // 2. 주기가 된 센서가 하나라도 있다면 서버로 패킷 전송
        if (out_count > 0) {
            // [Critical Section] 여러 스레드가 동시에 소켓을 사용하지 못하도록 잠금
            pthread_mutex_lock(&socket_mutex);
            
            // 데이터 전송 전 디버깅 정보 출력
            debug_print_packet(now, out_data, out_count);
            // 실시간 고속 전송 수행 (packet.c 내부 로직 호출)
            send_sensor_packet(global_sock, now, out_data, out_count);
            
            pthread_mutex_unlock(&socket_mutex); // 전송 후 즉시 잠금 해제
        }

        // 3. 미세 대기 (1ms): CPU 점유율 과부하 방지 및 실시간성 유지
        usleep(1000);
    }
    return NULL;
}

/**
 * @brief 장비 시뮬레이터 실행 메인 함수
 */
void run() {
    Sensor sensors[MAX_SENSORS];
    SensorState states[MAX_SENSORS];
    
    // 1. 센서 초기 설정 및 상태 메모리 초기화
    int count = init_sensors(sensors);
    init_sensor_states(states, sensors, count);

    // 2. 네트워크 서버 접속
    global_sock = connect_server();
    if (global_sock < 0) {
        fprintf(stderr, "서버 연결에 실패했습니다. 프로그램을 종료합니다.\n");
        return;
    }

    // 3. 모듈별 스레드 생성 (8개 모듈 독립 구동)
    pthread_t threads[MODULE_COUNT];
    ModuleThreadArgs thread_args[MODULE_COUNT];

    for (int i = 0; i < MODULE_COUNT; i++) {
        thread_args[i].module_id = (Module)i;
        thread_args[i].sensors = sensors;
        thread_args[i].states = states;
        thread_args[i].total_sensor_count = count;
        
        // 스레드 생성: 실패 시 에러 출력
        if (pthread_create(&threads[i], NULL, module_worker, &thread_args[i]) != 0) {
            perror("스레드 생성 실패");
        }
    }

    // 4. 스레드 종료 대기 (실제 시뮬레이터는 중단 없이 동작)
    for (int i = 0; i < MODULE_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    // 5. 자원 정리
    close(global_sock);
    pthread_mutex_destroy(&socket_mutex);
}