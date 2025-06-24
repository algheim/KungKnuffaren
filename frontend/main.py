import ctypes
from c_lib import wrappers


def main():
    ctypes.cdll.LoadLibrary("../backend/shared_lib/shared_lib.so")
    chess_lib = ctypes.CDLL("../backend/shared_lib/shared_lib.so")

    chess_lib.board_create.argtypes = []

    board = wrappers.board_create_w(chess_lib)
    wrappers.board_set_start_w(chess_lib, board)
    attack_table = wrappers.attack_table_create_w(chess_lib)

    legal_moves, move_count = wrappers.get_legal_moves_w(chess_lib, board, attack_table)

    for move in legal_moves:
        print(move.from_index, "\t", move.to_index)
    


if __name__ == "__main__":
    main()