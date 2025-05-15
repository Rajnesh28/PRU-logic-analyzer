#ifndef VCDGENERATOR_H
#define VCDGENERATOR_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define SUCCESS 1
#define FAILURE 0
#define SAMPLING_RATE 5

typedef struct {
    FILE* fptr;
    uint32_t sampling_rate;
    uint8_t* ddr_copy_buf;
    size_t trace_size;
} VCDGenerator_t;

void VCDGenerator_init(VCDGenerator_t* vcdGenerator, uint8_t* ddrCopyBuf, size_t traceSize);
int VCDGenerator_createWaveform(VCDGenerator_t* vcdGenerator);
int VCDGenerator_appendHeader(VCDGenerator_t* vcdGenerator);
int VCDGenerator_appendDate(VCDGenerator_t* vcdGenerator);
int VCDGenerator_insertSampledData(VCDGenerator_t* vcdGenerator);
void VCDGenerator_cleanup(VCDGenerator_t* vcdGenerator);

#endif // VCDGENERATOR_H
