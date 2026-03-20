#include "./network/network.h"
#include "./packet/packet.h"
#include "./sensor/sensor.h"
#include "./util/util.h"
// #include "./debug/debug.h"

#define SENSOR_COUNT 128        // 총 센서 수
#define MAX_PACKET_SENSORS 128  // 한 패킷에 담을 수 있는 최대 센서 수
#define MICRO_SEC 100000

int main(){

    // 서버 연결
    int sock = connect_server();

    // 데이터 생성 및 초기화
    Sensor sensors[SENSOR_COUNT];
    SensorData packet_data[SENSOR_COUNT];
    int count = init_sensors(sensors);
    // validate_sensors(sensors, count);


    while(1)
    {
        uint64_t now = now_ms();

        // 1. 센서 데이터 생성
        int count = generate_sensor_packet(
            sensors,
            SENSOR_COUNT,
            packet_data,
            MAX_PACKET_SENSORS,
            now
        );

        if(count == 0)
        {
            usleep(MICRO_SEC);
            continue;
        }

        // 2. 패킷 생성
        uint32_t size = build_packet(
            packet_buffer,
            packet_data,
            count,
            now
        );

        // 3. 전송
        send_packet(sock, packet_buffer, size);

        usleep(MICRO_SEC);
    }
}
