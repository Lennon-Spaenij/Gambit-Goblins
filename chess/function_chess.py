import chess # pip install python-chess
import chess.engine
from stockfish import Stockfish # pip install python-chess stockfish

# Path to Stockfish executable
stockfish_path="C:\\Users\\leonk\\Downloads\\stockfish-windows-x86-64-avx2\\stockfish\\stockfish-windows-x86-64-avx2.exe"

# Initialize
board = chess.Board()
engine = chess.engine.SimpleEngine.popen_uci(stockfish_path)

# FEN
current_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

def get_board_state():
    print("Put the camera code here")

def fen_to_uci(fen_camera):
    global current_fen
    new_fen = fen_camera

    curr_board = chess.Board(current_fen)
    new_board = chess.Board(new_fen)

    for move in curr_board.legal_moves:
        temp_board = curr_board.copy()
        temp_board.push(move)

        if temp_board.fen() == new_board.fen():
            uci_move = move.uci()
            current_fen = new_fen
            return uci_move
    print("No valid move detected from FEN")
    return None

def human_move(uci_move):
    # Get the uci from the picture to tell the engine what move has been made
    try:
        move = board.parse_uci(uci_move) # UCI like 'g1f3' Knight to f3
        board.push(move)
        return True
    except:
        print("Invalid move")
        return False

def robot_move():
    # Give a UCI to tell the arm where to go to make the move it wants
    result = engine.play(board, chess.engine.Limit(time=2.0))
    board.push(result.move)
    uci = result.move.uci()
    print(f"FEN: {board.fen()}\n")
    return uci 

def main_game_loop():
    while not board.is_game_over():
        # 1: Get camera FEN
        camera_fen = get_board_state()

        # 2: Make the fen into UCI
        human_uci = fen_to_uci(camera_fen)
        if human_uci and human_move(human_uci):
            # 3: stockfishes reply
            robot_uci = robot_move()

            # 4: Give the UCI to the 'arm' so it can move to the right coords
        else:
            print("Waiting for valid move")

    print("Game over:", board.result())
    engine.quit()

if __name__ == '__main__':
    main_game_loop()