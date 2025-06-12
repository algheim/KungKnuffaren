CFLAGS= -g -Wall -std=c99
OBJ_PATH= obj/
BIN_PATH= bin/

$(OBJ_PATH)%.o: %.c
	mkdir -p $(OBJ_PATH)
	gcc $(CFLAGS) -c -o $@ $<

kungknuffaren3000: $(OBJ_PATH)board.o $(OBJ_PATH)main.o $(OBJ_PATH)movegenerator.o $(OBJ_PATH)attacktable.o $(OBJ_PATH)move.o $(OBJ_PATH)bitboard.o
	mkdir -p $(BIN_PATH)
	gcc $(CFLAGS) -o $(BIN_PATH)kungknuffaren $(OBJ_PATH)board.o $(OBJ_PATH)movegenerator.o $(OBJ_PATH)attacktable.o $(OBJ_PATH)main.o $(OBJ_PATH)move.o $(OBJ_PATH)bitboard.o

