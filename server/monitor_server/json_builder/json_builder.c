#include "json_builder.h"
#include <time.h>
#define DEBUG 0


void print_sensor_log(int m, int t, int idx,
                      float value, float avg, int anomaly)
{

    /* printf("M=%d T=%d I=%d | val=%.3f | avg=%.3f | %s\n",
           m, t, idx,
           value,
           avg,
           anomaly ? "🚨 ANOMALY" : "OK"); */
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

char* build_periodic_json()
{
    static char json[JSON_BUF_SIZE];
    size_t len = 0;
    json[0] = '\0';
    int written;

    SensorState *states;
    int state_count;
    get_all_sensor_states(&states, &state_count);

    // 1. 헤더 (현재 시간을 타임스탬프로 사용)
    written = snprintf(json + len, JSON_BUF_SIZE - len,
                       "{\"timestamp\":%llu,\"count\":%d,\"sensors\":[",
                       (unsigned long long)time(NULL) * 1000, state_count);
    len += written;

    // 2. 모든 등록된 센서 상태 순회
    for (int i = 0; i < state_count; i++) 
    {
        SensorState *s = &states[i];

        int m, t, idx;
        decode_sensor_id(s->sensor_id, &m, &t, &idx);

        if (s->has_anomaly) {
            // 이상값이 감지된 경우 실제 값(value) 포함
            written = snprintf(json + len, JSON_BUF_SIZE - len,
                "{\"id\":%u,\"m\":%d,\"t\":%d,\"i\":%d,\"avg\":%.3f,\"anomaly\":1,\"value\":%.3f}%s",
                s->sensor_id, m, t, idx, s->last_avg, s->anomaly_value,
                (i == state_count - 1) ? "" : ",");
        } else {
            // 정상인 경우 평균값(avg)만 포함
            written = snprintf(json + len, JSON_BUF_SIZE - len,
                "{\"id\":%u,\"m\":%d,\"t\":%d,\"i\":%d,\"avg\":%.3f,\"anomaly\":0}%s",
                s->sensor_id, m, t, idx, s->last_avg,
                (i == state_count - 1) ? "" : ",");
        }
        len += written;

        // 전송 후 이상값 플래그 초기화
        s->has_anomaly = 0;
    }

    // 3. 푸터 닫기
    written = snprintf(json + len, JSON_BUF_SIZE - len, "]}");
    len += written;
    json[len] = '\0';

    return json;
}