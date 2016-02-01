CC=g++
FLAGS=-O0 -Wall -lopencv_highgui -lopencv_core -lopencv_imgproc -lpthread
RM=rm -rfv

C_SRCS += \
line_finder.c\
main.c

all : line_finder.elf

line_finder.elf : $(C_SRCS)
	$(CC) $(FLAGS) $(C_SRCS) -o $@


clean :
	$(RM) *.elf
