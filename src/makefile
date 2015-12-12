CC=gcc
CFLAGS=-g -Wall
LIBS=-lpthread -lwebsockets

all: Server

Server: erasure_Server.c llist.c llist.h hashmap.c hashmap.h hashmapSettings.c hashmapSettings.h string_functions.c string_functions.h
	$(CC) $(CFLAGS) erasure_Server.c llist.c hashmap.c hashmapSettings.c string_functions.c -o Server $(LIBS)
clean:
	rm Server
