#include "PRUHandler.h"

int main() {
    size_t size_request = 1024;
    PRUHandler_t pru_handler;
    PRU_init(size_request, pru_handler);
    PRU_load_firmware();
}