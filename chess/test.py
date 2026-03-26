import math

L1 = 152.61
L2 = 167.75
ARM_BASE_X = 140.0
ARM_BASE_Y = -38.72

# Where it actually ended up
actual_x, actual_y = 192.5, 87.5   # f3
x_local = actual_x - ARM_BASE_X
y_local = actual_y - ARM_BASE_Y
d = math.sqrt(x_local**2 + y_local**2)
cos_q2 = (d**2 - L1**2 - L2**2) / (2 * L1 * L2)
q2 = math.acos(cos_q2)
q1 = math.atan2(y_local, x_local) - math.atan2(L2 * math.sin(q2), L1 + L2 * math.cos(q2))
actual_q1 = math.degrees(q1)
actual_q2 = math.degrees(q2)

# Where it was supposed to go
target_q1 = 39.48  # d5
target_q2 = 104.27

# Home
home_q1 = 90.0
home_q2 = 0.0

print(f"Actual angles at f3:   q1={actual_q1:.2f}°  q2={actual_q2:.2f}°")
print(f"Target angles for d5:  q1={target_q1:.2f}°  q2={target_q2:.2f}°")
print(f"")
print(f"J1 commanded delta: {target_q1 - home_q1:.2f}°")
print(f"J1 actual delta:    {actual_q1 - home_q1:.2f}°")
print(f"J1 ratio error:     {(actual_q1 - home_q1) / (target_q1 - home_q1):.4f}")
print(f"")
print(f"J2 commanded delta: {target_q2 - home_q2:.2f}°")
print(f"J2 actual delta:    {actual_q2 - home_q2:.2f}°")
print(f"J2 ratio error:     {(actual_q2 - home_q2) / (target_q2 - home_q2):.4f}")