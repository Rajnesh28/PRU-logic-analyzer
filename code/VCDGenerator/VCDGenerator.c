#include "VCDGenerator.h"

// Assumes caller opens fptr* in append mode
int generateVCD(FILE* fptr) {
    int status;
    status = appendHeader(fptr);
    if (!status)
        return FAILURE;

    status = insertSampledData(fptr, 35); // Loop has 7 instructions, clock is 200MHz -> 7 * 5ns
    if (!status)
        return FAILURE;

    fclose(fptr);
    return SUCCESS;
}

int appendHeader(FILE* fptr) {
    if (!fptr)
        return FAILURE;

    fprintf(fptr, "$date\n");
    fprintf(fptr, "   ");
    if (!appendDate(fptr))
        return FAILURE;
    fprintf(fptr, "$end\n");

    fprintf(fptr, "$version\n");
    fprintf(fptr, "   Sample VCD Generator\n");
    fprintf(fptr, "$end\n");

    fprintf(fptr, "$timescale 1ns $end\n\n");
    fprintf(fptr, "$scope module top $end\n\n");
    fprintf(fptr, "$var wire 1 ! clk $end\n\n");
    fprintf(fptr, "$upscope $end\n");
    fprintf(fptr, "$enddefinitions $end\n\n");
    return SUCCESS;
}

int appendDate(FILE* fptr) {
    if (!fptr)
        return FAILURE;

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char s[64];
    size_t ret = strftime(s, sizeof(s), "%c", tm);
    if (!ret)
        return FAILURE;

    fprintf(fptr, "%s\n", s);
    return SUCCESS;
}

int insertSampledData(FILE* fptr, size_t sampling_rate) {
    if (!fptr)
        return FAILURE;

    int fd = open("/dev/mem", O_RDONLY | O_SYNC);
    if (fd < 0) {
        perror("Cannot access physical memory!\n");
        return FAILURE;
    }

    volatile unsigned int *shared_mem = mmap(NULL, PRU_SHARED_MEM_SIZE_IN_BYTES,
        PROT_READ, MAP_SHARED, fd, PRU_SHARED_MEM_PHYS_ADDR);

    if (shared_mem == MAP_FAILED) {
        perror("Could not map PRU shared memory in process!\n");
        close(fd);
        return FAILURE;
    }

    fprintf(fptr, "#0\n");
    fprintf(fptr, "$dumpvars\n");
    int init = shared_mem[0];
    init = (init == 32768) ? 1 : 0;
    fprintf(fptr, "%d!\n", init);
    fprintf(fptr, "$end\n\n");
    for (int i = 1; i < PRU_SHARED_MEM_SIZE_IN_DWORDS; i++) {
        unsigned int value = shared_mem[i];
	fprintf(fptr, "#%d\n", sampling_rate * i);
    	if (shared_mem[i] == 32768) {
		fprintf(fptr, "1!\n\n");
	}
	else if (shared_mem[i] == 0) {
    		fprintf(fptr, "0!\n\n");
	}
    }

    munmap((void *)shared_mem, PRU_SHARED_MEM_SIZE_IN_BYTES);
    close(fd);
    return SUCCESS;
}
