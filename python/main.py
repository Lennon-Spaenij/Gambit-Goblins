"""Main test program - GRIPPER ONLY"""

# ==================== ARDUINO APP LAB LIBRARY FIX ====================
from arduino.app_utils import App
import serial
print(f"PySerial version: {serial.__version__}")

# ==================== NORMAL IMPORTS ====================
import time
from robot_control import RobotControl
from gripper import Gripper
from config import DEBUG


def test_gripper_sequence(gripper: Gripper):
    """Run automated gripper test sequence"""
    print("\n" + "="*50)
    print("GRIPPER TEST SEQUENCE")
    print("="*50)
    
    tests = [
        ("Opening gripper", lambda: gripper.open()),
        ("Waiting 2 seconds", lambda: time.sleep(2)),
        ("Closing gripper", lambda: gripper.close()),
        ("Waiting 2 seconds", lambda: time.sleep(2)),
        ("Opening gripper", lambda: gripper.open()),
        ("Waiting 1 second", lambda: time.sleep(1)),
        ("Closing gripper", lambda: gripper.close()),
        ("Waiting 1 second", lambda: time.sleep(1)),
        ("Opening gripper", lambda: gripper.open()),
    ]
    
    for description, action in tests:
        print(f"\n→ {description}...")
        result = action()
        if result is not None and not result:
            print(f"✗ Failed: {description}")
            return False
        print(f"✓ Done")
    
    print("\n" + "="*50)
    print("✓ TEST SEQUENCE COMPLETE")
    print("="*50)
    return True


def interactive_mode(gripper: Gripper):
    """Interactive command mode for gripper"""
    print("\n" + "="*50)
    print("INTERACTIVE MODE - GRIPPER ONLY")
    print("="*50)
    print("\nCommands:")
    print("  open   - Open gripper")
    print("  close  - Close gripper")
    print("  status - Check gripper status")
    print("  test   - Run test sequence")
    print("  quit   - Exit program")
    print()
    
    while True:
        try:
            cmd = input("gripper> ").strip().lower()
            
            if cmd == "open":
                print("Opening gripper...")
                if gripper.open():
                    print("✓ Gripper opened")
                else:
                    print("✗ Failed to open gripper")
                    
            elif cmd == "close":
                print("Closing gripper...")
                if gripper.close():
                    print("✓ Gripper closed")
                else:
                    print("✗ Failed to close gripper")
                    
            elif cmd == "status":
                status = gripper.get_status()
                if status:
                    print(f"Arduino status: {status}")
                    print(f"Local state: {'CLOSED' if gripper.is_closed() else 'OPEN'}")
                else:
                    print("✗ Could not get status")
                    
            elif cmd == "test":
                test_gripper_sequence(gripper)
                
            elif cmd == "quit" or cmd == "exit":
                print("Exiting...")
                break
                
            elif cmd == "help" or cmd == "?":
                print("\nCommands: open, close, status, test, quit")
                
            elif cmd == "":
                continue
                
            else:
                print(f"Unknown command: '{cmd}'")
                print("Type 'help' for available commands")
                
        except KeyboardInterrupt:
            print("\n\nInterrupted by user")
            break
        except Exception as e:
            print(f"ERROR: {e}")


def main():
    """Main entry point"""
    print("="*50)
    print("CHESS ROBOT - GRIPPER TEST")
    print("="*50)
    
    robot = None
    
    try:
        print("\nInitializing robot connection...")
        robot = RobotControl()
        
        print("Initializing gripper...")
        gripper = Gripper(robot)
        print("✓ Gripper ready\n")
        
        print("Run automated test sequence? (y/n): ", end="")
        choice = input().strip().lower()
        
        if choice == 'y' or choice == 'yes':
            test_gripper_sequence(gripper)
        
        interactive_mode(gripper)
        
    except KeyboardInterrupt:
        print("\n\nProgram interrupted by user")
        
    except Exception as e:
        print(f"\nERROR: {e}")
        import traceback
        traceback.print_exc()
        
    finally:
        if robot:
            print("\nClosing connection...")
            robot.close()
        print("Goodbye!")


if __name__ == "__main__":
    App.run()