// Pin Definitions
#define Gripper 11 
#define SERVO_INTERVAL 20
#define GRIPPER_OPEN 900
#define GRIPPER_CLOSED 1700

int pulse;

void gripper(int pulse);
void CloseGripper();
void OpenGripper();
void process_command();

void setup() {
  Serial.begin(9600);
  pinMode(Gripper, OUTPUT);
  
  Serial.println("Arduino Q Ready - Gripper Control");
  Serial.println("Commands: OPEN, CLOSE, STATUS");
}

void loop() {
  // Check for serial commands
  if (Serial.available() > 0) {
    process_command();
  }
  
  // Keep regripping (maintains position)
  gripper(0);  // Uses last pulse value
  
}

void process_command() {
  String command = Serial.readStringUntil('\n');
  command.trim();
  command.toUpperCase();
  
  if (command == "OPEN") {
    OpenGripper();
    Serial.println("OK:OPEN");
    
  } else if (command == "CLOSE") {
    CloseGripper();
    Serial.println("OK:CLOSE");
    
  } else if (command == "STATUS") {
    // Report current position based on last pulse
    if (pulse == GRIPPER_OPEN) {
      Serial.println("STATUS:OPEN");
    } else if (pulse == GRIPPER_CLOSED) {
      Serial.println("STATUS:CLOSED");
    } else {
      Serial.println("STATUS:UNKNOWN");
    }
    
  } else {
    Serial.println("ERROR:UNKNOWN_COMMAND");
  }
}

void gripper(int new_pulse) {
  static unsigned long timer;
  static int lastPulse;
  
  if (new_pulse == 0) {
    new_pulse = lastPulse;
  } else {
    lastPulse = new_pulse;
    pulse = new_pulse;  // Store globally for status
  }
  
  if (millis() > timer) {
    digitalWrite(Gripper, HIGH);
    delayMicroseconds(new_pulse);
    digitalWrite(Gripper, LOW);
    timer = millis() + SERVO_INTERVAL;
  }
}

void CloseGripper() {
  static unsigned long timer;
  if (millis() > timer) {
    for (int i = 0; i < 10; i++) {
      gripper(GRIPPER_CLOSED);
      timer = millis() + 20;
    }
  }
}

void OpenGripper() {
  static unsigned long timer;
  if (millis() > timer) {
    for (int i = 0; i < 10; i++) {
      gripper(GRIPPER_OPEN);
      timer = millis() + 20;
    }
  }
}