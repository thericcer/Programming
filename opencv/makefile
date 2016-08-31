CC=g++
FLAGS=-O0 -Wall -lopencv_highgui -lopencv_core -lopencv_imgproc -lopencv_videoio -lpthread
RM=rm -rfv

all : lines.elf video.elf image.elf threads.elf

lines.elf : lines.cpp
	$(CC) $(FLAGS) lines.cpp -o $@

video.elf : video_capture.cpp
	$(CC) $(FLAGS) video_capture.cpp -o $@

image.elf : display_image.cpp
	$(CC) $(FLAGS) display_image.cpp -o $@

threads.elf : threads.cpp
	$(CC) $(FLAGS) threads.cpp -o $@


clean :
	$(RM) lines.elf video.elf image.elf threads.elf
