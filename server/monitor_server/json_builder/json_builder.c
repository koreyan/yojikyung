#include "json_builder.h"
#define DEBUG 0


void print_sensor_log(int m, int t, int idx,
                      float value, float avg, int anomaly)
{

    printf("M=%d T=%d I=%d | val=%.3f | avg=%.3f | %s\n",
           m, t, idx,
           value,
           avg,
           anomaly ? "🚨 ANOMALY" : "OK");
}


// =========================
// JSON 생성
// =========================
char* build_json(PacketData *pkt)
{
    static char json[JSON_BUF_SIZE];
    size_t len = 0;
    json[0] = '\0';
    int written;

    // 1. 헤더 부분: count를 MAX_SENSORS가 아닌 pkt->count로 기록
    written = snprintf(json + len, JSON_BUF_SIZE - len,
                       "{\"timestamp\":%llu,\"count\":%d,\"sensors\":[",
                       pkt->timestamp, pkt->count);
    len += written;

    // 2. [수정 핵심] MAX_SENSORS(128) 대신 실제 개수인 pkt->count만큼만 루프
    for (int i = 0; i < pkt->count; i++) 
    {
        SensorData s = pkt->sensors[i];

        int m, t, idx;
        decode_sensor_id(s.sensor_id, &m, &t, &idx);

        SensorState *state = get_sensor_state(s.sensor_id);
        float avg = update_moving_average(state, s.value);
        int anomaly = detect_anomaly(s.sensor_id, s.value);

        // DEBUG 모드일 때 콘솔 출력 (여기서도 들어온 것만 찍히게 됨)
        if (DEBUG){
            print_sensor_log(m, t, idx, s.value, avg, anomaly);
        }

        written = snprintf(
            json + len,
            JSON_BUF_SIZE - len,
            "{\"id\":%u,\"m\":%d,\"t\":%d,\"i\":%d,"
            "\"value\":%.3f,\"avg\":%.3f,\"anomaly\":%d}%s",
            s.sensor_id, m, t, idx, s.value, avg, anomaly,
            ((i != pkt->count - 1) ? "," : "") // 마지막 요소가 아니면 콤마 추가
        );
        len += written;
    }

    // 3. 푸터 닫기
    written = snprintf(json + len, JSON_BUF_SIZE - len, "]}");
    len += written;
    json[len] = '\0';

    return json;
}