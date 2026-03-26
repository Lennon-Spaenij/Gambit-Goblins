import math
 
L1 = 152.61  # length of arm 1 in mm
L2 = 167.75  # length of arm 2 in mm
SQ = 35      # square size in mm
 
# Shoulder joint is centered between d1 and e1, 38.72mm below the a1 corner.
# X: 4 squares from the left edge = 140mm
# Y: below the board = negative
ARM_BASE_X = 140.0
ARM_BASE_Y = -38.72
 
# Home position: arm fully extended straight up (toward row 8 and beyond).
# In standard math coordinates, straight up = 90 degrees.
# Set these as currentTheta1/2 in the Arduino code, and physically place
# the arm pointing straight up before every power-on.
HOME_Q1 = 90.0
HOME_Q2 = 0.0
 
 
def square_to_xy(square: str):
    """Convert a square name (e.g. 'e2') to world XY in mm."""
    col = ord(square[0]) - ord('a')   # a=0 .. h=7
    row = int(square[1]) - 1          # 1=0 .. 8=7
    x = col * SQ + SQ / 2
    y = row * SQ + SQ / 2             # row 1 is at bottom (low Y), row 8 at top
    return x, y
 
 
def scara_ik(x, y):
    """Compute joint angles for a world XY position (mm)."""
    x_local = x - ARM_BASE_X
    y_local = y - ARM_BASE_Y
 
    d = math.sqrt(x_local**2 + y_local**2)
    cos_q2 = (d**2 - L1**2 - L2**2) / (2 * L1 * L2)
 
    if cos_q2 < -1 or cos_q2 > 1:
        raise ValueError(
            f"Square out of reach — d={d:.1f}mm, cos_q2={cos_q2:.4f}, "
            f"max reach={L1 + L2:.1f}mm"
        )
 
    q2 = math.acos(cos_q2)
    q1 = math.atan2(y_local, x_local) - math.atan2(
        L2 * math.sin(q2), L1 + L2 * math.cos(q2)
    )
    return math.degrees(q1), math.degrees(q2)
 
 
def square_to_angles(square: str):
    """Full pipeline: square name → world XY → joint angles."""
    x, y = square_to_xy(square)
    q1, q2 = scara_ik(x, y)
    print(f"{square} -> world XY: ({x:.2f}, {y:.2f})  |  q1={q1:.2f}°  q2={q2:.2f}°")
    return q1, q2
 
 
def check_all_squares():
    """Print angles for all 64 squares, flagging unreachable ones."""
    print(f"Home position: q1={HOME_Q1}°  q2={HOME_Q2}°  (arm pointing straight up)\n")
    print(f"{'Square':<8} {'World X':>8} {'World Y':>8} {'q1':>8} {'q2':>8}")
    print("-" * 48)
    for row in range(8, 0, -1):
        for col in "abcdefgh":
            sq = f"{col}{row}"
            try:
                x, y = square_to_xy(sq)
                q1, q2 = scara_ik(x, y)
                print(f"{sq:<8} {x:>8.1f} {y:>8.1f} {q1:>8.2f} {q2:>8.2f}")
            except ValueError:
                print(f"{sq:<8} {'':>8} {'':>8} {'OUT OF REACH':>17}")
 
 
def main():
    print("=== Corner squares ===")
    for sq in ["a1", "h1", "a8", "h8"]:
        try:
            square_to_angles(sq)
        except ValueError as e:
            print(f"{sq} -> ERROR: {e}")
 
    print("\n=== Center squares ===")
    for sq in ["d4", "e4", "d5", "e5"]:
        try:
            square_to_angles(sq)
        except ValueError as e:
            print(f"{sq} -> ERROR: {e}")
 
    print("\n=== Full board ===")
    check_all_squares()
 
 
if __name__ == "__main__":
    main()