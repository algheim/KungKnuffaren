import ctypes


class Board(ctypes.Structure):
    _fields_ = [
        ("bit_boards", ctypes.c_uint64 * 14),
        ("en_pessant_board", ctypes.c_uint64),
        ("turn", ctypes.c_bool),
    ]

    def __repr__(self):
        return f"Turn: {self.turn}, bit_boards: {[b for b in self.bit_boards]}"


class AttackTable(ctypes.Structure):
    _fields_ = [
        ("king_table", ctypes.c_uint64 * 64),
        ("queen_table", ctypes.c_uint64 * 64),
        ("rook_table", ctypes.c_uint64 * 64),
        ("bishop_table", ctypes.c_uint64 * 64),
        ("knight_table", ctypes.c_uint64 * 64),
        ("white_pawn_table", ctypes.c_uint64 * 64),
        ("black_pawn_table", ctypes.c_uint64 * 64),
        ("white_pawn_attack_table", ctypes.c_uint64 * 64),
        ("black_pawn_attack_table", ctypes.c_uint64 * 64),
        ("ray_dir_table", ctypes.c_uint64 * 64 * 8),
    ]


class Move(ctypes.Structure):
    _fields_ = [
        ("from_x", ctypes.c_int),
        ("from_y", ctypes.c_int),
        ("to_x", ctypes.c_int),
        ("to_y", ctypes.c_int),
        ("from_index", ctypes.c_int),
        ("to_index", ctypes.c_int),
        ("from_type", ctypes.c_int),
        ("to_type", ctypes.c_int),
        ("score", ctypes.c_int),
    ]


def board_create_w(chess_lib):
    chess_lib.board_create.argtypes = []
    chess_lib.board_create.restype = ctypes.POINTER(Board)

    return chess_lib.board_create()


def board_set_start_w(chess_lib, board):
    chess_lib.board_set_start.argtypes = [ctypes.POINTER(Board)]
    chess_lib.board_set_start.restype = None

    chess_lib.board_set_start(board)


def attack_table_create_w(chess_lib):
    chess_lib.attack_table_create.argtypes = []
    chess_lib.attack_table_create.restype = ctypes.POINTER(AttackTable)

    attack_table = chess_lib.attack_table_create()
    return attack_table


def get_legal_moves_w(chess_lib, board, attack_table):
    chess_lib.get_legal_moves.argtypes = [ctypes.POINTER(Board), ctypes.POINTER(AttackTable), ctypes.POINTER(ctypes.c_int)]
    chess_lib.get_legal_moves.restype = ctypes.POINTER(Move)

    move_count = ctypes.c_int(0)
    legal_moves = chess_lib.get_legal_moves(board, attack_table, ctypes.pointer(move_count))

    return legal_moves[:move_count.value], move_count.value
