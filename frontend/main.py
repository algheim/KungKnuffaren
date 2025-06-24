import ctypes
from c_lib import wrappers
from gui import Gui
import pygame as p

def main():
    ctypes.cdll.LoadLibrary("../backend/shared_lib/shared_lib.so")
    chess_lib = ctypes.CDLL("../backend/shared_lib/shared_lib.so")

    gui = Gui(chess_lib)
    clock = p.time.Clock()

    board = wrappers.board_create_w(chess_lib)
    wrappers.board_set_start_w(chess_lib, board)
    attack_table = wrappers.attack_table_create_w(chess_lib)
    gui.update_button_board(board)
    gui.draw_board()


    while True:
        gui.update_event()

        if gui.update_active_square():
            gui.update_board(board, attack_table)
            gui.update_button_board(board)

        gui.draw_board()

        clock.tick(30)


if __name__ == "__main__":
    main()