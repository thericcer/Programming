CC=g++
FLAGS=-O0 -Wall -lopencv_highgui -lopencv_core -lopencv_imgproc -lpthread
RM=rm -rfv

all : line_finder.elf

line_finder.elf : line_finder.cpp
	$(CC) $(FLAGS) line_finder.cpp -o $@


clean :
	$(RM) *.elf
