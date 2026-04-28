#include "history_storage.h"

static HistoryStorage g_storage;

void init_storage() {
    g_storage.history = (PacketData*)malloc(sizeof(PacketData) * MAX_HISTORY);
    if (!g_storage.history) {
        perror("Memory allocation failed for storage");
        return;
    }
    memset(g_storage.history, 0, sizeof(PacketData) * MAX_HISTORY);
    g_storage.head = g_storage.tail = g_storage.count = 0;
    printf("📦 RAM Storage: Ready for 10min history\n");
}

void save_to_storage(PacketData *pkt) {
    if (!g_storage.history) return;

    memcpy(&g_storage.history[g_storage.tail], pkt, sizeof(PacketData));
    uint64_t now = pkt->timestamp;

    g_storage.tail = (g_storage.tail + 1) % MAX_HISTORY;
    if (g_storage.count < MAX_HISTORY) g_storage.count++;
    else g_storage.head = (g_storage.head + 1) % MAX_HISTORY;

    // 시간 기준 FIFO 삭제
    while (g_storage.count > 0) {
        if (now - g_storage.history[g_storage.head].timestamp > RETENTION_MS) {
            g_storage.head = (g_storage.head + 1) % MAX_HISTORY;
            g_storage.count--;
        } else break;
    }
}

void cleanup_storage() {
    if (g_storage.history) free(g_storage.history);
}