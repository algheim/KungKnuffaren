# Compiler and flags
CC = gcc
CFLAGS = -Wall -std=c99 -O3
CFLAGS_LIB = -shared -fPIC -std=c99 -Wall

# Paths
OBJ_PATH = obj/
BIN_PATH = bin/
BIN_LIB_PATH = shared_lib/

# Object files
OBJS = \
    $(OBJ_PATH)board.o \
    $(OBJ_PATH)movegenerator.o \
    $(OBJ_PATH)attacktable.o \
    $(OBJ_PATH)move.o \
    $(OBJ_PATH)bitboard.o \
	$(OBJ_PATH)fenparser.o \
	$(OBJ_PATH)evaluate.o \
	$(OBJ_PATH)search.o \
	$(OBJ_PATH)piece.o \
	$(OBJ_PATH)transpositiontable.o \

# Default target
backend: $(BIN_PATH)kungknuffaren

# Rule to compile .c to .o
$(OBJ_PATH)%.o: %.c
	mkdir -p $(OBJ_PATH)
	$(CC) $(CFLAGS) -c -o $@ $<

# Rule to build the main binary
$(BIN_PATH)kungknuffaren: $(OBJS) $(OBJ_PATH)main.o
	mkdir -p $(BIN_PATH)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(OBJ_PATH)main.o

frontend_lib: $(OBJS)
	mkdir -p $(BIN_LIB_PATH)
	$(CC) $(CFLAGS_LIB) -o $(BIN_LIB_PATH)/shared_lib.so $(OBJS)

clean:
	rm -rf $(OBJ_PATH) $(BIN_PATH) $(BIN_LIB_PATH)
