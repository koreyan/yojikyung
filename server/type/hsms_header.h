#ifndef HSME_HEADER_H
#define HSME_HEADER_H

#include "common.h"

/* =========================
   HSMS Header
========================= */
typedef struct {
    uint32_t length;
    uint16_t session_id;
    uint8_t  stream;
    uint8_t  function;
    uint8_t  p_type;
    uint8_t  s_type;
    uint32_t system_bytes;
} HSMSHeader;

#endif