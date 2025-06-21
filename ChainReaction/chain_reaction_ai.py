import math
from chain_reaction_game import Board, RED, BLUE, EMPTY, BOARD_ROWS, BOARD_COLS
def heuristic_orb_difference(board: Board, player_color):
    ai_orbs = 0
    opponent_orbs = 0
    opponent_color = RED if player_color == BLUE else BLUE

    for r in range(BOARD_ROWS):
        for c in range(BOARD_COLS):
            count, color = board.board[r][c]
            if color == player_color:
                ai_orbs += count
            elif color == opponent_color:
                opponent_orbs += count
    return ai_orbs - opponent_orbs

def heuristic_critical_mass_advantage(board: Board, player_color):
    score = 0
    opponent_color = RED if player_color == BLUE else BLUE

    for r in range(BOARD_ROWS):
        for c in range(BOARD_COLS):
            count, color = board.board[r][c]
            critical_mass = board.get_critical_mass(r, c)

            if color == player_color:
                if count > 0:
                    score += 1 
                if count == critical_mass - 1: 
                    score += 4 
                elif count >= critical_mass:
                    score += 8
            elif color == opponent_color:
                if count > 0:
                    score -= 1
                if count == critical_mass - 1:
                    score -= 4
                elif count >= critical_mass:
                    score -= 8
            elif color == EMPTY:
                score += 0.1 

    return score

def heuristic_control_value(board: Board, player_color):
    score = 0
    opponent_color = RED if player_color == BLUE else BLUE

    for r in range(BOARD_ROWS):
        for c in range(BOARD_COLS):
            count, color = board.board[r][c]
            critical_mass = board.get_critical_mass(r, c)
            cell_value = 0

            if critical_mass == 2: # Corner cell
                cell_value = 3
            elif critical_mass == 3: # Edge cell (not corner)
                cell_value = 2
            else: # Middle cell
                cell_value = 1

            if color == player_color:
                score += cell_value * count # Value based on orbs and cell type
            elif color == opponent_color:
                score -= cell_value * count # Penalize opponent's control

    return score

def heuristic_conversion_potential(board: Board, player_color):
    score = 0
    opponent_color = RED if player_color == BLUE else BLUE

    for r in range(BOARD_ROWS):
        for c in range(BOARD_COLS):
            count, color = board.board[r][c]
            critical_mass = board.get_critical_mass(r, c)

            if color == player_color:
                if count == critical_mass - 1: 
                    for nr, nc in board.get_neighbors(r, c):
                        n_count, n_color = board.board[nr][nc]
                        if n_color == opponent_color:
                            score += n_count 
                            score += 1 
            elif color == opponent_color:
                if count == critical_mass - 1:
                    for nr, nc in board.get_neighbors(r, c):
                        n_count, n_color = board.board[nr][nc]
                        if n_color == player_color:
                            score -= n_count
                            score -= 1
    return score

def heuristic_balanced_control(board: Board, player_color):
    h1_val = heuristic_orb_difference(board, player_color)
    h2_val = heuristic_critical_mass_advantage(board, player_color)
    h3_val = heuristic_control_value(board, player_color)
    h4_val = heuristic_conversion_potential(board, player_color)
    score = (h1_val * 0.8) + (h2_val * 0.7) + (h3_val * 0.1) + (h4_val * 0.7)
    return score

def minimax_alpha_beta(board: Board, depth, maximizing_player_color, alpha, beta, depth_limit, heuristic_func):
    current_player_color = RED if (depth % 2 == 0) == (maximizing_player_color == RED) else BLUE
    winner = board.check_win_condition()
    if winner:
        if winner == maximizing_player_color:
            return math.inf - depth 
        else:
            return -math.inf + depth 
    if depth == depth_limit:
        return heuristic_func(board, maximizing_player_color)
    legal_moves = board.get_legal_moves(current_player_color)
    if not legal_moves: 
        return 0 
    if current_player_color == maximizing_player_color: 
        max_eval = -math.inf
        for r, c in legal_moves:
            new_board = board.copy()
            new_board.place_orb(r, c, current_player_color) 
            eval = minimax_alpha_beta(new_board, depth + 1, maximizing_player_color, alpha, beta, depth_limit, heuristic_func)
            max_eval = max(max_eval, eval)
            alpha = max(alpha, eval)
            if beta <= alpha: 
                break
        return max_eval
    else: 
        min_eval = math.inf
        for r, c in legal_moves:
            new_board = board.copy()
            new_board.place_orb(r, c, current_player_color) 
            eval = minimax_alpha_beta(new_board, depth + 1, maximizing_player_color, alpha, beta, depth_limit, heuristic_func)
            min_eval = min(min_eval, eval)
            beta = min(beta, eval)
            if beta <= alpha: 
                break
        return min_eval

def find_best_move(board: Board, ai_player_color, depth_limit, heuristic_func=heuristic_orb_difference):
    best_value = -math.inf
    best_move = None
    
    legal_moves = board.get_legal_moves(ai_player_color)
    if not legal_moves:
        return None
    for r, c in legal_moves:
        new_board = board.copy()
        new_board.place_orb(r, c, ai_player_color)
        move_value = minimax_alpha_beta(new_board, 1, ai_player_color, -math.inf, math.inf, depth_limit, heuristic_func)
        if move_value > best_value:
            best_value = move_value
            best_move = (r, c)
            
    return best_move