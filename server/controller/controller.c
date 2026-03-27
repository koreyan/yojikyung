#include "controller.h"


#define DEBUG_HEX 1

//============================================ 디 버 깅 ==================================================//
void print_hex(uint8_t *data, int len)
{
    for (int i = 0; i < len; i++)
    {
        printf("%02X ", data[i]);

        if ((i + 1) % 16 == 0)
            printf("\n");
    }
    printf("\n");
}

/* sensor_id → module/type/index 분해 */
void decode_sensor_id(uint16_t id,
                      int *module,
                      int *type,
                      int *index)
{
    *module = (id >> 12) & 0x0F;
    *type   = (id >> 8)  & 0x0F;
    *index  = id & 0xFF;
}

/* Module 이름 */
const char* get_module_name(int m)
{
    switch(m)
    {
        case 0: return "BondHead";
        case 1: return "Stage";
        case 2: return "Heater";
        case 3: return "Vacuum";
        case 4: return "Motor";
        case 5: return "Vision";
        case 6: return "Env";
        case 7: return "Power";
        default: return "Unknown";
    }
}

/* Type 이름 */
const char* get_type_name(int t)
{
    static const char* names[] = {
        "Temperature",        // 0
        "Pressure",           // 1
        "Force",              // 2
        "UltrasonicPower",    // 3
        "PositionEncoder",    // 4
        "MotorCurrent",       // 5
        "MotorVoltage",       // 6
        "MotorSpeed",         // 7
        "Vibration",          // 8
        "VacuumPressure",     // 9
        "Flow",               // 10
        "VisionAlignment",    // 11
        "DefectDetection",    // 12
        "Humidity",           // 13
        "Airflow",            // 14
        "PowerConsumption"    // 15
    };

    if (t >= 0 && t < 16)
        return names[t];

    return "Unknown";
}

/* 단위 */
const char* get_unit(int m, int t)
{
    if (m == 0 && t == 0) return "°C";
    if (m == 0 && t == 2) return "N";
    if (m == 1 && t == 5) return "A";
    if (m == 3 && t == 9) return "kPa";

    return "";
}

/* 센서 하나 출력 */
void print_sensor(SensorData *s, int idx)
{
    int m, t, i;
    decode_sensor_id(s->sensor_id, &m, &t, &i);

    printf("[%02d] M=%d T=%d I=%d  →  %.3f\n",
           idx, m, t, i, s->value);

    printf("      %s / %s\n",
           get_module_name(m),
           get_type_name(t));
}
//==============================================================================================//



/* =========================
   패킷 처리 (Hook)
========================= */
void handle_packet(uint8_t *packet, int len)
{
    // 1️⃣ RAW 확인
    if (DEBUG_HEX)
        print_hex(packet, len);

    HSMSHeader h;

    // 2️⃣ HSMS 파싱
    parse_hsms_header(packet, &h);

    // printf("\n===== HSMS HEADER =====\n");
    // printf("Length: %u\n", h.length);
    // printf("Stream: %d, Function: %d\n", h.stream, h.function);

    // 3️⃣ sanity check
    if (h.stream != 6 || h.function != 1)
    {
        printf("⚠️ Skip non S6F1 message\n");
        return;
    }

    // 4️⃣ SECS 파싱
    PacketData pkt;
    decode_packet(packet, len, &pkt);

    printf("\n============================\n");
    printf("[Packet]\n");
    printf("Timestamp: %llu\n", pkt.timestamp);
    printf("Sensor Count: %d\n\n", pkt.count);

    for (int i = 0; i < pkt.count; i++)
    {
        print_sensor(&pkt.sensors[i], i);
    }
    
    // printf("===== SECS DATA =====\n");
    // printf("Timestamp: %llu\n", pkt.timestamp);
    // printf("Sensor Count: %d\n", pkt.count);

    // for (int i = 0; i < pkt.count; i++)
    // {
    //     printf("[%02d] ID=0x%04X Value=%.3f\n",
    //            i,
    //            pkt.sensors[i].sensor_id,
    //            pkt.sensors[i].value);
    // }
}

/* =========================
   버퍼 처리
========================= */

void process_buffer(RecvBuffer *rb)
{
    uint8_t packet[BUFFER_SIZE];
    int packet_len;

    while (try_extract_packet(rb, packet, &packet_len))
    {
        handle_packet(packet, packet_len);
    }
}