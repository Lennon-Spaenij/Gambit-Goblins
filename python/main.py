import time
from arduino.app_utils import App, Bridge

bridge = Bridge()

HOLD_TIME = 2       # seconds to hold each state
TOTAL_TIME = 1800   # 30 minutes in seconds

def main():
    last_switch = time.time()
    start_time = time.time()
    state = 0

    bridge.call("set_gripper", state)

    while True:
        now = time.time()

        # Stop after 30 minutes
        if now - start_time >= TOTAL_TIME:
            print("Timer finished")
            bridge.call("set_gripper", 0)  # open gripper at end
            break

        # Non-blocking gripper toggle
        if now - last_switch >= HOLD_TIME:
            state = 1 - state
            print("Closing gripper" if state else "Opening gripper")
            bridge.call("set_gripper", state)
            last_switch = now

if __name__ == "__main__":
    App.run(main)