import cv2
import numpy as np
import json
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--rotate", type=int, default=0, choices=[0, 90, 180, 270],
                    help="Board rotation clockwise relative to camera view")
parser.add_argument("--camera", type=int, default=None,
                    help="Camera index (omit to auto-detect)")
args = parser.parse_args()

points = []
CAM_ROT = args.rotate

def on_mouse(event, x, y, flags, param):
    if event == cv2.EVENT_LBUTTONDOWN and len(points) < 4:
        points.append((x, y))
        print(f"Point {len(points)}: ({x}, {y})")

def remap_square(r_cam, c_cam, rot):
    """Convert camera grid index -> standard chess (A1 = bottom-left for white)"""
    if rot == 0:
        return r_cam, c_cam
    elif rot == 90:
        return c_cam, 7 - r_cam
    elif rot == 180:
        return 7 - r_cam, 7 - c_cam
    elif rot == 270:
        return 7 - c_cam, r_cam
    return r_cam, c_cam

# Opening the Camera
cap = None
if args.camera is not None:
    cap = cv2.VideoCapture(args.camera)
else:
    for i in range(4):
        cap = cv2.VideoCapture(i)
        if cap.isOpened():
            print(f"Using camera index {i}")
            break

if cap is None or not cap.isOpened():
    print("No camera found or check permissions.")
    exit(1)

cv2.namedWindow("Calibration")
cv2.setMouseCallback("Calibration", on_mouse)

print("\nClick the four corners in order:")
print("  1. top-left    2. top-right")
print("  3. bottom-right  4. bottom-left")
print("\nKeys:  r = reset    s = save & quit    q = quit without saving\n")

while True:
    ret, frame = cap.read()
    if not ret:
        continue

    vis = frame.copy()

    # Draw clicked points + numbers
    for i, (x, y) in enumerate(points):
        cv2.circle(vis, (x, y), 6, (0, 0, 255), -1)
        cv2.putText(vis, str(i+1), (x+10, y-10),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)

    # When we have 4 points -> show preview grid + labels
    if len(points) == 4:
        src = np.float32([[0,0], [8,0], [8,8], [0,8]])
        dst = np.float32(points)
        H = cv2.getPerspectiveTransform(src, dst)

        # Generate 9×9 grid points in warped space
        grid_src = np.mgrid[0:9, 0:9][::-1].T.reshape(-1, 1, 2).astype(np.float32)
        grid_dst = cv2.perspectiveTransform(grid_src, H).reshape(9, 9, 2)

        # Draw grid lines
        for i in range(9):
            cv2.polylines(vis, [grid_dst[i].astype(int)], False, (200, 200, 200), 1)
            cv2.polylines(vis, [grid_dst[:,i].astype(int)], False, (200, 200, 200), 1)

        # Draw square labels (A1–H8)
        files = 'abcdefgh'
        ranks = '87654321'
        for r in range(8):
            for c in range(8):
                r_std, c_std = remap_square(r, c, CAM_ROT)
                label = files[c_std] + ranks[r_std]

                # Center of square
                center = (grid_dst[r:r+2, c:c+2].mean(axis=(0,1))).astype(int)
                cv2.putText(vis, label, (center[0]-12, center[1]+6),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.55, (0, 255, 255), 1)

    cv2.imshow("Calibration", vis)

    key = cv2.waitKey(20) & 0xFF
    if key == ord('q'):
        print("Quit without saving.")
        break
    elif key == ord('r'):
        points.clear()
        print("Points reset.")
    elif key == ord('s'):
        if len(points) != 4:
            print("Need exactly 4 points first.")
            continue

        # Same transform as above
        src = np.float32([[0,0], [8,0], [8,8], [0,8]])
        dst = np.float32(points)
        H = cv2.getPerspectiveTransform(src, dst)
        grid_src = np.mgrid[0:9, 0:9][::-1].T.reshape(-1, 1, 2).astype(np.float32)
        grid_dst = cv2.perspectiveTransform(grid_src, H).reshape(9, 9, 2)

        squares = {}
        files = 'abcdefgh'
        ranks = '87654321'
        for r in range(8):
            for c in range(8):
                r_std, c_std = remap_square(r, c, CAM_ROT)
                sq_name = files[c_std] + ranks[r_std]

                tl = grid_dst[r,   c  ].tolist()
                tr = grid_dst[r,   c+1].tolist()
                br = grid_dst[r+1, c+1].tolist()
                bl = grid_dst[r+1, c  ].tolist()

                squares[sq_name] = [tl, tr, br, bl]

        with open("sqdict.json", "w") as f:
            json.dump(squares, f, indent=2)

        print("Saved sqdict.json")
        break

cap.release()
cv2.destroyAllWindows()