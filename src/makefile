OUT=erasure_server
COMPILER=gcc
FLAGS=-g -Wall -pedantic
LIBS=-lwebsockets -lpthread

BIN_DIR=../bin

all: $(OUT)

$(OUT):erasure_server.o upload.o info.o audio.o video.o file_utilities.o intern.o
	$(COMPILER) $(FLAGS) -o $(BIN_DIR)/$(OUT) $(BIN_DIR)/erasure_server.o $(BIN_DIR)/upload.o $(BIN_DIR)/info.o $(BIN_DIR)/audio.o $(BIN_DIR)/video.o $(BIN_DIR)/file_utilities.o $(BIN_DIR)/intern.o $(LIBS)

erasure_server.o: erasure_server.c erasure_server.h upload.h info.h audio.h video.h file_utilities.h
	$(COMPILER) $(FLAGS) -o $(BIN_DIR)/erasure_server.o erasure_server.c -c

upload.o: upload.c upload.h
	$(COMPILER) $(FLAGS) -o $(BIN_DIR)/upload.o upload.c -c

info.o: info.c info.h file_utilities.h
	$(COMPILER) $(FLAGS) -o $(BIN_DIR)/info.o info.c -c
	
audio.o: audio.c audio.h file_utilities.h
	$(COMPILER) $(FLAGS) -o $(BIN_DIR)/audio.o audio.c -c

video.o: video.c video.h file_utilities.h
	$(COMPILER) $(FLAGS) -o $(BIN_DIR)/video.o video.c -c

file_utilities.o: file_utilities.c file_utilities.h
	$(COMPILER) $(FLAGS) -o $(BIN_DIR)/file_utilities.o file_utilities.c -c

intern.o: intern.c intern.h
	$(COMPILER) $(FLAGS) -o $(BIN_DIR)/intern.o intern.c -c