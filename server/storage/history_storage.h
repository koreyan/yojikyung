#ifndef HISTORY_STORAGE_H
#define HISTORY_STORAGE_H

#include "../common/common.h"
#include "../type/packet_data.h"

// 1ms 주기, 10분 데이터 = 600,000개. RAM 약 480MB 사용
#define MAX_HISTORY 650000 
#define RETENTION_MS (10 * 60 * 1000)

typedef struct {
    PacketData *history; 
    int head;
    int tail;
    int count;
} HistoryStorage;

void init_storage();
void save_to_storage(PacketData *pkt);
void cleanup_storage();

#endif