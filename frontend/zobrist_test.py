import ctypes
from c_lib import wrappers
from gui import Gui
import pygame as p


def make_enemy_move(chess_lib, board, attack_table):
        best_move = wrappers.search_best_move(chess_lib, board, attack_table, 1, wrappers.SearchAlg.ALPHA_BETA_ORDERED)
        #wrappers.search_best_move(chess_lib, board, attack_table, 3, wrappers.SearchAlg.ALPHA_BETA)
        ##best_move = wrappers.search_best_move(chess_lib, board, attack_table, 3, wrappers.SearchAlg.MIN_MAX)
        #best_move = wrappers.search_best_move(chess_lib, board, attack_table, 3, wrappers.SearchAlg.ITERATIVE_DEEPENING)

        if not wrappers.move_exists(chess_lib, best_move):
             print("I lost!!")
             return
        wrappers.board_push_move(chess_lib, best_move, board)
        wrappers.board_change_turn(chess_lib, board)


def main():
    ctypes.cdll.LoadLibrary("../backend/shared_lib/shared_lib.so")
    chess_lib = ctypes.CDLL("../backend/shared_lib/shared_lib.so")
    chess_lib.zobrist_init()

    gui = Gui(chess_lib)
    clock = p.time.Clock()

    #board = wrappers.board_create_w(chess_lib)
    start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
    fen2 = "rnbq1b1r/pppppppp/4n1k1/B7/3P1P2/4P3/PPP3PP/RNBQK1NR w KQ - 0 1"
    fen3 = "r4k1r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/5Q1p/PPPBBPPP/RN2K2R w QK - 0 0"
    board = wrappers.board_from_fen_w(chess_lib, fen3)
    #wrappers.board_set_start_w(chess_lib, board)
    attack_table = wrappers.attack_table_create_w(chess_lib)
    gui.draw_board(board)

    while True:
        gui.update_event()
        gui.update_pop_push_move(board)

        if gui.update_active_square():
            fen = wrappers.board_get_fen_w(chess_lib, board)
            legal_moves, _ = wrappers.get_legal_moves_w(chess_lib, board, attack_table)
            gui.update_board(board, legal_moves)
            #print("No depth eval: ", wrappers.board_evaluate_current(chess_lib, board))

            absolute_hash = wrappers.calculate_zobrist_hash(chess_lib, board)
            relative_hash = wrappers.board_get_zobrist_hash(chess_lib, board)

            wrappers.test_search(chess_lib, board, attack_table)

            if absolute_hash == relative_hash:
                 print("Absolute matches relative!!!", absolute_hash, relative_hash)
            else:
                 print("Absolute does not match relative!!!", absolute_hash, relative_hash)

        gui.draw_board(board)

        clock.tick(30)

if __name__ == "__main__":
    main()