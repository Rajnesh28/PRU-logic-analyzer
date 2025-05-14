#include "PRUHandler.h"

/**
 * Initializes the PRUHandler_t struct
 * Allocates memory in address space based on requested trace size. 
 *
 * @param requestedTraceSize - Size of the requested trace size in 32-bit words
 * @param pruHandler         - PRUHandler_t struct object to initialize
*/
void PRU_init(size_t requestedTraceSize, PRUHandler_t *pruHandler) {
    pruHandler->complete_flag  = 0;
    pruHandler->word_count     = 0;
    pruHandler->requested_trace_size = requestedTraceSize;
    pruHandler->ddr_copy_buf = (uint32_t*) mmap(NULL, requestedTraceSize, PROT_READ | PROT_WRITE,
                                                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
                                                
    if (pruHandler->ddr_copy_buf == MAP_FAILED) {
        printf("Could not allocate %ld in address space using mmap()\n", requestedTraceSize);
        exit(-1);
    }

    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        printf("Cannot access phyical memory\n");
        exit(-1);
    }

    pruHandler->pru_shared_mem_fd  = fd;
    pruHandler->pru_shared_mem = (uint32_t*) mmap(NULL, PRU_SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, 
        MAP_SHARED, fd, PRU_SHARED_MEM_PHYS_ADDR);
        
    if (pruHandler->pru_shared_mem == MAP_FAILED) {
        printf("Could not map physical memory into virtual memory space\n");
        exit(-1);
    }
    return;
}


/**
* Loads firmware to PRU1 core
* 
* Warning: Must have pru_fw.bin (compiled with clpru) present in ../firmware/ folder 
*/
void PRU_load_firmware(void) {
    int ret;
    ret = system("cp ../PRU/bin/pru_fw /lib/firmware/am335x-pru1-fw");
    if (ret == -1) {
        printf("Could not load firmware to PRU1 core\n");
        exit(-1);
    }
};

/**
 * Starts the PRU1 Core on the TI AM3358 Sitara Processor using sysfs API shell command
 * 
 * Warning: In this implementation, user only needs to call PRU_trace(..) not PRU_start()
*/
void PRU_start(void) {
    int ret;
    ret = system("echo start | sudo tee /sys/class/remoteproc/remoteproc2/state");
    if (ret == -1) {
        printf("Could not start the PRU1 core\n");
        exit(-1);
    }
    
    return;
}

/**
 * Begins tracing incoming 8-bit signal by starting the PRU, 
 * and coordinating a ping-pong buffer data transfer between 
 * the collected trace data in the PRU Shared Memory region 
 * to the mapped address in memory.
*/
void PRU_trace(PRUHandler_t *pruHandler) {
    volatile uint32_t* shared_mem_base_address = pruHandler->pru_shared_mem; 
    uint32_t*          ddr_buf                 = pruHandler->ddr_copy_buf;

    PRU_start();

    int index   = 0;
    int counter = 0;
    int requested_trace_size_in_dwords = pruHandler->requested_trace_size / 4;
    while (pruHandler->word_count < 6144) {
        while (*(shared_mem_base_address) == 0);

        if (*(shared_mem_base_address) == 1) {
            for (int i = 0; i < 1532; i++)
                ddr_buf[index + i] =  *(shared_mem_base_address + HEADER_OFFSET + i);
            *(shared_mem_base_address) = 0; }

        else if (*(shared_mem_base_address) == 2) {
            for (int i = 0; i < 1532; i++) 
                ddr_buf[index + i] = *(shared_mem_base_address + HEADER_OFFSET + i + 1532);
            *(shared_mem_base_address) = 0; }
	index += 1532;
        pruHandler->word_count += 1532;
    }

    pruHandler->complete_flag = 1;
    PRU_stop();
}

/**
 * Stops the PRU1 Core on the TI AM3358 Sitara Processor using sysfs API shell command
 *
*/
void PRU_stop(void) {
    int ret;
    ret = system("echo stop | sudo tee /sys/class/remoteproc/remoteproc2/state");
    if (ret == -1) {
        printf("Could not stop the PRU1 core\n");
        exit(-1);
    }

    return;
}

/**
 * Unmaps the address space region that holds the trace data
 * 
 * @param pruHandler object which has the base address of the mapped address space
 *                   in its ddr_copy_buf field
 * Warning: Call after creating waveform with trace using VCDGenerator
 *          as this will unmap the address space with the traced data.
*/
void PRU_cleanup(PRUHandler_t *pruHandler) {
    munmap(pruHandler->ddr_copy_buf, pruHandler->requested_trace_size);
    munmap(pruHandler->pru_shared_mem, PRU_SHARED_MEM_SIZE);
    close(pruHandler->pru_shared_mem_fd);
}

#ifdef DEBUG
void print_buffer(PRUHandler_t* pruHandler) {
    uint8_t* ddrPtr8bit = (uint8_t*)pruHandler->ddr_copy_buf;
    
    for (int i = 0; i < 24576; i++)
        printf("Index: %d, Data: %d\n", i, ddrPtr8bit[i]);        

    return;
}
#endif // DEBUG
