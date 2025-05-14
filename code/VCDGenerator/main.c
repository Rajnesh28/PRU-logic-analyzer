#include "VCDGenerator.h"

int main() {
    FILE* fptr = fopen("wave.vcd", "a");
    if (!fptr) {
        perror("Caller failed to create file\n");
        return FAILURE;
    }

    int status = generateVCD(fptr);

    return status;
}
