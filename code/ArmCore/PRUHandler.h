#ifndef PRUHANDLER_H
#define PRU_HANDLER_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>

#define PAGESIZE                  4096
#define PRU_SHARED_MEM_PHYS_ADDR  0x4A310000
#define PRU_SHARED_MEM_SIZE       12288
#define HEADER_OFFSET             0x8

typedef struct {
    volatile uint32_t complete_flag;
    uint32_t          word_count;
    size_t            requested_trace_size;
    uint32_t*         ddr_copy_buf;
    uint32_t*         pru_shared_mem; 
} PRUHandler_t;

void PRU_init(size_t requestedTraceSize, PRUHandler_t* pruHandler);

void PRU_load_firmware();

void PRU_start(void);

void PRU_trace(const char* bin);

void PRU_stop(void);

void PRU_cleanup(PRUHandler_t* pruHandler);

#ifdef DEBUG
    void print_buffer(PRUHandler_t* pruHandler);
#endif //DEBUG

#endif // PRUHANDLER_H