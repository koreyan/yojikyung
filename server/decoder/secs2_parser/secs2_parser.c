#include "secs2_parser.h"


/* 가변 길이 필드 파싱 */
int parse_length(uint8_t *buf, int len_bytes)
{
    int len = 0;

    for (int i = 0; i < len_bytes; i++) {
        len = (len << 8) | buf[i];
    }

    return len;
}

/* network → host float 변환 */
float ntohf(uint32_t net)
{
    uint32_t host = ntohl(net);

    float f;
    memcpy(&f, &host, sizeof(float));

    return f;
}


uint16_t parse_u2(uint8_t *buf)
{
    return ntohs(*(uint16_t*)buf);
}


uint64_t parse_u8(uint8_t *buf)
{
    uint64_t val = 0;

    for (int i = 0; i < 8; i++) {
        val = (val << 8) | buf[i];
    }

    return val;
}

float parse_f4(uint8_t *buf)
{
    uint32_t net;
    memcpy(&net, buf, 4);

    return ntohf(net);
}




/* U2 전체 item 파싱 */
int parse_u2_item(uint8_t *buf, uint16_t *out)
{
    int offset = 0;

    uint8_t format = buf[offset++];
    int len_bytes = GET_LEN_BYTES(format);

    int len = parse_length(buf + offset, len_bytes);
    offset += len_bytes;

    if (len != 2) {
        printf("U2 length error\n");
        return -1;
    }

    *out = parse_u2(buf + offset);
    offset += 2;

    return offset;
}


/* U8 전체 item 파싱 */
int parse_u8_item(uint8_t *buf, uint64_t *out)
{
    int offset = 0;

    uint8_t format = buf[offset++];
    int len_bytes = GET_LEN_BYTES(format);

    int len = parse_length(buf + offset, len_bytes);
    offset += len_bytes;

    if (len != 8) {
        printf("U8 length error\n");
        return -1;
    }

    *out = parse_u8(buf + offset);
    offset += 8;

    return offset;
}


/* F4 전체 item 파싱 */
int parse_f4_item(uint8_t *buf, float *out)
{
    int offset = 0;

    uint8_t format = buf[offset++];
    int len_bytes = GET_LEN_BYTES(format);

    int len = parse_length(buf + offset, len_bytes);
    offset += len_bytes;

    if (len != 4) {
        printf("F4 length error\n");
        return -1;
    }

    *out = parse_f4(buf + offset);
    offset += 4;

    return offset;
}


/* 하나의 센서 데이터를 파싱 */
int parse_sensor_item(uint8_t *buf, SensorData *out)
{
    int offset = 0;

    // List
    uint8_t format = buf[offset++];
    int len_bytes = GET_LEN_BYTES(format);

    int list_len = parse_length(buf + offset, len_bytes);
    offset += len_bytes;

    // sensor_id
    offset += parse_u2_item(buf + offset, &out->sensor_id);

    // value
    offset += parse_f4_item(buf + offset, &out->value);

    return offset;
}

/* 센서 리스트 전체 파싱 */
int parse_sensor_list(uint8_t *buf, PacketData *pkt)
{
    int offset = 0;

    uint8_t format = buf[offset++];
    int len_bytes = GET_LEN_BYTES(format);

    // 🔥 이건 "개수"
    int item_count = parse_length(buf + offset, len_bytes);
    offset += len_bytes;

    int count = 0;

    while (count < item_count && count < MAX_SENSORS)
    {
        int consumed = parse_sensor_item(buf + offset,
                                         &pkt->sensors[count]);

        if (consumed <= 0) {
            printf("Sensor parse error\n");
            break;
        }

        offset += consumed;
        count++;
    }

    pkt->count = count;

    return offset;
}

/* SECS-II Body 파싱 */
int parse_secs_body(uint8_t *buf, PacketData *pkt) {
    int offset = 0;

    // 1. Root List 및 Timestamp (기존 로직 동일)
    offset += 1; // List Format
    offset += GET_LEN_BYTES(buf[offset-1]); 
    offset += parse_u8_item(buf + offset, &pkt->timestamp);

    // 2. [중요] 센서 개수 추출
    uint16_t sensor_count;
    offset += parse_u2_item(buf + offset, &sensor_count);

    // 3. Sensor List Header Skip
    offset += 1; 
    offset += GET_LEN_BYTES(buf[offset-1]);

    // 4. 장비가 보낸 실제 개수만큼만 반복 파싱
    pkt->count = 0;
    while (pkt->count < sensor_count && pkt->count < MAX_SENSORS) {
        int consumed = parse_sensor_item(buf + offset, &pkt->sensors[pkt->count]);
        if (consumed <= 0) break;
        offset += consumed;
        pkt->count++;
    }

    return offset;
}