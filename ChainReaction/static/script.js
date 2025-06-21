// static/script.js

const BOARD_ROWS = 9; // Must match chain_reaction_game.py
const BOARD_COLS = 6; // Must match chain_reaction_game.py
const CELL_SIZE = 60; // Must match style.css

const gameBoardElement = document.getElementById('game-board');
const statusMessageElement = document.getElementById('status-message');
const resetButton = document.getElementById('reset-button');

let isGameOver = false;
let isWaitingForAI = false;

// Function to create the initial board grid in HTML
function createBoardGrid() {
    gameBoardElement.style.gridTemplateColumns = `repeat(${BOARD_COLS}, ${CELL_SIZE}px)`;
    gameBoardElement.style.gridTemplateRows = `repeat(${BOARD_ROWS}, ${CELL_SIZE}px)`;

    for (let r = 0; r < BOARD_ROWS; r++) {
        for (let c = 0; c < BOARD_COLS; c++) {
            const cell = document.createElement('div');
            cell.classList.add('cell');
            cell.dataset.row = r;
            cell.dataset.col = c;
            cell.addEventListener('click', () => handleCellClick(r, c));
            gameBoardElement.appendChild(cell);
        }
    }
}

// Function to update the board display based on received game state
function updateBoardDisplay(boardData) {
    for (let r = 0; r < BOARD_ROWS; r++) {
        for (let c = 0; c < BOARD_COLS; c++) {
            const cell = gameBoardElement.querySelector(`.cell[data-row="${r}"][data-col="${c}"]`);
            const { count, color, critical_mass } = boardData[r][c]; // Get critical_mass from backend

            cell.textContent = count > 0 ? count : ''; // Don't show '0' for empty cells
            cell.className = 'cell'; // Reset classes for dynamic updates

            if (isGameOver) {
                cell.classList.add('game-over-cell'); // Disable clicks visually
            } else {
                 cell.classList.remove('game-over-cell');
            }

            if (color === 'R') {
                cell.classList.add('red-player');
            } else if (color === 'B') {
                cell.classList.add('blue-player');
            }

            // Highlight cells that are at critical mass
            if (count >= critical_mass && color !== 'E') {
                cell.classList.add('critical-mass');
            }
        }
    }
}

// Function to fetch game state from the backend
async function fetchGameState() {
    try {
        const response = await fetch('/game_state');
        const data = await response.json();

        updateBoardDisplay(data.board);
        statusMessageElement.textContent = data.status_message;

        isGameOver = data.game_over;
        isWaitingForAI = data.waiting_for_ai;

        if (isGameOver) {
            // Game ended, no more polling for moves
            console.log("Game Over. Winner:", data.winner);
        } else if (isWaitingForAI) {
            // Still waiting for AI, keep polling
            setTimeout(fetchGameState, 1000); // Poll every 1 second
        } else {
            // It's human's turn, clicks are enabled by default
        }
    } catch (error) {
        console.error('Error fetching game state:', error);
        statusMessageElement.textContent = 'Error loading game. Please refresh or check server.';
    }
}

// Function to handle cell clicks by sending move to backend
async function handleCellClick(row, col) {
    if (isGameOver) {
        alert("The game is over! Please reset to play again.");
        return;
    }
    if (isWaitingForAI) {
        alert("It's AI's turn. Please wait.");
        return;
    }

    try {
        const response = await fetch('/make_move', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({ row, col }),
        });

        const result = await response.json();
        if (result.success) {
            console.log("Move sent, waiting for AI...");
            // Immediately update the display with human's move (as processed by backend)
            // and then start polling for AI's move
            fetchGameState();
        } else {
            alert(result.message);
        }
    } catch (error) {
        console.error('Error making move:', error);
        alert('An error occurred while making your move. Check console for details.');
    }
}

// Function to handle game reset
async function handleResetGame() {
    if (!confirm("Are you sure you want to reset the game? This will start a new game.")) {
        return;
    }
    try {
        const response = await fetch('/reset_game', {
            method: 'POST',
        });
        const result = await response.json();
        if (result.success) {
            alert("Game has been reset!");
            // After reset, fetch the new initial state
            fetchGameState();
        } else {
            alert("Failed to reset game: " + result.message);
        }
    } catch (error) {
        console.error('Error resetting game:', error);
        alert('An error occurred during game reset. Check console for details.');
    }
}

// Initial setup when the page loads
document.addEventListener('DOMContentLoaded', () => {
    createBoardGrid();
    fetchGameState(); // Start the game state polling loop
    resetButton.addEventListener('click', handleResetGame);
});