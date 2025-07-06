import ctypes
from c_lib import wrappers
from gui import Gui
import pygame as p
import chess


def print_diff2(chess_lib, chess_legal, my_legal):
    my_move_set = {(wrappers.move_get_from_index(chess_lib, m), wrappers.move_get_to_index(chess_lib, m)) for m in my_legal}

    for chess_move in chess_legal:
        key = (chess_move.from_square, chess_move.to_square)

        if key not in my_move_set:
            print("From:", chess_move.from_square, "To:", chess_move.to_square)


def print_diff(chess_lib, chess_legal, my_legal):
    # Create a set of (from, to) tuples from chess (reference)
    chess_move_set = {(m.from_square, m.to_square) for m in chess_legal}

    # Check which of your moves are not in the chess reference set
    for my_move in my_legal:
        from_index = wrappers.move_get_from_index(chess_lib, my_move)
        to_index = wrappers.move_get_to_index(chess_lib, my_move)
        key = (from_index, to_index)

        if key not in chess_move_set:
            print("Extra in my engine — From:", from_index, "To:", to_index)

def contains_doubles(arr):
    for i in range(len(arr)):
        for j in range(i + 1, len(arr)):
            if (arr[i] == arr[j]):
                print("Double:", arr[i].from_index, arr[i].to_index)


def print_legal(chess_lib, chess_board, board, attack_table):
    chess_legal = [move for move in chess_board.legal_moves]
    my_legal, _ = wrappers.get_legal_moves_w(chess_lib, board, attack_table)

    print("Real legal length:\t", len(chess_legal))
    print("My legal length:\t", len(my_legal))
    #print_diff(chess_legal, my_legal)
    print("Contains doubles: ", contains_doubles(my_legal))


def perft(chess_lib, board, attack_table, depth):
    if depth == 0:
        return 1
    
    total_moves = 0
    legal_moves, current_move_count = wrappers.get_legal_moves_w(chess_lib, board, attack_table)

    for move in legal_moves:
        wrappers.board_push_move(chess_lib, move, board)
        wrappers.board_change_turn(chess_lib, board)

        total_moves += perft(chess_lib, board, attack_table, depth - 1)

        wrappers.board_pop_move(chess_lib, board)
        wrappers.board_change_turn(chess_lib, board)

    fen = wrappers.board_get_fen_w(chess_lib, board)
    chess_board = chess.Board(fen)
    legal_chess_moves = [move for move in chess_board.legal_moves]
    chess_legal_count = len(legal_chess_moves)

    if (chess_legal_count != current_move_count):
        print("MOVE MISSMATCH!")
        print_diff(chess_lib, legal_chess_moves, legal_moves)
        print(wrappers.board_get_fen_w(chess_lib, board))
        print(chess_board)

    return total_moves

def main():
    ctypes.cdll.LoadLibrary("../backend/shared_lib/shared_lib.so")
    chess_lib = ctypes.CDLL("../backend/shared_lib/shared_lib.so")

    start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
    fen_2 = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0"
    board = wrappers.board_from_fen_w(chess_lib, fen_2)
    attack_table = wrappers.attack_table_create_w(chess_lib)

    total_moves = perft(chess_lib, board, attack_table, 3)

    print(total_moves)


if __name__ == "__main__":
    main()