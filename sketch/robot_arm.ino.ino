// Pin Definitions
#define Gripper 11 

#define SERVO_INTERVAL 20
#define GRIPPER_OPEN 900
#define GRIPPER_CLOSED 1700
#define GRIPPER_HOLD_TIME_CLOSED 2000  // 3 seconds closed
#define GRIPPER_HOLD_TIME_OPEN 2000    // 1 second open

// Timing variables for gripper state machine
unsigned long gripper_state_timer = 0;
bool gripper_closed = false;
int pulse;

void gripper(int pulse);
void CloseGripper();
void OpenGripper();

void setup() {
  Serial.begin(9600);
  pinMode(Gripper, OUTPUT);
}

void loop() {
  // State machine for gripper with millis()
  if (millis() > gripper_state_timer) {
    if (gripper_closed) {
      OpenGripper();
    } else {
      CloseGripper();
    }
  }
  
  // Keep regripping while in current state
  if (gripper_closed) {
    gripper(GRIPPER_CLOSED);  // Keep closed position
  } else {
    gripper(GRIPPER_OPEN);    // Keep open position
  }
}

void gripper(int pulse) {    // 900 open, 1700 closed angle 
  static unsigned long timer;
  static int lastPulse;
  if (pulse == 0) {
    pulse = lastPulse;
  } else {
    lastPulse = pulse;
  }
  if (millis() > timer) {
    digitalWrite(Gripper, HIGH);
    delayMicroseconds(pulse);
    digitalWrite(Gripper, LOW);
    timer = millis() + SERVO_INTERVAL;
  }
}

void CloseGripper() {
  static unsigned long timer;
  if (millis() > timer) {
    for (int i = 0; i < 10; i++) { // close gripper 10 pulses per 200 ms
      gripper(GRIPPER_CLOSED);
      timer = millis() + 20; // update every 0.02s can change
    }
  }
  
  // Update state
  gripper_closed = true;
  gripper_state_timer = millis() + GRIPPER_HOLD_TIME_CLOSED;  // 3 seconds
  Serial.println("Gripper close");
}

void OpenGripper() {
  static unsigned long timer;
  if (millis() > timer) {
    for (int i = 0; i < 10; i++) { // open gripper 10 pulses per 200 ms
      gripper(GRIPPER_OPEN);
      timer = millis() + 20; // update every 0.02s can change
    }
  }
  
  // Update state
  gripper_closed = false;
  gripper_state_timer = millis() + GRIPPER_HOLD_TIME_OPEN;  // 1 second
  Serial.println("Gripper open");
}