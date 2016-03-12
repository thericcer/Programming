CC=g++
FLAGS=-O0 -Wall -lopencv_highgui -lopencv_core -lopencv_imgproc -lpthread -lncurses
RM=rm -rfv

C_SRCS += \
main.c

all : autopilot.elf

autopilot.elf : $(C_SRCS)
	$(CC) $(FLAGS) $(C_SRCS) -o $@


clean :
	$(RM) *.elf
