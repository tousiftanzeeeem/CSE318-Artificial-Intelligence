from flask import Flask, render_template, request, jsonify
import threading
import time
import os
import sys
from chain_reaction_game import Board, RED, BLUE, EMPTY, BOARD_ROWS, BOARD_COLS
from chain_reaction_protocol import read_game_state_file, write_game_state_file, \
                                    HUMAN_MOVE_HEADER, AI_MOVE_HEADER, GAMESTATE_FILE

app = Flask(__name__)

HUMAN_PLAYER_COLOR = RED 
AI_PLAYER_COLOR = BLUE   

current_game_board = Board()
current_player_turn = HUMAN_PLAYER_COLOR # Human (Red) always starts
waiting_for_ai_move = False
game_over_flag = False
winner_color = None
cnt=0
# --- Helper Functions ---
def initialize_game_state_file():
    cnt=0
    global current_game_board, current_player_turn, waiting_for_ai_move, game_over_flag, winner_color

    if os.path.exists(GAMESTATE_FILE):
        try:
            os.remove(GAMESTATE_FILE)
            print(f"[*] Backend: Removed existing {GAMESTATE_FILE} to start fresh.")
        except OSError as e:
            print(f"[*] Backend: Error removing {GAMESTATE_FILE}: {e}", file=sys.stderr)
            sys.exit(1)
    time.sleep(0.5)
    current_game_board = Board()
    current_player_turn = HUMAN_PLAYER_COLOR
    waiting_for_ai_move = False
    game_over_flag = False
    winner_color = None
    print("[*] Backend: Game state initialized.")

def check_for_ai_move_in_background():
    global current_game_board, current_player_turn, waiting_for_ai_move, game_over_flag, winner_color

    print("[*] Backend: AI move polling thread started.")
    while True:
        if game_over_flag:
            # print("[*] Backend: Game over, stopping AI polling thread.")
            break # Stop checking if game is over

        # Only check for AI's move if we are actively waiting for it
        if waiting_for_ai_move:
            header_type, board_from_file = read_game_state_file()

            if header_type == AI_MOVE_HEADER and board_from_file:
                # AI has made a move
                print("[*] Backend: AI move detected in file.")
                current_game_board.board = board_from_file.board # Update server's board state
                waiting_for_ai_move = False
                current_player_turn = HUMAN_PLAYER_COLOR # Switch back to human

                # --- IMPORTANT: Check for AI win here! ---
                winner = current_game_board.check_win_condition()
                if winner:
                    game_over_flag = True
                    winner_color = winner
                    print(f"[*] Backend: Game Over. Winner: {winner_color} (AI's winning move).")
                else:
                    print("[*] Backend: AI move processed. Human's turn.")
            # else:
                # print("[*] Backend: Still waiting for AI move or file doesn't exist/invalid header.")

        time.sleep(0.5) # Check every 500 ms

# --- Flask Routes ---

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/game_state', methods=['GET'])
def get_game_state():
    board_data = []
    for r in range(BOARD_ROWS):
        row_data = []
        for c in range(BOARD_COLS):
            count, color = current_game_board.board[r][c]
            critical_mass = current_game_board.get_critical_mass(r, c)
            row_data.append({'count': count, 'color': color, 'critical_mass': critical_mass})
        board_data.append(row_data)

    status_message = ""
    if game_over_flag:
        status_message = f"🎉🎉🎉 {winner_color} WINS! Game Over! 🎉🎉🎉"
    elif waiting_for_ai_move:
        status_message = "AI is thinking..."
    elif current_player_turn == HUMAN_PLAYER_COLOR:
        status_message = f"Your turn ({HUMAN_PLAYER_COLOR})! Click a cell to place an orb."
    else:
        status_message = "Waiting for AI..." # Fallback, should align with waiting_for_ai_move

    return jsonify({
        'board': board_data,
        'current_player': current_player_turn,
        'waiting_for_ai': waiting_for_ai_move,
        'game_over': game_over_flag,
        'winner': winner_color,
        'status_message': status_message
    })

@app.route('/make_move', methods=['POST'])
def make_move():
    global current_game_board, current_player_turn, waiting_for_ai_move, game_over_flag, winner_color,cnt

    if game_over_flag:
        return jsonify({'success': False, 'message': f"Game over! {winner_color} won."}), 400
    if waiting_for_ai_move:
        return jsonify({'success': False, 'message': "It's AI's turn. Please wait."}), 400
    if current_player_turn != HUMAN_PLAYER_COLOR:
        return jsonify({'success': False, 'message': "It's not your turn."}), 400

    data = request.get_json()
    r = data.get('row')
    c = data.get('col')

    # Validate input
    if not isinstance(r, int) or not isinstance(c, int) or \
       not (0 <= r < BOARD_ROWS) or not (0 <= c < BOARD_COLS):
        return jsonify({'success': False, 'message': "Invalid row or column."}), 400

    temp_board = current_game_board.copy()
    if not temp_board.place_orb(r, c, HUMAN_PLAYER_COLOR):
        return jsonify({'success': False, 'message': "You cannot place an orb here. Cell is occupied by opponent or invalid."}), 400

    # If move is valid, apply it to the main game board
    current_game_board.board = temp_board.board
    print(f"[*] Backend: Human attempting to write move ({r},{c}) to file.")
    write_game_state_file(HUMAN_MOVE_HEADER, current_game_board)
    cnt+=1
    # Check for immediate win after human's move (e.g., if human cleared AI orbs)
    if cnt>=2:
        winner = current_game_board.check_win_condition()
        if winner:
            game_over_flag = True
            winner_color = winner
            print(f"[*] Backend: Human won immediately. Winner: {winner_color}")
            return jsonify({'success': True, 'game_over': True, 'winner': winner_color})

    # Switch to AI's turn and set flag
    current_player_turn = AI_PLAYER_COLOR
    waiting_for_ai_move = True
    print(f"[*] Backend: Human placed at ({r}, {c}). Now waiting for AI.")

    return jsonify({'success': True, 'message': 'Move made, waiting for AI.'})

@app.route('/reset_game', methods=['POST'])
def reset_game():
    global cnt
    cnt = 0
    initialize_game_state_file()
    get_game_state()
    ai_poll_thread = threading.Thread(target=check_for_ai_move_in_background, daemon=True)
    ai_poll_thread.start()
    print("[*] Backend: Game reset via API.")
    return jsonify({'success': True, 'message': 'Game reset successfully.'})

if __name__ == '__main__':
    initialize_game_state_file()
    ai_poll_thread = threading.Thread(target=check_for_ai_move_in_background, daemon=True)
    ai_poll_thread.start()

    print("\n" + "="*50)
    print("  Chain Reaction Web Server Started!")
    print("  Open your browser to http://127.0.0.1:5000/")
    print("  Run 'python ai_player.py' in a separate terminal.")
    print("="*50 + "\n")

    # Run the Flask app
    # debug=True allows for automatic reloading on code changes, but should be False in production
    app.run(debug=True, port=5000)