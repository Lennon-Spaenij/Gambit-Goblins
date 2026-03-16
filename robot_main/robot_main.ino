// ==================== ROBOT MAIN CONTROLLER ====================
// Chess Robot - Modular Controller
// Gripper + Steppers + LCD + Serial Communication
// ================================================================

// ==================== INCLUDES ====================
// LCD library (commented out for gripper-only test)
// #include <Wire.h>
// #include <LiquidCrystal_I2C.h>

// ==================== PIN DEFINITIONS ====================
// GRIPPER
#define GRIPPER_PIN 11

// STEPPERS (commented out for gripper-only test)
// #define X_STEP 2
// #define X_DIR 5
// #define Y_STEP 3
// #define Y_DIR 6
// #define Z_STEP 4
// #define Z_DIR 7
// #define ENABLE 8

// LCD (commented out for gripper-only test)
// #define LCD_ADDRESS 0x27
// #define LCD_COLS 20
// #define LCD_ROWS 4

// ==================== CONFIGURATION ====================
// Gripper - YOUR VALUES
#define SERVO_INTERVAL 20
#define GRIPPER_OPEN 400
#define GRIPPER_CLOSED 1300

// Steppers (commented out for gripper-only test)
// #define STEPS_PER_REV 3200
// #define STEP_DELAY 100  // microseconds

// ==================== GLOBAL VARIABLES ====================
// LCD (commented out for gripper-only test)
// LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);

// Gripper state
int pulse;
bool gripper_closed = false;

// Stepper state (commented out for gripper-only test)
// long x_position = 0;
// long y_position = 0;
// long z_position = 0;
// long x_target = 0;
// long y_target = 0;
// long z_target = 0;
// bool x_moving = false;
// bool y_moving = false;
// bool z_moving = false;
// unsigned long stepper_timer = 0;
// bool step_state = false;

// LCD update timing (commented out for gripper-only test)
// unsigned long lcd_timer = 0;
// int lcd_counter = 0;

// ==================== FUNCTION DECLARATIONS ====================
// Gripper functions
void gripper(int pulse);
void start_open_gripper();
void start_close_gripper();

// Stepper functions (commented out for gripper-only test)
// void stepper_update();
// void move_x(long steps);
// void move_y(long steps);
// void move_z(long steps);
// void home_all();

// LCD functions (commented out for gripper-only test)
// void lcd_update();

// Serial communication
void process_serial_command();

// ==================== SETUP ====================
void setup() {
  Serial.begin(9600);
  
  // Gripper setup
  pinMode(GRIPPER_PIN, OUTPUT);
  
  // Stepper setup (commented out for gripper-only test)
  // pinMode(X_STEP, OUTPUT);
  // pinMode(X_DIR, OUTPUT);
  // pinMode(Y_STEP, OUTPUT);
  // pinMode(Y_DIR, OUTPUT);
  // pinMode(Z_STEP, OUTPUT);
  // pinMode(Z_DIR, OUTPUT);
  // pinMode(ENABLE, OUTPUT);
  // digitalWrite(ENABLE, LOW);
  // digitalWrite(X_DIR, HIGH);
  // digitalWrite(Y_DIR, HIGH);
  // digitalWrite(Z_DIR, HIGH);
  
  // LCD setup (commented out for gripper-only test)
  // unsigned long start_time = millis();
  // Wire.begin();
  // Wire.setClock(100000);
  // lcd.init();
  // lcd.backlight();
  // unsigned long boot_time = millis() - start_time;
  // lcd.setCursor(0, 0);
  // lcd.print("Chess Robot ");
  // lcd.print(boot_time);
  // lcd.print("ms");
  // lcd.setCursor(0, 1);
  // lcd.print("Ready!");
  
  Serial.println("READY");
}

// ==================== MAIN LOOP ====================
void loop() {
  // Process serial commands from Python
  if (Serial.available() > 0) {
    process_serial_command();
  }
  
  // Update gripper (continuous regripping)
  gripper(0);
  
  // Stepper updates (commented out for gripper-only test)
  // stepper_update();
  
  // LCD update (commented out for gripper-only test)
  // lcd_update();
}

// ==================== GRIPPER FUNCTIONS ====================
// YOUR ORIGINAL GRIPPER CODE - UNCHANGED!

void gripper(int new_pulse) {
  static unsigned long timer;
  static int last_pulse;
  
  if (new_pulse == 0) {
    new_pulse = last_pulse;
  } else {
    last_pulse = new_pulse;
    pulse = new_pulse;
  }
  
  if (millis() > timer) {
    digitalWrite(GRIPPER_PIN, HIGH);
    delayMicroseconds(new_pulse);
    digitalWrite(GRIPPER_PIN, LOW);
    timer = millis() + SERVO_INTERVAL;
  }
}

void start_close_gripper() {
  static unsigned long timer;
  if (millis() > timer) {
    for (int i = 0; i < 10; i++) {
      gripper(GRIPPER_CLOSED);
      timer = millis() + 20;
    }
  }
  
  gripper_closed = true;
  Serial.println("DONE:CLOSE");
}

void start_open_gripper() {
  static unsigned long timer;
  if (millis() > timer) {
    for (int i = 0; i < 10; i++) {
      gripper(GRIPPER_OPEN);
      timer = millis() + 20;
    }
  }
  
  gripper_closed = false;
  Serial.println("DONE:OPEN");
}

// ==================== STEPPER FUNCTIONS ====================
// COMMENTED OUT FOR GRIPPER-ONLY TEST
/*
void stepper_update() {
  unsigned long current_micros = micros();
  
  if (current_micros - stepper_timer < STEP_DELAY) {
    return;
  }
  
  stepper_timer = current_micros;
  bool any_moving = false;
  
  // X Motor
  if (x_moving) {
    if (step_state) {
      digitalWrite(X_STEP, HIGH);
    } else {
      digitalWrite(X_STEP, LOW);
      
      if (x_position < x_target) {
        x_position++;
      } else if (x_position > x_target) {
        x_position--;
      }
      
      if (x_position == x_target) {
        x_moving = false;
        Serial.println("DONE:MOVE_X");
      }
    }
    any_moving = true;
  }
  
  // Y Motor
  if (y_moving) {
    if (step_state) {
      digitalWrite(Y_STEP, HIGH);
    } else {
      digitalWrite(Y_STEP, LOW);
      
      if (y_position < y_target) {
        y_position++;
      } else if (y_position > y_target) {
        y_position--;
      }
      
      if (y_position == y_target) {
        y_moving = false;
        Serial.println("DONE:MOVE_Y");
      }
    }
    any_moving = true;
  }
  
  // Z Motor
  if (z_moving) {
    if (step_state) {
      digitalWrite(Z_STEP, HIGH);
    } else {
      digitalWrite(Z_STEP, LOW);
      
      if (z_position < z_target) {
        z_position++;
      } else if (z_position > z_target) {
        z_position--;
      }
      
      if (z_position == z_target) {
        z_moving = false;
        Serial.println("DONE:MOVE_Z");
      }
    }
    any_moving = true;
  }
  
  if (any_moving) {
    step_state = !step_state;
  }
}

void move_x(long steps) {
  x_target = steps;
  digitalWrite(X_DIR, (steps > x_position) ? HIGH : LOW);
  x_moving = true;
}

void move_y(long steps) {
  y_target = steps;
  digitalWrite(Y_DIR, (steps > y_position) ? HIGH : LOW);
  y_moving = true;
}

void move_z(long steps) {
  z_target = steps;
  digitalWrite(Z_DIR, (steps > z_position) ? HIGH : LOW);
  z_moving = true;
}

void home_all() {
  move_x(0);
  move_y(0);
  move_z(0);
}
*/

// ==================== LCD FUNCTIONS ====================
// COMMENTED OUT FOR GRIPPER-ONLY TEST
/*
void lcd_update() {
  if (millis() - lcd_timer < 500) {
    return;
  }
  
  lcd_timer = millis();
  
  lcd.setCursor(0, 0);
  lcd.print("Chess Robot      ");
  
  lcd.setCursor(0, 1);
  lcd.print("Grip: ");
  lcd.print(gripper_closed ? "CLOSED" : "OPEN  ");
  
  lcd.setCursor(0, 2);
  lcd.print("X:");
  lcd.print(x_position);
  lcd.print(" Y:");
  lcd.print(y_position);
  lcd.print("    ");
  
  lcd.setCursor(0, 3);
  lcd.print("Z:");
  lcd.print(z_position);
  lcd.print(" C:");
  if (lcd_counter < 10) lcd.print("0");
  lcd.print(lcd_counter);
  lcd.print("  ");
  
  lcd_counter++;
  if (lcd_counter > 99) lcd_counter = 0;
}
*/

// ==================== SERIAL COMMAND PROCESSING ====================
void process_serial_command() {
  String command = Serial.readStringUntil('\n');
  command.trim();
  command.toUpperCase();
  
  // ===== GRIPPER COMMANDS =====
  if (command == "OPEN") {
    start_open_gripper();
    Serial.println("OK:OPEN");
    
  } else if (command == "CLOSE") {
    start_close_gripper();
    Serial.println("OK:CLOSE");
    
  } else if (command == "GRIP_STATUS") {
    Serial.print("GRIPPER:");
    Serial.println(gripper_closed ? "CLOSED" : "OPEN");
    
  // ===== STEPPER COMMANDS (commented out for gripper-only test) =====
  // } else if (command.startsWith("MOVE_X ")) {
  //   long steps = command.substring(7).toInt();
  //   move_x(steps);
  //   Serial.println("OK:MOVE_X");
  //   
  // } else if (command.startsWith("MOVE_Y ")) {
  //   long steps = command.substring(7).toInt();
  //   move_y(steps);
  //   Serial.println("OK:MOVE_Y");
  //   
  // } else if (command.startsWith("MOVE_Z ")) {
  //   long steps = command.substring(7).toInt();
  //   move_z(steps);
  //   Serial.println("OK:MOVE_Z");
  //   
  // } else if (command == "HOME") {
  //   home_all();
  //   Serial.println("OK:HOME");
  //   
  // } else if (command == "STATUS") {
  //   Serial.print("X:");
  //   Serial.print(x_position);
  //   Serial.print(" Y:");
  //   Serial.print(y_position);
  //   Serial.print(" Z:");
  //   Serial.print(z_position);
  //   Serial.print(" GRIP:");
  //   Serial.println(gripper_closed ? "CLOSED" : "OPEN");
    
  } else {
    Serial.println("ERROR:UNKNOWN_COMMAND");
  }
}