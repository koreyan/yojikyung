#include "file_logger.h"
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

static char g_log_filename[PATH_MAX];
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
static int g_buffer_idx = 0;
static PacketData g_temp_buffer[10000];
static long g_last_flush_time = 0;

#define RETENTION_SEC 600
#define MAX_LOG_SIZE (1024 * 1024 * 1024) // 1GB 제한

// ID 기반 단위 반환 함수 (업로드해주신 센서 체계 반영)
const char* get_unit_by_id(uint16_t id) {
    int m = (id >> 12) & 0x0F; 
    int t = (id >> 8) & 0x0F;  

    switch (m) {
        case 0: // Bond Head
            if (t == 0) return "°C"; if (t == 2) return "N";
            if (t == 3) return "W"; if (t == 8) return "g";
            break;
        case 1: // Stage
            if (t == 4) return "mm"; if (t == 5) return "A";
            if (t == 6) return "V"; if (t == 7) return "mm/s";
            if (t == 8) return "g";
            break;
        case 2: // Heater
            if (t == 0) return "°C"; if (t == 15) return "W";
            break;
        case 3: // Vacuum
            if (t == 9) return "kPa"; if (t == 10) return "L/min";
            break;
        case 4: // Motor/Drive
            if (t == 5) return "A"; if (t == 6) return "V";
            if (t == 7) return "rpm"; if (t == 8) return "g";
            break;
        case 5: // Vision
            if (t == 11) return "μm"; if (t == 12) return "-";
            break;
        case 6: // Environment
            if (t == 0) return "°C"; if (t == 13) return "%RH";
            if (t == 14) return "m/s";
            break;
        case 7: // Power
            if (t == 15) return "kW";
            break;
    }
    return "";
}

static long get_now_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static long get_file_size(const char *filename) {
    struct stat st;
    if (stat(filename, &st) == 0) return st.st_size;
    return 0;
}

void init_file_logger(const char* filename) {
    pthread_mutex_lock(&log_mutex);
    
    char base_path[PATH_MAX];
#ifdef __APPLE__
    uint32_t size = sizeof(base_path);
    if (_NSGetExecutablePath(base_path, &size) == 0) {
        char *last_slash = strrchr(base_path, '/');
        if (last_slash) *last_slash = '\0';
    } else {
        getcwd(base_path, sizeof(base_path));
    }
#else
    ssize_t len = readlink("/proc/self/exe", base_path, sizeof(base_path) - 1);
    if (len != -1) {
        base_path[len] = '\0';
        char *last_slash = strrchr(base_path, '/');
        if (last_slash) *last_slash = '\0';
    } else {
        getcwd(base_path, sizeof(base_path));
    }
#endif

    snprintf(g_log_filename, sizeof(g_log_filename), "%s/%s", base_path, filename);

    FILE *f = fopen(g_log_filename, "a");
    if (f) fclose(f);
    
    g_last_flush_time = get_now_ms();
    printf("📍 [Logger Target Path] %s\n", g_log_filename);
    pthread_mutex_unlock(&log_mutex);
}

void flush_log_to_disk() {
    pthread_mutex_lock(&log_mutex);
    if (g_buffer_idx == 0) {
        pthread_mutex_unlock(&log_mutex);
        return;
    }

    const char* mode = "a";
    if (get_file_size(g_log_filename) >= MAX_LOG_SIZE) {
        mode = "w"; // 1GB 초과 시 초기화 (FIFO)
        printf("⚠️ [WARNING] Log size exceeded 1GB. Resetting file.\n");
    }

    FILE *f = fopen(g_log_filename, mode);
    if (!f) {
        pthread_mutex_unlock(&log_mutex);
        return;
    }

    uint64_t now_ms = (uint64_t)get_now_ms();
    uint64_t cutoff = now_ms - (RETENTION_SEC * 1000);

    for (int i = 0; i < g_buffer_idx; i++) {
        if (g_temp_buffer[i].timestamp < cutoff) continue;

        // 요구하신 포맷: [시간] ID:값:단위 (가독성을 위한 개행 추가)
        fprintf(f, "------------------------------------------\n");
        fprintf(f, "TIME : %llu\n", (unsigned long long)g_temp_buffer[i].timestamp);
        
        for (int j = 0; j < g_temp_buffer[i].count; j++) {
            uint16_t id = g_temp_buffer[i].sensors[j].sensor_id;
            float val = g_temp_buffer[i].sensors[j].value;
            const char* unit = get_unit_by_id(id);
            fprintf(f, "  => ID:0x%04X | VAL:%.3f | UNIT:%s\n", id, val, unit);
        }
    }

    fflush(f);
#ifdef __APPLE__
    fcntl(fileno(f), F_FULLFSYNC); 
#else
    fsync(fileno(f));
#endif
    fclose(f); 

    printf("💾 [DISK SAVE] %d 패킷 동기화 완료 (Size: %ld MB)\n", 
            g_buffer_idx, get_file_size(g_log_filename) / (1024 * 1024));
    
    g_buffer_idx = 0;
    pthread_mutex_unlock(&log_mutex);
}

void add_to_log_buffer(PacketData *pkt) {
    pthread_mutex_lock(&log_mutex);
    if (g_buffer_idx < 10000) {
        memcpy(&g_temp_buffer[g_buffer_idx++], pkt, sizeof(PacketData));
    }
    pthread_mutex_unlock(&log_mutex);

    long now = get_now_ms();
    if (now - g_last_flush_time >= 1000) {
        flush_log_to_disk();
        g_last_flush_time = now;
    }
}

void close_file_logger() {
    flush_log_to_disk();
}