import cv2
import numpy as np
import base64
import os
import atexit
import time
from arduino.app_utils import App
from arduino.app_bricks.web_ui import WebUI

# A standard 8x8 chessboard has 7x7 internal corners
CHESSBOARD_SIZE = (7, 7)

ui = WebUI()

def initialize_camera():
    """
    Scans for the Logitech C920 across multiple indices.
    Clears existing locks and forces MJPG mode for USB-C Hub stability.
    As it often fails and doesn't work again when running the code.
    """
    print("Pre-clearing camera locks...")
    os.system("fuser -k /dev/video* 2>/dev/null")
    time.sleep(1) # Short pause for hardware reset
    
    # Unplugging and plugging back in often bumps the index (0, 1, 2...)
    for index in [0, 1, 2, 3]:
        try:
            print(f"Checking index {index}...")
            cap = cv2.VideoCapture(index, cv2.CAP_V4L2)
            
            if cap.isOpened():
                # Test read to verify it's a real video stream
                ret, frame = cap.read()
                if ret:
                    # Logitech C920 Tweak: Force MJPG to save USB bandwidth
                    cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc(*'MJPG'))
                    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
                    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
                    
                    # Warm-up frames for auto-exposure
                    for _ in range(5):
                        cap.read()
                        
                    print(f"SUCCESS: Camera active at index {index}")
                    return cap
                else:
                    cap.release()
        except Exception as e:
            print(f"Index {index} error: {e}")
            continue
            
    return None

# Start the camera session
camera = initialize_camera()

@atexit.register
def shutdown_handler():
    """
    Ensures the camera is released so the next run doesn't fail.
    
    """
    if camera:
        camera.release()
        print("Camera hardware released.")

def get_live_frame():
    if not camera or not camera.isOpened():
        return {"error": "Camera offline. Check connection."}
        
    success, frame = camera.read()
    if not success:
        return {"error": "Failed to read frame"}
    
    # Resize for smooth web performance
    preview = cv2.resize(frame, (320, 240))
    _, buffer = cv2.imencode('.jpg', preview)
    img_str = base64.b64encode(buffer).decode('utf-8')
    return {"image": f"data:image/jpeg;base64,{img_str}"}

def capture_and_analyze():
    if not camera or not camera.isOpened():
        return {"status": "error", "message": "Hardware connection lost"}

    success, frame = camera.read()
    if not success:
        return {"status": "error", "message": "Frame grab failed"}

    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    
    # Standard detection with adaptive thresholding for better reliability
    ret, corners = cv2.findChessboardCorners(
        gray, 
        CHESSBOARD_SIZE, 
        cv2.CALIB_CB_ADAPTIVE_THRESH + cv2.CALIB_CB_NORMALIZE_IMAGE
    )

    if ret:
        # Draw for the final report
        cv2.drawChessboardCorners(frame, CHESSBOARD_SIZE, corners, ret)
        
        # Crop logic to focus on the chessboard
        x, y, w, h = cv2.boundingRect(corners)
        pad = 40
        y1, y2 = max(0, y-pad), min(frame.shape[0], y+h+pad)
        x1, x2 = max(0, x-pad), min(frame.shape[1], x+w+pad)
        output_frame = frame[y1:y2, x1:x2]
        
        message = "Test Passed: Chessboard Detected and Cropped!"
    else:
        output_frame = frame
        message = "Test Failed: Board not found. Center the board and check lighting."

    _, buffer = cv2.imencode('.jpg', output_frame)
    img_str = base64.b64encode(buffer).decode('utf-8')
    
    return {
        "status": "success" if ret else "fail",
        "message": message,
        "image": f"data:image/jpeg;base64,{img_str}"
    }

# Endpoints
ui.expose_api("GET", "/stream", get_live_frame)
ui.expose_api("GET", "/capture", capture_and_analyze)

print(f"Application Ready. Local URL: {ui.local_url}")

# The App lab way
App.run()