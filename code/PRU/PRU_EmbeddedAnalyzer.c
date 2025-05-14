#include <stdint.h>
#include <pru_cfg.h>
#include "resource_table_empty.h"

#define SHARED_MEM_ADDRESS    0x00010000
#define SHARED_MEM_SIZE       12288 // in-bytes
#define HEADER_OFFSET         32    // in-bytes
#define PING_PONG_BUFFER_SIZE (12288 - 32) / 2

volatile uint8_t*  shared_mem = (volatile uint8_t *) 0x00010000;
volatile register uint32_t __R31;

void main(void) {

    volatile uint8_t gpio;
    uint32_t index = HEADER_OFFSET;

    CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;
    
    for (uint32_t i = 0; i < SHARED_MEM_SIZE; i++)
        shared_mem[i] = 0;

    __delay_cycles(1000000000); // 200MHz * 5 = 10s delay

    // Lower byte of __R31 maps completely to GPIO
    // Ideally, this while loop is kept as minimal as possible
    // to allow for the fastest sampling rate possible
    while (1) {

        gpio = (uint8_t)(__R31);
        shared_mem[index] = gpio;
        index++; // Pack 4 samples into a single 32-bit word
        if (index == PING_PONG_BUFFER_SIZE - 1) {
            shared_mem[0] = 1; // flags Arm core read bytes 32 to 6127
            __delay_cycles(1);
        } else if (index == SHARED_MEM_SIZE) {
            shared_mem[0] = 2; // flags Arm core to read bytes 6129 to 12288
            index = HEADER_OFFSET; 
        } else {
            delay_cycles(2);
        }
    }

}