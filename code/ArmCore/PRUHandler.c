#include "PRUHandler.h"

/**
 * Initializes the PRUHandler_t struct
 * Allocates memory in the heap based on the requested trace size.
 * Maps PRU Shared memory, and passes the base pointer into a struct variable
 *
 * @param requestedTraceSize - Size of the requested trace size in bytes
 * @param pruHandler         - PRUHandler_t struct object to initialize
*/
void PRU_init(size_t requestedTraceSize, PRUHandler_t *pruHandler) {
    pruHandler->complete_flag  = 0;
    pruHandler->word_count     = 0;

    if (requestedTraceSize < 0) {
        printf("Cannot trace a negative amount of bytes\n");
        exit(-1);
    }
    pruHandler->requested_trace_size = requestedTraceSize;
    
    pruHandler->ddr_copy_buf   = (uint32_t*) malloc(requestedTraceSize);
    if (pruHandler->ddr_copy_buf == NULL) {
	    printf("Could not allocate memory using malloc(), considering reducing trace size\n");
    	exit(-1);
    }

    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        printf("Cannot access phyical memory\n");
        exit(-1);
    }

    pruHandler->pru_shared_mem_fd  = fd;
    pruHandler->pru_shared_mem     = (uint32_t*) mmap(NULL, PRU_SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, 
                                    MAP_SHARED, fd, PRU_SHARED_MEM_PHYS_ADDR);
        
    if (pruHandler->pru_shared_mem == MAP_FAILED || pruHandler->pru_shared_mem == NULL) {
        printf("Could not map physical memory into virtual memory space\n");
        free(pruHandler->ddr_copy_buf);
        exit(-1);
    }

    return;
}

/**
* Loads firmware to PRU1 core
* 
* Warning: Must have pru_fw.bin (compiled with clpru) present in ../firmware/ folder 
*/
int PRU_load_firmware(void) {
    int ret;
    ret = system("cp ./code/PRU/bin/pru_fw /lib/firmware/am335x-pru1-fw");
    if (ret == -1)
        printf("Could not load firmware to PRU1 core!\n");
    return ret;
};

/**
 * Starts the PRU1 Core on the TI AM3358 Sitara Processor using sysfs API shell command
 * 
 * Warning: In this implementation, user only needs to call PRU_trace(..) not PRU_start().
 *          PRU_trace() will load the firmware, start the core and stop it.
*/
int PRU_start(void) {
    int ret;
    ret = system("echo start | sudo tee /sys/class/remoteproc/remoteproc2/state");
    if (ret == -1)
        printf("Could not start the PRU1 core!\n");
    return ret;
}

/**
 * Starts a PRU-based trace session to capture an 8-bit digital signal.
 *
 * This function initiates PRU execution and continuously monitors a ping-pong
 * buffer flag in PRU shared memory to transfer sampled data into a DDR buffer
 * on the host side. The PRU writes sampled data into alternating halves of
 * shared memory, which are then copied into the provided DDR buffer.
 *
 * @param pruHandler         - PRUHandler_t struct object
 *
 */
void PRU_trace(PRUHandler_t *pruHandler) {
    volatile uint32_t* sharedMemBaseAddress = pruHandler->pru_shared_mem; 
    uint32_t*          ddrBuffer            = pruHandler->ddr_copy_buf;

    if (PRU_load_firmware() == -1)
        PRU_cleanup(pruHandler);

    if (PRU_start() == -1)
        PRU_cleanup(pruHandler);

    int index   = 0;
    int counter = 0;
    int requested_trace_size_in_dwords = pruHandler->requested_trace_size / BYTES_PER_DWORD;
    int ping_or_pong_buffer_size_in_dwords = PING_PONG_HALF_SIZE / BYTES_PER_DWORD;

    while (pruHandler->word_count < requested_trace_size_in_dwords) {
        while (*(sharedMemBaseAddress) == 0);

        if (*(sharedMemBaseAddress) == 1) {
	    if (index + ping_or_pong_buffer_size_in_dwords > requested_trace_size_in_dwords) {
		for (int i = 0; i < requested_trace_size_in_dwords - index; i++) {
		    ddrBuffer[index + i] = *(sharedMemBaseAddress + HEADER_OFFSET + i);
		}
		break;
	    }

	    for (int i = 0; i < ping_or_pong_buffer_size_in_dwords; i++)
                ddrBuffer[index + i] =  *(sharedMemBaseAddress + HEADER_OFFSET + i);
            *(sharedMemBaseAddress) = 0; }

        else if (*(sharedMemBaseAddress) == 2) {
	    if (index + ping_or_pong_buffer_size_in_dwords > requested_trace_size_in_dwords) {
		for (int i = 0; i < requested_trace_size_in_dwords - index; i++) {
			ddrBuffer[index + i] = *(sharedMemBaseAddress + HEADER_OFFSET + i + ping_or_pong_buffer_size_in_dwords);
		}
		break;
	    }
            for (int i = 0; i < ping_or_pong_buffer_size_in_dwords; i++) 
                ddrBuffer[index + i] = *(sharedMemBaseAddress + HEADER_OFFSET + i + ping_or_pong_buffer_size_in_dwords);
            *(sharedMemBaseAddress) = 0; 
	}

	    index += ping_or_pong_buffer_size_in_dwords;
        pruHandler->word_count += ping_or_pong_buffer_size_in_dwords;
    }

    pruHandler->complete_flag = 1;
    
    if (PRU_stop() == -1)
        PRU_cleanup(pruHandler);

    return;
}

/**
 * Stops the PRU1 Core on the TI AM3358 Sitara Processor using sysfs API shell command
 *
*/
int PRU_stop(void) {
    int ret;
    ret = system("echo stop | sudo tee /sys/class/remoteproc/remoteproc2/state");
    if (ret == -1)
        printf("Could not stop the PRU1 core\n");
    return ret;
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
    free(pruHandler->ddr_copy_buf);
    munmap(pruHandler->pru_shared_mem, PRU_SHARED_MEM_SIZE);
    close(pruHandler->pru_shared_mem_fd);
}

#ifdef DEBUG
void print_buffer(PRUHandler_t* pruHandler) {
    uint8_t* ddrPtr8bit = (uint8_t*) pruHandler->ddr_copy_buf;
    
    for (int i = 0; i < pruHandler->requested_trace_size; i++)
        printf("Index: %d, Data: %d\n", i, ddrPtr8bit[i]);        
    return;
}
#endif // DEBUG
