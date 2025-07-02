
import pygame as p
from button import Button
import os
from c_lib import wrappers
import time

SCREEN_HEIGHT = 600
SCREEN_LENGTH = 600

LIGHT_SQUARE = (230, 230, 250)
DARK_SQUARE = (70, 130, 180)
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
        self.square_size = min(SCREEN_LENGTH, SCREEN_HEIGHT) / 8
        self.move_stack = []

    def update_board(self, board, attack_table):
        legal_moves, _ = wrappers.get_legal_moves_w(self.chess_lib, board, attack_table)
        wrappers.board_change_turn(self.chess_lib, board)
        legal_opponent_moves, _ = wrappers.get_legal_moves_w(self.chess_lib, board, attack_table)
        wrappers.board_change_turn(self.chess_lib, board)

        #legal_moves.extend(legal_opponent_moves)

        self.marked_squares = set()

        if self.prev_active_square != None and self.active_square != None:
            from_index = self.prev_active_square
            to_index = self.active_square
            for legal_move in legal_moves:
                if (from_index == legal_move.from_index and to_index == legal_move.to_index):
                    wrappers.board_make_move(self.chess_lib, legal_move, board)
                    wrappers.board_change_turn(self.chess_lib, board)
                    self.move_stack.append(legal_move)
                    break
            else:
                move = wrappers.Move()
                move.from_index = from_index
                move.to_index = to_index
                wrappers.board_make_move(self.chess_lib, move, board)
                wrappers.board_change_turn(self.chess_lib, board)
                self.move_stack.append(move)

            self.active_square = None
            self.prev_active_square = None

        elif self.active_square != None:
            self.marked_squares.add(self.active_square)
            from_index = self.active_square
            for move in legal_moves:
                if move.from_index == from_index:
                    self.marked_squares.add(move.to_index)

    def update_pop_push_move(self, board):
        if self.event is None:
            return False

        if self.event.type == p.KEYDOWN:
            if self.event.key == p.K_LEFT:
                if len(self.move_stack) == 0:
                    return False

                move = self.move_stack.pop()
                wrappers.board_unmake_move(self.chess_lib, move, board)
                wrappers.board_change_turn(self.chess_lib, board)
                return True

        return False


    def draw_board(self):
        self.win.fill((0, 0, 0))

        for button in self.button_board:
            button.draw(self.win)

        p.display.flip()

    def update_button_board(self, board):
        for i in range(64):
            sprite = None
            piece_type = wrappers.board_get_piece_w(self.chess_lib, i , board)
            if piece_type != -1:
                path = self.sprite_paths[piece_type]
                sprite = p.transform.scale(p.image.load(path), (self.square_size, self.square_size))


            self.button_board[i].dispay_secondary = False
            if i in self.marked_squares:
                self.button_board[i].dispay_secondary = True

            self.button_board[i].set_sprite(sprite)

    def update_event(self):
        self.event = None
        for event in p.event.get():
            if event.type == p.QUIT:
                p.quit()
                quit()

            if event.type == p.MOUSEBUTTONDOWN or event.type == p.KEYDOWN:
                self.event = event

    def update_active_square(self):
        if self.event == None or self.event.type != p.MOUSEBUTTONDOWN:
            return False
        
        for i in range(64):
            if (self.button_board[i].check_if_pressed(self.event.pos[0], self.event.pos[1])):
                if self.active_square == i:
                    self.prev_active_square = None
                    self.active_square = None
                else:
                    self.prev_active_square = self.active_square
                    self.active_square = i
                    #print("Active square:", self.active_square)
                return True
        return False
    
    def _get_button_board(self):
        buttons = [None for i in range(64)]
        length = min(SCREEN_LENGTH, SCREEN_HEIGHT) / 8
        
        for i in range(64):
            rank = i // 8
            file = i % 8

            color = LIGHT_SQUARE if (rank + file) % 2 == 0 else DARK_SQUARE
            total_length = length * 7

            buttons[i] = Button(file * length, total_length - rank * length, length, length, main_color=color)
            buttons[i].secondary_color = MARKED_SQUARE

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