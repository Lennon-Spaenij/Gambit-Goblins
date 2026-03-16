#include <Arduino_RouterBridge.h>
#include <LCDi2c.h>

// Stepper Motor Pins
#define X_STEP 2
#define X_DIR 5
#define Y_STEP 3
#define Y_DIR 6
#define Z_STEP 4
#define Z_DIR 7
#define ENABLE 8

// Button Pins
#define BUTTON_UP 11
#define BUTTON_OK 10
#define BUTTON_DOWN 9

// Gripper
#define Gripper 12
#define SERVO_INTERVAL 20
#define GRIPPER_OPEN 400
#define GRIPPER_CLOSED 1200

// Stepper Settings
#define STEPS_PER_REV 3200
#define STEP_DELAY 100

// LCD Settings
#define LCD_ROWS 4
#define LCD_COLUMNS 20

// Menu States
enum MenuState {
  MENU_START,
  MENU_CONFIRM_START,
  MENU_GAME,
  MENU_CONFIRM_RESIGN
};

// Game States
MenuState currentMenu = MENU_START;
bool robotTurn = false;
bool gameActive = false;

// Menu Navigation
int menuSelection = 0; // 0 = Yes/Start Game, 1 = No

// Button State Variables (non-blocking debounce)
bool buttonUpState = false;
bool buttonOkState = false;
bool buttonDownState = false;
bool buttonUpLastState = false;
bool buttonOkLastState = false;
bool buttonDownLastState = false;
unsigned long buttonUpDebounceTime = 0;
unsigned long buttonOkDebounceTime = 0;
unsigned long buttonDownDebounceTime = 0;
unsigned long buttonOkPressStart = 0;
const unsigned long DEBOUNCE_DELAY = 50;
const unsigned long LONG_PRESS_TIME = 3000;

// Timer Variables
#define STARTING_TIME 1800000  // 30 minutes in milliseconds (30 * 60 * 1000)
unsigned long playerStartTime = 0;
unsigned long robotStartTime = 0;
unsigned long playerElapsedTime = 0;
unsigned long robotElapsedTime = 0;
unsigned long playerPauseTime = 0;
unsigned long robotPauseTime = 0;

// Score Variables
int playerScore = 0;
int robotScore = 0;

// Move Display
String robotNextMove = "e2e4";
String playerLastMove = "----";

// Display Update Control
unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_UPDATE_INTERVAL = 100;

// Gripper State
bool gripper_closed = false;

// LCD Object
LCDi2c lcd(0x27, Wire);

// Function Declarations
void gripper(int pulse);
void CloseGripper();
void OpenGripper();
void set_gripper(int state);
void readButtons();
bool buttonUpPressed();
bool buttonDownPressed();
bool buttonOkPressed();
bool buttonOkLongPress();
void handleStartMenu();
void handleConfirmStartMenu();
void handleGameScreen();
void checkForResignHold();
void handleConfirmResignMenu();
void startGame();
void endGame();
void toggleTurn();
void updateTimers();
void pauseTimers();
void resumeTimers();
unsigned long getPlayerTime();
unsigned long getRobotTime();
void displayStartMenu();
void displayConfirmStartMenu();
void displayGameScreen();
void displayConfirmResignMenu();
void updateDisplay();
void formatTime(unsigned long milliseconds);

void setup() {
  Serial.begin(9600);
  
  // Initialize Stepper Pins
  pinMode(X_STEP, OUTPUT);
  pinMode(X_DIR, OUTPUT);
  pinMode(Y_STEP, OUTPUT);
  pinMode(Y_DIR, OUTPUT);
  pinMode(Z_STEP, OUTPUT);
  pinMode(Z_DIR, OUTPUT);
  pinMode(ENABLE, OUTPUT);
  digitalWrite(ENABLE, LOW);  // Enable drivers
  
  // Initialize Gripper
  pinMode(Gripper, OUTPUT);
  
  // Initialize Buttons
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_OK, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  
  // Initialize LCD
  lcd.begin(LCD_ROWS, LCD_COLUMNS);
  lcd.cls();
  
  // Show initial menu
  displayStartMenu();
  
  // Initialize Bridge
  Bridge.begin();
  Bridge.provide("set_gripper", set_gripper);
  
  Serial.println("Chess Robot System Initialized");
}

void loop() {
  // Maintain gripper position (non-blocking)
  if (gripper_closed) {
    gripper(GRIPPER_CLOSED);
  } else {
    gripper(GRIPPER_OPEN);
  }
  
  // Read buttons (non-blocking debounce)
  readButtons();
  
  // Handle menu state
  switch (currentMenu) {
    case MENU_START:
      handleStartMenu();
      break;
      
    case MENU_CONFIRM_START:
      handleConfirmStartMenu();
      break;
      
    case MENU_GAME:
      handleGameScreen();
      checkForResignHold();
      break;
      
    case MENU_CONFIRM_RESIGN:
      handleConfirmResignMenu();
      break;
  }
  
  // Update display if needed
  if (millis() - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
    updateDisplay();
    lastDisplayUpdate = millis();
  }
}

// ==================== GRIPPER FUNCTIONS ====================
void gripper(int pulse) {
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
    gripper(GRIPPER_CLOSED);
    timer = millis() + SERVO_INTERVAL;
  }
  gripper_closed = true;
  Serial.println("Gripper close");
}

void OpenGripper() {
  static unsigned long timer;
  if (millis() > timer) {
    gripper(GRIPPER_OPEN);
    timer = millis() + SERVO_INTERVAL;
  }
  gripper_closed = false;
  Serial.println("Gripper open");
}

void set_gripper(int state) {
  if (state) {
    CloseGripper();
  } else {
    OpenGripper();
  }
}

// ==================== BUTTON READING ====================
void readButtons() {
  unsigned long currentTime = millis();
  
  // Read UP button
  bool upReading = (digitalRead(BUTTON_UP) == LOW);
  if (upReading != buttonUpLastState) {
    buttonUpDebounceTime = currentTime;
  }
  if ((currentTime - buttonUpDebounceTime) > DEBOUNCE_DELAY) {
    if (upReading != buttonUpState) {
      buttonUpState = upReading;
    }
  }
  buttonUpLastState = upReading;
  
  // Read DOWN button
  bool downReading = (digitalRead(BUTTON_DOWN) == LOW);
  if (downReading != buttonDownLastState) {
    buttonDownDebounceTime = currentTime;
  }
  if ((currentTime - buttonDownDebounceTime) > DEBOUNCE_DELAY) {
    if (downReading != buttonDownState) {
      buttonDownState = downReading;
    }
  }
  buttonDownLastState = downReading;
  
  // Read OK button (with long press detection)
  bool okReading = (digitalRead(BUTTON_OK) == LOW);
  if (okReading != buttonOkLastState) {
    buttonOkDebounceTime = currentTime;
    if (okReading) {
      buttonOkPressStart = currentTime;
    }
  }
  if ((currentTime - buttonOkDebounceTime) > DEBOUNCE_DELAY) {
    if (okReading != buttonOkState) {
      buttonOkState = okReading;
    }
  }
  buttonOkLastState = okReading;
}

bool buttonUpPressed() {
  static bool lastState = false;
  if (buttonUpState && !lastState) {
    lastState = true;
    return true;
  }
  if (!buttonUpState) lastState = false;
  return false;
}

bool buttonDownPressed() {
  static bool lastState = false;
  if (buttonDownState && !lastState) {
    lastState = true;
    return true;
  }
  if (!buttonDownState) lastState = false;
  return false;
}

bool buttonOkPressed() {
  static bool lastState = false;
  if (buttonOkState && !lastState) {
    lastState = true;
    return true;
  }
  if (!buttonOkState) lastState = false;
  return false;
}

bool buttonOkLongPress() {
  if (buttonOkState && (millis() - buttonOkPressStart >= LONG_PRESS_TIME)) {
    buttonOkPressStart = millis() + 10000; // Prevent repeated triggers
    return true;
  }
  return false;
}

// ==================== MENU HANDLERS ====================
void handleStartMenu() {
  if (buttonOkPressed()) {
    currentMenu = MENU_CONFIRM_START;
    menuSelection = 0;
    displayConfirmStartMenu();
    Serial.println("Moved to confirm start menu");
  }
}

void handleConfirmStartMenu() {
  if (buttonUpPressed() || buttonDownPressed()) {
    menuSelection = 1 - menuSelection; // Toggle between 0 and 1
    displayConfirmStartMenu();
  }
  
  if (buttonOkPressed()) {
    if (menuSelection == 0) { // Yes
      startGame();
    } else { // No
      currentMenu = MENU_START;
      menuSelection = 0;
      displayStartMenu();
      Serial.println("Returned to start menu");
    }
  }
}

void handleGameScreen() {
  if (buttonOkPressed() && gameActive) {
    // Player pressed OK - their move is complete
    toggleTurn();
    Serial.println("Turn toggled");
  }
  
  // Check for time out
  if (gameActive) {
    if (getPlayerTime() == 0) {
      Serial.println("Player time out - Robot wins!");
      // You can add endGame() or other logic here
    }
    if (getRobotTime() == 0) {
      Serial.println("Robot time out - Player wins!");
      // You can add endGame() or other logic here
    }
  }
  
  // Update timers
  updateTimers();
}

void checkForResignHold() {
  if (buttonOkLongPress() && gameActive) {
    currentMenu = MENU_CONFIRM_RESIGN;
    menuSelection = 0;
    pauseTimers();
    displayConfirmResignMenu();
    Serial.println("Resign menu activated");
  }
}

void handleConfirmResignMenu() {
  if (buttonUpPressed() || buttonDownPressed()) {
    menuSelection = 1 - menuSelection; // Toggle between 0 and 1
    displayConfirmResignMenu();
  }
  
  if (buttonOkPressed()) {
    if (menuSelection == 0) { // Yes - Resign
      endGame();
    } else { // No - Continue
      currentMenu = MENU_GAME;
      menuSelection = 0;
      resumeTimers();
      displayGameScreen();
      Serial.println("Returned to game");
    }
  }
}

// ==================== GAME LOGIC ====================
void startGame() {
  gameActive = true;
  robotTurn = false; // Player starts
  currentMenu = MENU_GAME;
  
  // Reset scores and timers
  playerScore = 0;
  robotScore = 0;
  playerElapsedTime = 0;
  robotElapsedTime = 0;
  
  // Start player timer
  playerStartTime = millis();
  robotStartTime = 0;
  
  // Reset moves
  robotNextMove = "e2e4";
  playerLastMove = "----";
  
  displayGameScreen();
  Serial.println("Game started!");
}

void endGame() {
  gameActive = false;
  currentMenu = MENU_START;
  menuSelection = 0;
  displayStartMenu();
  Serial.println("Game ended - Resigned");
}

void toggleTurn() {
  unsigned long currentTime = millis();
  
  if (robotTurn) {
    // Was robot's turn, now player's turn
    robotTurn = false;
    
    // Stop robot timer
    if (robotStartTime > 0) {
      robotElapsedTime += (currentTime - robotStartTime);
    }
    
    // Start player timer
    playerStartTime = currentTime;
    
    Serial.println("Player's turn");
    
  } else {
    // Was player's turn, now robot's turn
    robotTurn = true;
    
    // Stop player timer
    if (playerStartTime > 0) {
      playerElapsedTime += (currentTime - playerStartTime);
    }
    
    // Start robot timer
    robotStartTime = currentTime;
    
    Serial.println("Robot's turn");
  }
}

void updateTimers() {
  // Timers update automatically based on millis()
  // No blocking code needed
}

void pauseTimers() {
  unsigned long currentTime = millis();
  
  if (robotTurn && robotStartTime > 0) {
    robotPauseTime = currentTime - robotStartTime;
  } else if (!robotTurn && playerStartTime > 0) {
    playerPauseTime = currentTime - playerStartTime;
  }
}

void resumeTimers() {
  unsigned long currentTime = millis();
  
  if (robotTurn) {
    robotStartTime = currentTime - robotPauseTime;
  } else {
    playerStartTime = currentTime - playerPauseTime;
  }
}

unsigned long getPlayerTime() {
  unsigned long elapsed;
  if (gameActive && !robotTurn && playerStartTime > 0) {
    elapsed = playerElapsedTime + (millis() - playerStartTime);
  } else {
    elapsed = playerElapsedTime;
  }
  
  // Count down from STARTING_TIME
  if (elapsed >= STARTING_TIME) {
    return 0;  // Time's up
  }
  return STARTING_TIME - elapsed;
}

unsigned long getRobotTime() {
  unsigned long elapsed;
  if (gameActive && robotTurn && robotStartTime > 0) {
    elapsed = robotElapsedTime + (millis() - robotStartTime);
  } else {
    elapsed = robotElapsedTime;
  }
  
  // Count down from STARTING_TIME
  if (elapsed >= STARTING_TIME) {
    return 0;  // Time's up
  }
  return STARTING_TIME - elapsed;
}

// ==================== DISPLAY FUNCTIONS ====================
void displayStartMenu() {
  lcd.cls();
  lcd.locate(2, 1);
  lcd.printf("   > Start Game <");
  lcd.locate(3, 1);
  lcd.printf("  Press OK to begin");
}

void displayConfirmStartMenu() {
  lcd.cls();
  lcd.locate(1, 1);
  lcd.printf("  Start the game?");
  
  lcd.locate(3, 1);
  if (menuSelection == 0) {
    lcd.printf("      > Yes <");
  } else {
    lcd.printf("        Yes");
  }
  
  lcd.locate(4, 1);
  if (menuSelection == 1) {
    lcd.printf("      > No <");
  } else {
    lcd.printf("        No");
  }
}

void displayGameScreen() {
  lcd.cls();
  
  // Line 1: Turn indicator and next move
  lcd.locate(1, 1);
  if (robotTurn) {
    lcd.printf("Robot");
  } else {
    lcd.printf("You  ");
  }
  lcd.locate(1, 7);
  lcd.printf("%-4s", robotNextMove.c_str());
  lcd.locate(1, 15);
  lcd.printf("Robot");
  
  // Line 2: Timers (left-aligned for player, right-aligned for robot)
  lcd.locate(2, 1);
  formatTime(getPlayerTime());
  lcd.locate(2, 15);
  formatTime(getRobotTime());
  
  // Line 3: Scores and turn indicator
  lcd.locate(3, 1);
  lcd.printf("+%-2d", playerScore);
  lcd.locate(3, 7);
  if (robotTurn) {
    lcd.printf("Robot");
  } else {
    lcd.printf("Your ");
  }
  lcd.locate(3, 18);
  lcd.printf("+%-2d", robotScore);
  
  // Line 4: Difficulty and resign
  lcd.locate(4, 1);
  lcd.printf("STOCKFISH");
  lcd.locate(4,15);
  lcd.printf("resign");
}

void displayConfirmResignMenu() {
  lcd.cls();
  lcd.locate(1, 1);
  lcd.printf("    Resign game?");
  
  lcd.locate(3, 1);
  if (menuSelection == 0) {
    lcd.printf("      > Yes <");
  } else {
    lcd.printf("        Yes");
  }
  
  lcd.locate(4, 1);
  if (menuSelection == 1) {
    lcd.printf("      > No <");
  } else {
    lcd.printf("        No");
  }
}

void updateDisplay() {
  if (currentMenu == MENU_GAME && gameActive) {
    // Update timers without clearing screen
    lcd.locate(2, 1);
    formatTime(getPlayerTime());
    
    lcd.locate(2, 15);
    formatTime(getRobotTime());
    
    // Update turn indicator on line 1
    lcd.locate(1, 1);
    if (robotTurn) {
      lcd.printf("Robot");
    } else {
      lcd.printf("You  ");
    }
    
    // Update turn indicator on line 3
    lcd.locate(3, 7);
    if (robotTurn) {
      lcd.printf("Robot move");
    } else {
      lcd.printf("Your move");
    }
  }
}

void formatTime(unsigned long milliseconds) {
  unsigned long totalSeconds = milliseconds / 1000;
  unsigned long minutes = totalSeconds / 60;
  unsigned long seconds = totalSeconds % 60;
  
  // Format as MM:SS (exactly 5 characters)
  lcd.printf("%02lu:%02lu", minutes, seconds);
}
