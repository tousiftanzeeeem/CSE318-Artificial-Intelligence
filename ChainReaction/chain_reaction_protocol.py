import time
import copy
import sys
from chain_reaction_game import Board, BOARD_ROWS, BOARD_COLS, RED, BLUE, EMPTY

GAMESTATE_FILE = 'gamestate.txt'
HUMAN_MOVE_HEADER = "Human Move:"
AI_MOVE_HEADER = "AI Move:"

def read_game_state_file():
    try:
        with open(GAMESTATE_FILE, 'r') as f:
            lines = f.readlines()
        if not lines:
            return None, None 

        header_line = lines[0].strip()
        board_str_data = "".join(lines[1:]) 

        header_type = None
        if header_line == HUMAN_MOVE_HEADER:
            header_type = HUMAN_MOVE_HEADER
        elif header_line == AI_MOVE_HEADER:
            header_type = AI_MOVE_HEADER
        else:
            print(f"Protocol Error: Unknown header '{header_line}'", file=sys.stderr)
            return None, None 
        board = Board.from_string(board_str_data)
        return header_type, board

    except FileNotFoundError:
        return None, None 
    except Exception as e:
        print(f"Error reading gamestate.txt: {e}", file=sys.stderr)
        return None, None

def write_game_state_file(header_type, board_object: Board):
    try:
        with open(GAMESTATE_FILE, 'w') as f:
            f.write(header_type + "\n")
            f.write(board_object.to_string() + "\n")
        print(f"[*] Wrote {header_type} to {GAMESTATE_FILE}")
    except Exception as e:
        print(f"Error writing gamestate.txt: {e}", file=sys.stderr)

def wait_for_move(expected_header_type, current_board: Board):
    print(f"[*] Waiting for {expected_header_type}...")
    while True:
        header_type, board_from_file = read_game_state_file()
        
        if header_type == expected_header_type:
            current_board.board = copy.deepcopy(board_from_file.board)
            print(f"[*] Received {expected_header_type}.")
            return True 
        time.sleep(0.5) 

