#include "PRUHandler.h"
#include "VCDGenerator.h"

int main(int argc, char* argv[]) {

    if (argc < 2) {
        printf("Did not successfully pass a trace size argument\n");
        return -1;
    }

    size_t trace_size = (size_t) atoi(argv[1]);
    if (trace_size < PAGE_SIZE ) {
        printf("Requested trace size must be at least the length of a PAGE SIZE (4096 bytes) and be page aligned\n");
        return -1;
    }

    PRUHandler_t pru_handler;
    VCDGenerator_t vcd_generator;

    PRU_init(trace_size, &pru_handler);
    VCDGenerator_init(&vcd_generator, (uint8_t*) pru_handler.ddr_copy_buf, pru_handler.requested_trace_size);

    PRU_trace(&pru_handler);
    VCDGenerator_createWaveform(&vcd_generator);
    
    PRU_cleanup(&pru_handler);
    VCDGenerator_cleanup(&vcd_generator);
}
