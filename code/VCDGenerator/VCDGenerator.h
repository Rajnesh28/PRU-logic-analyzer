#ifndef VCDGENERATOR_H
#define VCDGENERATOR_H

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define SUCCESS 1
#define FAILURE 0

#define PRU_SHARED_MEM_PHYS_ADDR      0x4a310000
#define PRU_SHARED_MEM_SIZE_IN_BYTES  12000
#define PRU_SHARED_MEM_SIZE_IN_DWORDS (PRU_SHARED_MEM_SIZE_IN_BYTES / 4)

int generateVCD(FILE* fptr);
int appendHeader(FILE* fptr);
int appendDate(FILE* fptr);
int insertSampledData(FILE* fptr, size_t sampling_rate);

#endif // VCDGENERATOR_H
