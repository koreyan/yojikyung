#include "json_builder.h"


// =========================
// JSON 생성
// =========================
char* build_json(PacketData *pkt)
{
    static char json[JSON_BUF_SIZE];
    size_t len = 0;
    json[0] = '\0';

    int written;

    // =========================
    // 헤더
    // =========================
    written = snprintf(json + len, JSON_BUF_SIZE - len,
                       "{\"timestamp\":%llu,\"count\":%d,\"sensors\":[",
                       pkt->timestamp, MAX_SENSORS);

    if (written < 0) return NULL;
    if (written >= JSON_BUF_SIZE - len) {
        printf("❌ JSON buffer overflow (header)\n");
        return NULL;
    }
    len += written;

    // =========================
    // 센서 데이터
    // =========================
    for (int i = 0; i < MAX_SENSORS; i++)
    {
        SensorData s;
        if (i < pkt->count)
            s = pkt->sensors[i];
        else
            memset(&s, 0, sizeof(SensorData));

        int m, t, idx;
        decode_sensor_id(s.sensor_id, &m, &t, &idx);

        SensorState *state = get_sensor_state(s.sensor_id);
        float avg = update_moving_average(state, s.value);
        int anomaly = detect_anomaly(s.sensor_id, s.value);

        written = snprintf(
            json + len,
            JSON_BUF_SIZE - len,
            "{\"id\":%u,\"m\":%d,\"t\":%d,\"i\":%d,"
            "\"value\":%.3f,\"avg\":%.3f,\"anomaly\":%d}%s",
            s.sensor_id,
            m,
            t,
            idx,
            s.value,
            avg,
            anomaly,
            ((i != MAX_SENSORS - 1) ? "," : "")
        );

        if (written < 0) return NULL;
        if (written >= JSON_BUF_SIZE - len) {
            printf("❌ JSON buffer overflow (sensor %d)\n", i);
            return NULL;
        }

        len += written;
    }

    // =========================
    // footer
    // =========================
    written = snprintf(json + len, JSON_BUF_SIZE - len, "]}");

    if (written < 0) return NULL;
    if (written >= JSON_BUF_SIZE - len) {
        printf("❌ JSON buffer overflow (footer)\n");
        return NULL;
    }

    len += written;

    // 문자열 종료
    json[len] = '\0';

    return json;
}