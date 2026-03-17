import cv2
import numpy as np
import base64
import os
import atexit
import time
from arduino.app_utils import App
from arduino.app_bricks.web_ui import WebUI

# --- Configuration ---
BOARD_SIZE = 400 
CHESSBOARD_SIZE = (7, 7)
TARGET_FPS = 3
FRAME_DELAY = 1.0 / TARGET_FPS

# Global State
locked_outer_pts = None
empty_board_gray = None

# Standard Starting Position
# Lowercase = Black, Uppercase = White, '.' = Empty
virtual_board = [
    ['r','n','b','q','k','b','n','r'],
    ['p','p','p','p','p','p','p','p'],
    ['.','.','.','.','.','.','.','.'],
    ['.','.','.','.','.','.','.','.'],
    ['.','.','.','.','.','.','.','.'],
    ['.','.','.','.','.','.','.','.'],
    ['P','P','P','P','P','P','P','P'],
    ['R','N','B','Q','K','B','N','R']
]

ui = WebUI()

def initialize_camera():
    os.system("fuser -k /dev/video* 2>/dev/null")
    time.sleep(1)
    for index in [0, 1, 2, 3]:
        try:
            cap = cv2.VideoCapture(index, cv2.CAP_V4L2)
            if cap.isOpened():
                cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc(*'MJPG'))
                cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
                cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
                return cap
        except: continue
    return None

camera = initialize_camera()

@atexit.register
def shutdown_handler():
    if camera: camera.release()

# --- Logic: Occupancy & FEN ---

def get_outer_corners(gray, corners):
    criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)
    refined = cv2.cornerSubPix(gray, corners, (11,11), (-1,-1), criteria)
    grid = refined.reshape(7, 7, 2)
    h_step = np.mean(grid[:, 1:] - grid[:, :-1], axis=(0, 1))
    v_step = np.mean(grid[1:, :] - grid[:-1, :], axis=(0, 1))
    return np.array([
        grid[0,0]-h_step-v_step, grid[0,6]+h_step-v_step, 
        grid[6,6]+h_step+v_step, grid[6,0]-h_step+v_step
    ], dtype="float32")

def update_virtual_board(new_occ):
    global virtual_board
    from_sq = None
    to_sq = None
    
    for r in range(8):
        for c in range(8):
            is_virt_occupied = (virtual_board[r][c] != '.')
            # If it was occupied in memory but is now empty on camera
            if is_virt_occupied and new_occ[r][c] == 0:
                from_sq = (r, c)
            # If it was empty in memory but is now occupied on camera
            elif not is_virt_occupied and new_occ[r][c] == 1:
                to_sq = (r, c)
                
    if from_sq and to_sq:
        piece = virtual_board[from_sq[0]][from_sq[1]]
        virtual_board[to_sq[0]][to_sq[1]] = piece
        virtual_board[from_sq[0]][from_sq[1]] = '.'
        return f"Detected Move: {piece} to {chr(97+to_sq[1])}{8-to_sq[0]}"
    return "Board updated. No clear move detected."

def generate_fen():
    fen_rows = []
    for row in virtual_board:
        empty = 0; s = ""
        for char in row:
            if char == '.': empty += 1
            else:
                if empty > 0: s += str(empty); empty = 0
                s += char
        if empty > 0: s += str(empty)
        fen_rows.append(s)
    return "/".join(fen_rows) + " w - - 0 1"

# --- API Functions ---

def get_live_frame():
    time.sleep(1/3) # Force 3 FPS
    success, frame = camera.read()
    if not success: return {"error": "Camera fail"}
    preview = cv2.resize(frame, (320, 240))
    _, buffer = cv2.imencode('.jpg', preview, [int(cv2.IMWRITE_JPEG_QUALITY), 70])
    return {"image": f"data:image/jpeg;base64,{base64.b64encode(buffer).decode('utf-8')}"}

def calibrate_board():
    global locked_outer_pts, empty_board_gray
    success, frame = camera.read()
    if not success: return {"status": "error"}
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    ret, corners = cv2.findChessboardCorners(gray, CHESSBOARD_SIZE, None)
    if ret:
        locked_outer_pts = get_outer_corners(gray, corners)
        M = cv2.getPerspectiveTransform(locked_outer_pts, np.array([[0,0],[400,0],[400,400],[0,400]], dtype="float32"))
        empty_board_gray = cv2.warpPerspective(gray, M, (400, 400))
        return {"status": "success", "message": "Cloth board calibrated! Position locked."}
    return {"status": "fail", "message": "Empty board not found."}

def capture_and_analyze():
    global locked_outer_pts, empty_board_gray
    if locked_outer_pts is None: return {"status": "fail", "message": "Please calibrate first!"}
    
    success, frame = camera.read()
    M = cv2.getPerspectiveTransform(locked_outer_pts, np.array([[0,0],[400,0],[400,400],[0,400]], dtype="float32"))
    warped = cv2.warpPerspective(frame, M, (400, 400))
    
    # Difference analysis
    curr_gray = cv2.cvtColor(warped, cv2.COLOR_BGR2GRAY)
    diff = cv2.absdiff(curr_gray, empty_board_gray)
    _, thresh = cv2.threshold(diff, 35, 255, cv2.THRESH_BINARY)
    
    sq_size = 400 // 8
    new_occ = np.zeros((8, 8), dtype=int)
    for r in range(8):
        for c in range(8):
            mask = thresh[r*sq_size:(r+1)*sq_size, c*sq_size:(c+1)*sq_size]
            if (cv2.countNonZero(mask) / (sq_size**2)) > 0.15:
                new_occ[r, c] = 1
                
    move_msg = update_virtual_board(new_occ)
    fen = generate_fen()
    
    _, buffer = cv2.imencode('.jpg', warped)
    return {
        "status": "success",
        "message": move_msg,
        "fen": fen,
        "image": f"data:image/jpeg;base64,{base64.b64encode(buffer).decode('utf-8')}"
    }

# --- Registration (Manual to avoid Decorator Error) ---
ui.expose_api("GET", "/stream", get_live_frame)
ui.expose_api("GET", "/calibrate", calibrate_board)
ui.expose_api("GET", "/capture", capture_and_analyze)

print(f"Server starting on Uno Q. URL: {ui.local_url}")
App.run()