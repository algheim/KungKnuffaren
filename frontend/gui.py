
import pygame as p
from button import Button
import os
from c_lib import wrappers

SCREEN_HEIGHT = 600
SCREEN_LENGTH = 600

LIGHT_SQUARE = (150, 150, 150)
DARK_SQUARE = (50, 50, 50)
MARKED_SQUARE = (200, 100, 100)

WHITE_KNIGHT_P = os.path.join("sprites", "knight-w.svg")
WHITE_KING_P   = os.path.join("sprites", "king-w.svg")
WHITE_ROOK_P   = os.path.join("sprites", "rook-w.svg")
WHITE_BISHOP_P = os.path.join("sprites", "bishop-w.svg")
WHITE_PAWN_P   = os.path.join("sprites", "pawn-w.svg")
WHITE_QUEEN_P  = os.path.join("sprites", "queen-w.svg")

BLACK_KNIGHT_P = os.path.join("sprites", "knight-b.svg")
BLACK_KING_P   = os.path.join("sprites", "king-b.svg")
BLACK_ROOK_P   = os.path.join("sprites", "rook-b.svg")
BLACK_BISHOP_P = os.path.join("sprites", "bishop-b.svg")
BLACK_PAWN_P   = os.path.join("sprites", "pawn-b.svg")
BLACK_QUEEN_P  = os.path.join("sprites", "queen-b.svg")


class Gui:
    def __init__(self, chess_lib):
        self.win = p.display.set_mode((SCREEN_LENGTH, SCREEN_HEIGHT))
        self.event = None
        self.button_board = self._get_button_board()
        self.sprite_paths = self._get_sprite_paths()
        self.chess_lib = chess_lib
        self.prev_active_square = None
        self.active_square = None
        self.marked_squares = set()

    def draw_board(self):
        self.win.fill((0, 0, 0))

        for row in self.button_board:
            for button in row:
                button.draw(self.win)

        p.display.flip()

    def update_board(self, board, attack_table):
        legal_moves, move_count = wrappers.get_legal_moves_w(self.chess_lib, board, attack_table)
        self.marked_squares = set()

        if self.prev_active_square != None and self.active_square != None:
            from_index = self.prev_active_square[0] * 8 + self.prev_active_square[1]
            to_index = self.active_square[0] * 8 + self.active_square[1]
            move = wrappers.Move()
            move.from_index = from_index
            move.to_index = to_index
            wrappers.board_make_move(self.chess_lib, move, board)
            wrappers.board_change_turn(self.chess_lib, board)

            self.active_square = None
            self.prev_active_square = None

        elif self.active_square != None:
            from_index = self.active_square[0] * 8 + self.active_square[1]
            for move in legal_moves:
                if move.from_index == from_index:
                    self.marked_squares.add(move.to_index)


    def update_button_board(self, board):
        for i in range(8):
            for j in range(8):
                sprite = None
                piece_type = wrappers.board_get_piece_w(self.chess_lib, i * 8 + j, board)
                if piece_type != -1:
                    path = self.sprite_paths[piece_type]
                    sprite = p.image.load(path)

                self.button_board[i][j].dispay_secondary = False
                if (i * 8 + j) in self.marked_squares:
                    self.button_board[i][j].dispay_secondary = True

                self.button_board[i][j].set_sprite(sprite)

    def update_event(self):
        self.event = None
        for event in p.event.get():
            if event.type == p.QUIT:
                p.quit()
                quit()

            if event.type == p.MOUSEBUTTONDOWN:
                self.event = event

    def update_active_square(self):
        if self.event == None:
            return False
        
        for i in range(8):
            for j in range(8):
                if (self.button_board[i][j].check_if_pressed(self.event.pos[0], self.event.pos[1])):
                    if self.active_square == (i, j):
                        self.prev_active_square = None
                        self.active_square = None
                    else:
                        self.prev_active_square = self.active_square
                        self.active_square = (i, j)
                    return True

        return False
    
    def _get_button_board(self):
        buttons = [[None for i in range(8)] for j in range(8)]
        length = min(SCREEN_LENGTH, SCREEN_HEIGHT) / 8

        for i in range(8):
            for j in range(8):
                color = LIGHT_SQUARE
                if i % 2 == 0 and j % 2 != 0:
                    color = DARK_SQUARE
                if i % 2 != 0 and j % 2 == 0:
                    color = DARK_SQUARE

                buttons[i][j] = Button(j * length, i * length, length, length, main_color=color)
                buttons[i][j].secondary_color = MARKED_SQUARE

        return buttons
    
    def _get_sprite_paths(self):
        return {
            wrappers.PieceType.WHITE_KNIGHT: WHITE_KNIGHT_P,
            wrappers.PieceType.WHITE_KING:   WHITE_KING_P,
            wrappers.PieceType.WHITE_ROOK:   WHITE_ROOK_P,
            wrappers.PieceType.WHITE_BISHOP: WHITE_BISHOP_P,
            wrappers.PieceType.WHITE_PAWN:   WHITE_PAWN_P,
            wrappers.PieceType.WHITE_QUEEN:  WHITE_QUEEN_P,
            wrappers.PieceType.BLACK_KNIGHT: BLACK_KNIGHT_P,
            wrappers.PieceType.BLACK_KING:   BLACK_KING_P,
            wrappers.PieceType.BLACK_ROOK:   BLACK_ROOK_P,
            wrappers.PieceType.BLACK_BISHOP: BLACK_BISHOP_P,
            wrappers.PieceType.BLACK_PAWN:   BLACK_PAWN_P,
            wrappers.PieceType.BLACK_QUEEN:  BLACK_QUEEN_P,
        }