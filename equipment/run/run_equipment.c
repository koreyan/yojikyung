#include "run_equipment.h"

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
            send_sensor_packet(sock, now, data, count);
        }

        // 로그 (디버깅용)
            printf("Sent packet - time=%llu, sensors=%d\n",
                (unsigned long long)now,
                SENSOR_COUNT);

            // 100ms 주기
            usleep(1000);
    }

    close(sock);
    
    return -1;
}