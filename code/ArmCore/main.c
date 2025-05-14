#include "PRUHandler.h"
#define PAGE_SIZE 4096
int main() {
    size_t size_request = 4096 * 10;
    PRUHandler_t pru_handler;
    PRU_init(size_request, &pru_handler);
    PRU_load_firmware();
    PRU_trace(&pru_handler);
    print_buffer(&pru_handler);
    PRU_cleanup(&pru_handler);
}
