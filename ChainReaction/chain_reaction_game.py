import copy
import sys

RED = 'R'
BLUE = 'B'
EMPTY = '0'

BOARD_ROWS = 9
BOARD_COLS = 6

class Board:
    def __init__(self):
        self.board = [[[0, EMPTY] for _ in range(BOARD_COLS)] for _ in range(BOARD_ROWS)]
    def print_board(self):
        for row in self.board:
            print(" ".join(f"({count},{color})" for count, color in row))

    def is_valid_coord(self, r, c):
        return 0 <= r < BOARD_ROWS and 0 <= c < BOARD_COLS

    def get_critical_mass(self, r, c):
        if not self.is_valid_coord(r, c):
            return 0 

        neighbors = 0
        if r > 0: neighbors += 1 # Up
        if r < BOARD_ROWS - 1: neighbors += 1 # Down
        if c > 0: neighbors += 1 # Left
        if c < BOARD_COLS - 1: neighbors += 1 # Right
        return neighbors

    def get_neighbors(self, r, c):
        neighbors = []
        if r > 0: neighbors.append((r - 1, c))
        if r < BOARD_ROWS - 1: neighbors.append((r + 1, c))
        if c > 0: neighbors.append((r, c - 1))
        if c < BOARD_COLS - 1: neighbors.append((r, c + 1))
        return neighbors

    def _explode_cell(self, r, c, exploding_color):
        self.board[r][c] = [0, EMPTY]
        for nr, nc in self.get_neighbors(r, c):
            current_count, current_color = self.board[nr][nc]
            if current_color != EMPTY and current_color != exploding_color:
                current_color = exploding_color 

            new_count = current_count + 1
            self.board[nr][nc] = [new_count, exploding_color]

            if new_count >= self.get_critical_mass(nr, nc):
                self._explode_cell(nr, nc, exploding_color)

    def place_orb(self, r, c, player_color):
        if not self.is_valid_coord(r, c):
            return False 

        current_count, current_color = self.board[r][c]
        if current_color != EMPTY and current_color != player_color:
            return False 
        self.board[r][c][0] += 1
        self.board[r][c][1] = player_color 

        if self.board[r][c][0] >= self.get_critical_mass(r, c):
            self._explode_cell(r, c, player_color) 
        return True 
    def check_win_condition(self):
        red_orbs_present = False
        blue_orbs_present = False

        for r in range(BOARD_ROWS):
            for c in range(BOARD_COLS):
                orb_count, orb_color = self.board[r][c]
                if orb_color == RED:
                    red_orbs_present = True
                elif orb_color == BLUE:
                    blue_orbs_present = True

        if red_orbs_present and not blue_orbs_present:
            return RED
        elif blue_orbs_present and not red_orbs_present:
            return BLUE
        else:
            return None 

    def get_legal_moves(self, player_color):

        legal_moves = []
        for r in range(BOARD_ROWS):
            for c in range(BOARD_COLS):
                orb_count, orb_color = self.board[r][c]
                if orb_color == EMPTY or orb_color == player_color:
                    legal_moves.append((r, c))
        return legal_moves

    def copy(self):
        new_board = Board()
        new_board.board = copy.deepcopy(self.board)
        return new_board

    def to_string(self):
        board_str = []
        for r in range(BOARD_ROWS):
            row_str = []
            for c in range(BOARD_COLS):
                count, color = self.board[r][c]
                if count == 0:
                    row_str.append(f"{count}{EMPTY}") # "00" for empty cell
                else:
                    row_str.append(f"{count}{color}")
            board_str.append(" ".join(row_str))
        return "\n".join(board_str)

    @classmethod
    def from_string(cls, board_str_data):
        new_board = cls()
        rows = board_str_data.strip().split('\n')
        if len(rows) != BOARD_ROWS:
            raise ValueError(f"Board string has {len(rows)} rows, expected {BOARD_ROWS}")

        for r, row_str in enumerate(rows):
            cells = row_str.strip().split(' ')
            if len(cells) != BOARD_COLS:
                raise ValueError(f"Row {r} has {len(cells)} cells, expected {BOARD_COLS}")
            for c, cell_val in enumerate(cells):
                if cell_val == "00":
                    new_board.board[r][c] = [0, EMPTY]
                elif len(cell_val) == 2:
                    count = int(cell_val[0])
                    color = cell_val[1]
                    if color not in [RED, BLUE, EMPTY]:
                        raise ValueError(f"Invalid color '{color}' at ({r},{c})")
                    new_board.board[r][c] = [count, color]
                else:
                    raise ValueError(f"Invalid cell format '{cell_val}' at ({r},{c})")
        return new_board
