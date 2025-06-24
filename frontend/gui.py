
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
        self.button_pressed = None

    def draw_board(self):
        self.win.fill((0, 0, 0))

        for row in self.button_board:
            for button in row:
                button.draw(self.win)

        p.display.flip()

    def update_button_board(self, board):
        for i in range(8):
            for j in range(8):
                sprite = None
                piece_type = wrappers.board_get_piece_w(self.chess_lib, i * 8 + j, board)
                if piece_type != -1:
                    path = self.sprite_paths[piece_type]
                    sprite = p.image.load(path)

                
                self.button_board[i][j].set_sprite(sprite)

    def update_event(self):
        self.event = None
        for event in p.event.get():
            if event.type == p.QUIT:
                p.quit()
                quit()

            if event.type == p.MOUSEBUTTONDOWN:
                self.event = event

    def update_button_pressed(self):
        if self.event == None:
            return
        for i in range(8):
            for j in range(8):
                if (self.button_board[i][j].check_if_pressed(self.event.pos[0], self.event.pos[1])):
                    if self.button_pressed == self.button_board[i][j]:
                        self.button_pressed = None
                    else:
                        self.button_pressed = self.button_board[i][j]

        print(self.button_pressed)

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