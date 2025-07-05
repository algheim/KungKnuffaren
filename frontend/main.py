import ctypes
from c_lib import wrappers
from gui import Gui
import pygame as p


def make_enemy_move(chess_lib, board, attack_table):
        best_move = wrappers.board_get_best_move_w(chess_lib, board, attack_table, 3)
        wrappers.board_push_move(chess_lib, best_move, board)
        wrappers.board_change_turn(chess_lib, board)


def main():
    ctypes.cdll.LoadLibrary("../backend/shared_lib/shared_lib.so")
    chess_lib = ctypes.CDLL("../backend/shared_lib/shared_lib.so")

    gui = Gui(chess_lib)
    clock = p.time.Clock()

    #board = wrappers.board_create_w(chess_lib)
    start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
    fen2 = "rnbq1b1r/pppppppp/4n1k1/B7/3P1P2/4P3/PPP3PP/RNBQK1NR w KQ - 0 1"
    board = wrappers.board_from_fen_w(chess_lib, start_fen)
    #wrappers.board_set_start_w(chess_lib, board)
    attack_table = wrappers.attack_table_create_w(chess_lib)
    gui.draw_board(board)

    while True:
        gui.update_event()
        gui.update_pop_push_move(board)

        if gui.update_active_square():
            legal_moves, _ = wrappers.get_legal_moves_w(chess_lib, board, attack_table)
            gui.update_board(board, legal_moves)

            if not board.contents.turn:
                make_enemy_move(chess_lib, board, attack_table)

        gui.draw_board(board)

        clock.tick(30)



if __name__ == "__main__":
    main()