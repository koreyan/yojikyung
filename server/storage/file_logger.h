#ifndef FILE_LOGGER_H
#define FILE_LOGGER_H

#include "../common/common.h"
#include "../type/packet_data.h"

#define LOG_BUFFER_SIZE 1200 // 1초치 임시 버퍼

void init_file_logger(const char* filename);
void add_to_log_buffer(PacketData *pkt);
void flush_log_to_disk();
void close_file_logger();

#endif