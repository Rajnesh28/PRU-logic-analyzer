ARM_CC     = gcc
ARM_SRCS   = ./code/ArmCore/main.c ./code/ArmCore/PRUHandler.c ./code/ArmCore/VCDGenerator.c
ARM_BIN    = main

.PHONY: all clean trace

all: $(ARM_BIN)

$(ARM_BIN): $(ARM_SRCS)
	$(ARM_CC) $(ARM_SRCS) -o $(ARM_BIN)

trace: $(ARM_BIN)
	sudo ./$(ARM_BIN) 

clean:
	rm -f $(ARM_BIN)
