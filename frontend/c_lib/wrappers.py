import ctypes
from enum import IntEnum

class PieceType(IntEnum):
    WHITE_KING = 0
    WHITE_QUEEN = 1
    WHITE_ROOK = 2
    WHITE_BISHOP = 3
    WHITE_KNIGHT = 4
    WHITE_PAWN = 5
    WHITE_PIECES = 6

    BLACK_KING = 7
    BLACK_QUEEN = 8
    BLACK_ROOK = 9
    BLACK_BISHOP = 10
    BLACK_KNIGHT = 11
    BLACK_PAWN = 12
    BLACK_PIECES = 13

class MoveFlag(IntEnum):
    NORMAL_MOVE_FLAG = 0
    EN_PASSANT_FLAG = 1
    EN_PASSANT_AVAILABLE_FLAG = 2
    QUEEN_PROMOTION_FLAG = 3
    ROOK_PROMOTION_FLAG = 4
    BISHOP_PROMOTION_FLAG = 5
    KNIGHT_PROMOTION_FLAG = 6
    CASTLE_FLAG = 7

class UndoNode(ctypes.Structure):
    _fields_ = [
        ("move", ctypes.c_uint16),
        ("en_passant_index", ctypes.c_int8),
        ("move_piece", ctypes.c_int8),
        ("captured_piece", ctypes.c_int8),
        ("castling_rights", ctypes.c_uint8),
    ]


class Board(ctypes.Structure):
    _fields_ = [
        ("bit_boards", ctypes.c_uint64 * 14),
        ("en_passant_index", ctypes.c_int8),
        ("turn", ctypes.c_bool),
        ("castling_rights", ctypes.c_uint8),
        ("undo_stack", ctypes.POINTER(UndoNode)),
        ("undo_stack_size", ctypes.c_int),
        ("undo_stack_capacity", ctypes.c_int),
    ]


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


def board_create_w(chess_lib):
    chess_lib.board_create.argtypes = []
    chess_lib.board_create.restype = ctypes.POINTER(Board)

    return chess_lib.board_create()


def board_set_start_w(chess_lib, board):
    chess_lib.board_set_start.argtypes = [ctypes.POINTER(Board)]
    chess_lib.board_set_start.restype = None

    chess_lib.board_set_start(board)


def board_get_piece_w(chess_lib, index, board):
    chess_lib.board_get_piece.argtypes = [ctypes.c_int, ctypes.POINTER(Board)]
    chess_lib.board_get_piece.restype = ctypes.c_int

    return chess_lib.board_get_piece(ctypes.c_int(index), board)

def board_change_turn(chess_lib, board):
    chess_lib.board_change_turn.argtypes = [ctypes.POINTER(Board)]
    chess_lib.board_change_turn.restype = None

    chess_lib.board_change_turn(board)


def board_push_move(chess_lib, move, board):
    chess_lib.board_push_move.argtypes = [ctypes.c_uint16, ctypes.POINTER(Board)]
    chess_lib.board_push_move.restype = None

    chess_lib.board_push_move(move, board)

def board_pop_move(chess_lib, board):
    chess_lib.board_pop_move.argtypes = [ctypes.POINTER(Board)]
    chess_lib.board_pop_move.restype = ctypes.c_uint16

    return chess_lib.board_pop_move(board)


def get_legal_moves_w(chess_lib, board, attack_table):
    chess_lib.get_legal_moves.argtypes = [ctypes.POINTER(Board), ctypes.POINTER(AttackTable), ctypes.POINTER(ctypes.c_int)]
    chess_lib.get_legal_moves.restype = ctypes.POINTER(ctypes.c_uint16)

    move_count = ctypes.c_int(0)
    legal_moves = chess_lib.get_legal_moves(board, attack_table, ctypes.pointer(move_count))

    return legal_moves[:move_count.value], move_count.value

def board_from_fen_w(chess_lib, fen):
    fen = ctypes.create_string_buffer(fen.encode('utf-8'))
    chess_lib.board_from_fen.argtypes = [ctypes.POINTER(ctypes.c_char), ctypes.c_int]
    chess_lib.board_from_fen.restype = ctypes.POINTER(Board)

    return chess_lib.board_from_fen(fen, ctypes.c_int(len(fen) + 1))

def board_get_fen_w(chess_lib, board):
    chess_lib.board_get_fen.argtypes = [ctypes.POINTER(Board)]
    chess_lib.board_get_fen.restype = ctypes.POINTER(ctypes.c_char)

    fen_ptr = chess_lib.board_get_fen(board)
    return ctypes.string_at(fen_ptr).decode('utf-8')

def board_get_best_move_w(chess_lib, board, attack_table, depth):
    chess_lib.board_get_best_move.argtypes = [ctypes.POINTER(Board), ctypes.POINTER(AttackTable), ctypes.c_int]
    chess_lib.board_get_best_move.restype = ctypes.c_uint16

    return chess_lib.board_get_best_move(board, attack_table, ctypes.c_int(depth))

def attack_table_create_w(chess_lib):
    chess_lib.attack_table_create.argtypes = []
    chess_lib.attack_table_create.restype = ctypes.POINTER(AttackTable)

    attack_table = chess_lib.attack_table_create()
    return attack_table

# ------------------------------ Moves --------------------------------------

def move_get_from_index(chess_lib, move):
    chess_lib.move_get_from_index.argtypes = [ctypes.c_uint16]
    chess_lib.move_get_from_index.restype = ctypes.c_int

    return chess_lib.move_get_from_index(move)

def move_get_to_index(chess_lib, move):
    chess_lib.move_get_to_index.argtypes = [ctypes.c_uint16]
    chess_lib.move_get_to_index.restype = ctypes.c_int

    return chess_lib.move_get_to_index(move)

def move_get_flag(chess_lib, move):
    chess_lib.move_get_flag.argtypes = [ctypes.c_uint16]
    chess_lib.move_get_flag.restype = ctypes.c_int

    return chess_lib.move_get_flag(move)

def move_create(chess_lib, from_index, to_index, flag):
    chess_lib.move_create.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_int]
    chess_lib.move_create.restype = ctypes.c_uint16

    return chess_lib.move_create(ctypes.c_int(from_index), ctypes.c_int(to_index), ctypes.c_int(flag))

