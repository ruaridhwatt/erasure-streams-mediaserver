CC=gcc
CFLAGS=-g -Wall
LIBS=-lpthread -lwebsockets

all: Server

Server: global_variables.h erasure_Server.h erasure_Server.c upload.h upload.c stream_video_audio.h stream_video_audio.c llist.c llist.h hashmap.c hashmap.h hashmapSettings.c hashmapSettings.h string_functions.c string_functions.h
	$(CC) $(CFLAGS) erasure_Server.c upload.c stream_video_audio.c llist.c hashmap.c hashmapSettings.c string_functions.c -o ../bin/Server $(LIBS)
clean:
	rm Server
