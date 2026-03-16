import chess # pip install python-chess
import chess.engine
from stockfish import Stockfish # pip install python-chess stockfish

# Path to Stockfish executable
# sf = Stockfish(path="C:\\Users\\leonk\\Downloads\\stockfish-windows-x86-64-avx2\\stockfish\\stockfish-windows-x86-64-avx2.exe")
stockfish_path="C:\\Users\\leonk\\Downloads\\stockfish-windows-x86-64-avx2\\stockfish\\stockfish-windows-x86-64-avx2.exe"

# Initialize
board = chess.Board()
engine = chess.engine.SimpleEngine.popen_uci(stockfish_path)

print("Terminal chess VS Stockfish\n")

print("Please choose a color (black/white)")
color_assignment = input("> ").strip().lower()

print(board, "\n")

while not board.is_game_over():
    if color_assignment == "black": # If you chose to play with black 
        if board.turn == chess.WHITE:
            result = engine.play(board, chess.engine.Limit(time=2.0))
            board.push(result.move)
            print(f"Stockfish plays: {result.move}")
        else:
            print("Your move (e.g., 'e2e4' or 'Nf3'):")
            move_str = input("> ").strip()
            try: 
                move = board.parse_san(move_str) # SAN like 'Nf3' Knight to f3
                board.push(move)
            except:
                print("Invalid move")
                continue
    else: # You chose white
        if board.turn == chess.WHITE:
            print("Your move (e.g., 'e2e4' or 'Nf3'):")
            move_str = input("> ").strip()
            try: 
                move = board.parse_san(move_str) # SAN like 'Nf3' Knight to f3
                board.push(move)
            except:
                print("Invalid move")
                continue
        else: # Stockfish turn
            result = engine.play(board, chess.engine.Limit(time=2.0))
            board.push(result.move)
            print(f"Stockfish plays: {result.move}")

    print("\n" + str(board) + "\n")
    print(f"FEN: {board.fen()}\n")

print("Game over:", board.result())
engine.quit()