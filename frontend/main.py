import ctypes


class Board(ctypes.Structure):
    _fields_ = [
        ("bit_boards", ctypes.c_uint64 * 14),
        ("en_pessant_board", ctypes.c_uint64),
        ("turn", ctypes.c_bool),
    ]

    def __repr__(self):
        return f"<Board turn={self.turn}, en_passant={self.en_pessant_board}, bit_boards={[hex(b) for b in self.bit_boards]}>"



def main():
    ctypes.cdll.LoadLibrary("../backend/shared_lib/shared_lib.so")
    chess_lib = ctypes.CDLL("../backend/shared_lib/shared_lib.so")

    chess_lib.board_create.argtypes = []
    chess_lib.board_create.restype = ctypes.POINTER(Board)

    b = chess_lib.board_create()

    print(b.contents)


if __name__ == "__main__":
    main()