import time
import sys
from chain_reaction_game import Board, RED, BLUE, EMPTY
from chain_reaction_ai import find_best_move, heuristic_balanced_control,heuristic_orb_difference,heuristic_critical_mass_advantage,heuristic_control_value,heuristic_conversion_potential
from chain_reaction_protocol import read_game_state_file, write_game_state_file, wait_for_move, HUMAN_MOVE_HEADER, AI_MOVE_HEADER

AI_PLAYER_COLOR = BLUE 
HUMAN_PLAYER_COLOR = RED 
AI_DEPTH_LIMIT = 2

def main():
    print("AI Game Engine Started.")
    current_board = Board() 
    cnt = 0
    while True:
        header_type, board_from_file = read_game_state_file()
        if header_type != HUMAN_MOVE_HEADER:
            if not wait_for_move(HUMAN_MOVE_HEADER, current_board):
                break 
        cnt += 1
        header_type, board_from_file = read_game_state_file() 
        if board_from_file:
            current_board.board = board_from_file.board 
        print("received human board configuration")
        current_board.print_board()
        if cnt >= 2:
            winner = current_board.check_win_condition()
            if winner:
                print(f"AI Engine: {winner} won! Game over.")
                break

        print(f"\nAI Engine: AI ({AI_PLAYER_COLOR}) is thinking... (Depth: {AI_DEPTH_LIMIT})")
        start_time = time.time()
        best_move = find_best_move(current_board, AI_PLAYER_COLOR, AI_DEPTH_LIMIT, heuristic_control_value)
        end_time = time.time()
        print(f"AI Engine: Thinking time: {end_time - start_time:.2f} seconds.")

        if best_move is None:
            print("AI Engine: No legal moves for AI. Game might be stuck or over unexpectedly.")
            break
        
        ai_r, ai_c = best_move
        print(f"AI Engine: AI chooses to place at ({ai_r}, {ai_c})")
        ai_next_board = current_board.copy()
        ai_next_board.place_orb(ai_r, ai_c, AI_PLAYER_COLOR)
        print("current board configuration: ")
        ai_next_board.print_board()
        write_game_state_file(AI_MOVE_HEADER, ai_next_board)
        current_board.board = ai_next_board.board 
        winner = current_board.check_win_condition()
        if winner:
            print(f"AI Engine: {winner} won! Game over.")
            break

    print("AI Game Engine Exited.")

if __name__ == "__main__":
    main()