CC=gcc
LIB_DIR=./lib/
INC_DIR=./include/
BIN_DIR=./bin/
SRC_DIR=./src/
TSTDIR=./testes/

all: octhread alib

octhread: $(SRC_DIR)/cthread.c
	$(CC) -c $^ -Wall
	mv cthread.o $(BIN_DIR)

alib: $(BIN_DIR)/cthread.o $(BIN_DIR)/support.o
	ar crs libcthread.a
	mv libcthread.a $(LIB_DIR)

clean:
	rm -rf $(LIB_DIR)/*.a $(BIN_DIR)/cthread.o
