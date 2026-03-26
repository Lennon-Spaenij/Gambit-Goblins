import chess # pip install python-chess
import chess.engine
from stockfish import Stockfish # pip install python-chess stockfish

# Path to Stockfish executable
# sf = Stockfish(path="C:\\Users\\leonk\\Downloads\\stockfish-windows-x86-64-avx2\\stockfish\\stockfish-windows-x86-64-avx2.exe")
stockfish_path="C:\\Users\\leonk\\Downloads\\stockfish-windows-x86-64-avx2\\stockfish\\stockfish-windows-x86-64-avx2.exe"

# Initialize
board = chess.Board()
engine = chess.engine.SimpleEngine.popen_uci(stockfish_path)

# FEN
current_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
new_fen = ""   

print("Terminal chess VS Stockfish\n")
print(f"Current FEN: {current_fen}")
print("Please choose a color (black/white)")
color_assignment = input("> ").strip().lower()

print(board, "\n")

while not board.is_game_over():
    if color_assignment == "black": # If you chose to play with black 
        if board.turn == chess.WHITE:
            result = engine.play(board, chess.engine.Limit(time=2.0))
            board.push(result.move)
            print(f"Stockfish plays: {result.move}")
            new_fen = board.fen()
        else:
            print("Your move (e.g., 'e2e4' or 'g1f3'):")
            move_str = input("> ").strip()
            try: 
                move = board.parse_uci(move_str) # uci like 'g1f3' Knight to f3
                board.push(move)
                new_fen = board.fen()
            except:
                print("Invalid move")
                continue
    else: # You chose white
        if board.turn == chess.WHITE:
            print("Your move (e.g., 'e2e4' or 'g1f3'):")
            move_str = input("> ").strip()
            try: 
                move = board.parse_uci(move_str) # uci like 'g1f3' Knight to f3
                board.push(move)
                new_fen = board.fen()
            except:
                print("Invalid move")
                continue
        else: # Stockfish turn
            result = engine.play(board, chess.engine.Limit(time=2.0))
            uci = result.move.uci()
            split_uci = len(uci) // 2
            uci_sq1, uci_sq2 = uci[:split_uci], uci[split_uci:]
            square1 = chess.parse_square(uci_sq1)
            square2 = chess.parse_square(uci_sq2)
            piece_sq1 = board.piece_type_at(square1)
            piece_sq2 = board.piece_type_at(square2)

            board.push(result.move)
            print(f"Stockfish plays: {result.move}")
            new_fen = board.fen()
            print(f"FEN: {board.fen()}\n")
            print(uci_sq1, uci_sq2, piece_sq1, piece_sq2)

    if current_fen != new_fen:
        current_fen = new_fen
        print(f"Updated FEN: {current_fen}")

    print("\n" + str(board) + "\n")
    print(f"FEN: {board.fen()}\n")

print("Game over:", board.result())
engine.quit()