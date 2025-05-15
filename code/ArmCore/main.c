#include "PRUHandler.h"
int main(int argc, char* argv[]) {

    if (argc < 2) {
        printf("Did not successfully pass a size argument\n");
        return -1;
    }

    size_t trace_size = (size_t) atoi(argv[1]);
    if (trace_size < PAGESIZE ) {
        printf("Requested trace size must be at least the length of a PAGE SIZE (4096 bytes) and be page aligned\n");
        return -1;
    }

    PRUHandler_t pru_handler;
    PRU_init(trace_size, &pru_handler);
    PRU_load_firmware();
    PRU_trace(&pru_handler);
    print_buffer(&pru_handler);
    PRU_cleanup(&pru_handler);
}
