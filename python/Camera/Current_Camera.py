import cv2
import numpy as np
import base64
import os
import atexit
import time
from arduino.app_utils import App
from arduino.app_bricks.web_ui import WebUI

# Target warped size for 8x8 grid analysis
BOARD_SIZE = 400 
CHESSBOARD_SIZE = (7, 7)

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

def get_outer_corners(gray, corners):
    """
    Takes the 7x7 inner corners and extrapolates the outer 4 corners 
    of the full 8x8 board grid.
    """
    # Refine corner accuracy for better math
    criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)
    refined_corners = cv2.cornerSubPix(gray, corners, (11,11), (-1,-1), criteria)
    
    grid = refined_corners.reshape(7, 7, 2)

    # Calculate the average width and height of a single square
    horiz_step = np.mean(grid[:, 1:] - grid[:, :-1], axis=(0, 1))
    vert_step = np.mean(grid[1:, :] - grid[:-1, :], axis=(0, 1))

    # Extrapolate from the inner grid [1:8, 1:8] to the full grid [0:9, 0:9]
    # Top-Left (from grid[0,0] which is internal corner 1,1)
    top_left = grid[0, 0] - horiz_step - vert_step
    # Top-Right
    top_right = grid[0, 6] + horiz_step - vert_step
    # Bottom-Right
    bottom_right = grid[6, 6] + horiz_step + vert_step
    # Bottom-Left
    bottom_left = grid[6, 0] - horiz_step + vert_step

    return np.array([top_left, top_right, bottom_right, bottom_left], dtype="float32")

def analyze_occupancy(warped):
    gray = cv2.cvtColor(warped, cv2.COLOR_BGR2GRAY)
    sq_size = BOARD_SIZE // 8
    occupancy = np.zeros((8, 8), dtype=int)
    overlay = warped.copy()

    for r in range(8):
        for c in range(8):
            y1, y2 = r * sq_size, (r + 1) * sq_size
            x1, x2 = c * sq_size, (c + 1) * sq_size
            square = gray[y1:y2, x1:x2]
            
            thresh = cv2.adaptiveThreshold(square, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, 
                                          cv2.THRESH_BINARY_INV, 11, 2)
            
            pixel_count = cv2.countNonZero(thresh)
            area_ratio = pixel_count / (sq_size * sq_size)
            
            if area_ratio > 0.12: 
                occupancy[r, c] = 1
                cv2.rectangle(overlay, (x1, y1), (x2, y2), (0, 255, 0), 2)
                # Label for FEN/Notation
                label = f"{chr(ord('a')+c)}{8-r}"
                cv2.putText(overlay, label, (x1+5, y1+15), cv2.FONT_HERSHEY_SIMPLEX, 0.4, (0,255,0), 1)
            else:
                cv2.rectangle(overlay, (x1, y1), (x2, y2), (0, 0, 255), 1)
                
    return occupancy, overlay

def get_live_frame():
    success, frame = camera.read()
    if not success: return {"error": "Camera fail"}
    
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    ret, corners = cv2.findChessboardCorners(gray, CHESSBOARD_SIZE, None)
    
    if ret:
        cv2.drawChessboardCorners(frame, CHESSBOARD_SIZE, corners, ret)
        
    preview = cv2.resize(frame, (320, 240))
    _, buffer = cv2.imencode('.jpg', preview)
    return {"image": f"data:image/jpeg;base64,{base64.b64encode(buffer).decode('utf-8')}"}

def capture_and_analyze():
    success, frame = camera.read()
    if not success: return {"status": "error"}

    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    ret, corners = cv2.findChessboardCorners(gray, CHESSBOARD_SIZE, None)

    if ret:
        # 1. Use your math to get the full 8x8 boundaries
        outer_pts = get_outer_corners(gray, corners)
        
        # 2. Perspective Warp to perfect square
        dst_pts = np.array([[0, 0], [BOARD_SIZE, 0], [BOARD_SIZE, BOARD_SIZE], [0, BOARD_SIZE]], dtype="float32")
        M = cv2.getPerspectiveTransform(outer_pts, dst_pts)
        warped = cv2.warpPerspective(frame, M, (BOARD_SIZE, BOARD_SIZE))
        
        # 3. Analyze squares
        occupancy, annotated = analyze_occupancy(warped)
        
        
        _, buffer = cv2.imencode('.jpg', annotated)
        return {
            "status": "success",
            "message": f"Board Captured! Detected {np.sum(occupancy)} pieces.",
            "image": f"data:image/jpeg;base64,{base64.b64encode(buffer).decode('utf-8')}",
            "grid": occupancy.tolist()
        }
    
    return {"status": "fail", "message": "Chessboard internal corners not detected."}

ui.expose_api("GET", "/stream", get_live_frame)
ui.expose_api("GET", "/capture", capture_and_analyze)

print(f"System Ready at {ui.local_url}")
App.run()