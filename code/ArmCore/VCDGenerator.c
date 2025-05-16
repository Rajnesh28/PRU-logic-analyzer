#include "VCDGenerator.h"

/**
 * Initializes a VCDGenerator_t instance with buffer, size, and file handle.
 *
 * @param vcdGenerator  VCDGenerator_t object to initialize
 * @param ddrCopyBuf    Pointer to DDR buffer containing trace data
 * @param traceSize     Size of the trace in bytes
*/
void VCDGenerator_init(VCDGenerator_t* vcdGenerator, uint8_t* ddrCopyBuf, size_t traceSize) {
    vcdGenerator->fptr          = fopen("wave.vcd", "w");
    if (vcdGenerator->fptr == NULL) {
        printf("Could not create a valid waveform file: wave.vcd\n");
        return;
    }

    vcdGenerator->sampling_rate = SAMPLING_RATE;
    vcdGenerator->ddr_copy_buf  = ddrCopyBuf;
    vcdGenerator->trace_size    = traceSize;
}

/**
 * Generates a complete VCD file for waveform viewing (e.g., in GTKWave).
 * 
 * Wraps header creation and trace data insertion.
 *
 * @param vcdGenerator  Initialized VCDGenerator_t object
*/
int VCDGenerator_createWaveform(VCDGenerator_t* vcdGenerator) {
    int status;
    status = VCDGenerator_appendHeader(vcdGenerator);
    if (!status)
        return FAILURE;

    status = VCDGenerator_insertSampledData(vcdGenerator); 
    if (!status)
        return FAILURE;

    return SUCCESS;
}

/**
* Appends header information to the VCD file including the date
*
* @param pruHandler         - VCDGenerator_t struct object
*/
int VCDGenerator_appendHeader(VCDGenerator_t* vcdGenerator) {
    FILE* fptr = vcdGenerator->fptr;

    if (!fptr)
        return FAILURE;

    fprintf(fptr, "$date\n");
    fprintf(fptr, "   ");
    if (!VCDGenerator_appendDate(vcdGenerator))
        return FAILURE;
    fprintf(fptr, "$end\n");

    fprintf(fptr, "$version\n");
    fprintf(fptr, "   Sample VCD Generator\n");
    fprintf(fptr, "$end\n");

    fprintf(fptr, "$timescale 1ns $end\n\n");
    fprintf(fptr, "$scope module top $end\n\n");
    fprintf(fptr, "$var wire 8 B signal_in $end\n\n");
    fprintf(fptr, "$upscope $end\n");
    fprintf(fptr, "$enddefinitions $end\n\n");
    return SUCCESS;
}

/**
* Appends date information to the VCD file
*
* @param pruHandler         - VCDGenerator_t struct object
*/
int VCDGenerator_appendDate(VCDGenerator_t* vcdGenerator) {
    FILE* fptr = vcdGenerator->fptr;

    if (!fptr)
        return FAILURE;

    time_t t      = time(NULL);
    struct tm *tm = localtime(&t);
    char s[64];
    size_t ret = strftime(s, sizeof(s), "%c", tm);
    if (!ret)
        return FAILURE;

    fprintf(fptr, "%s\n", s);
    return SUCCESS;
}

/**
* Loops over the entire DDR trace buffer and completes the
* VCD file by plotting the trace data at intervals dictated by 
* vcdGenerator->sampling_rate
*
* @param pruHandler         - VCDGenerator_t struct object
*/
int VCDGenerator_insertSampledData(VCDGenerator_t* vcdGenerator) {
    uint32_t value;
    int init;

    FILE* fptr             = vcdGenerator->fptr;
    uint8_t* shared_mem    = vcdGenerator->ddr_copy_buf;
    uint32_t sampling_rate = vcdGenerator->sampling_rate;
    size_t trace_size      = vcdGenerator->trace_size;

    if (!fptr)
        return FAILURE;

    fprintf(fptr, "$dumpvars\n\n");
    
    init = shared_mem[0];
    
    fprintf(fptr, "# Initial value \n");
    fprintf(fptr, "b");

    for (int i = 7; i >= 0; i--)
	    fprintf(fptr, "%d", (shared_mem[0] >> i) & 1);

    fprintf(fptr, "B\n", init);
    fprintf(fptr, "$end\n\n");

    for (int i = 1; i < trace_size; i++) {
        value = shared_mem[i];
        fprintf(fptr, "#%d\n", sampling_rate * i);
	    fprintf(fptr, "b");
        for (int j = 7; j >= 0; j--) {
	        fprintf(fptr, "%d", (value >> j) & 1);
	    }
	    fprintf(fptr, " B\n\n");
    }

    return SUCCESS;
}

/**
* Cleans up vcdGenerator struct by closing the FILE* to the VCD file
*
* @param pruHandler         - VCDGenerator_t struct object
*/
void VCDGenerator_cleanup(VCDGenerator_t* vcdGenerator) {
    fclose(vcdGenerator->fptr);
}