import serial
import time

class GripperControl:
    def __init__(self, port='/dev/ttyACM0', baudrate=9600):
        self.ser = serial.Serial(port, baudrate, timeout=1)
        time.sleep(2)
        
        while self.ser.in_waiting:
            print(self.ser.readline().decode('utf-8').strip())
    
    def send_command(self, command):
        self.ser.write(f"{command}\n".encode('utf-8'))
        time.sleep(0.1)
        
        if self.ser.in_waiting:
            response = self.ser.readline().decode('utf-8').strip()
            print(f"Arduino: {response}")
            return response
        return None
    
    def open_gripper(self):
        return self.send_command("OPEN")
    
    def close_gripper(self):
        return self.send_command("CLOSE")
    
    def get_status(self):
        return self.send_command("STATUS")
    
    def close(self):
        self.ser.close()


if __name__ == "__main__":
    gripper = GripperControl(port='/dev/ttyACM0')
    
    try:
        # Test
        gripper.close_gripper()
        time.sleep(2)
        gripper.open_gripper()
        time.sleep(2)
        gripper.get_status()
        
    finally:
        gripper.close()