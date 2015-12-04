OUT=erasure_server
COMPILER=gcc
FLAGS=-Wall
LIBS=-lwebsockets

BIN_DIR=../bin

all: $(OUT)

$(OUT): ErasureServer.c
	$(COMPILER) $(FLAGS) $(LIBS) ErasureServer.c -o $(BIN_DIR)/$(OUT)
