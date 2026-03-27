#include "controller.h"


#define DEBUG 1

/* =========================
   패킷 처리 (Hook)
========================= */
void handle_packet(uint8_t *packet, int len)
{
    // 1️⃣ RAW 확인
    // if (DEBUG){
    //     print_hex(packet, len);
    // }

    HSMSHeader h;

    // 2️⃣ HSMS 파싱
    parse_hsms_header(packet, &h);

    if(DEBUG){
        printf("\n===== HSMS HEADER =====\n");
        printf("Length: %u\n", h.length);
        printf("Stream: %d, Function: %d\n", h.stream, h.function);
    }
    // 3️⃣ sanity check
    if (h.stream != 6 || h.function != 1)
    {
        printf("⚠️ Skip non S6F1 message\n");
        return;
    }

    // 4️⃣ SECS 파싱
    PacketData pkt;
    decode_packet(packet, len, &pkt);

    if (DEBUG){
        printf("\n============================\n");
        printf("[Packet]\n");
        printf("Timestamp: %llu\n", pkt.timestamp);
        printf("Sensor Count: %d\n\n", pkt.count);

        for (int i = 0; i < pkt.count; i++)
        {
            print_sensor(&pkt.sensors[i], i);
        }
    }
   

    for (int i = 0; i < pkt.count; i++)
    {
        process_sensor(&pkt.sensors[i]);
    }


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