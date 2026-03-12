// ALLE 3 MOTOREN TESTEN - Non-blocking met millis()
#define X_STEP 2
#define X_DIR 5
#define Y_STEP 3
#define Y_DIR 6
#define Z_STEP 4
#define Z_DIR 7
#define ENABLE 8

#define STEPS_PER_REV 3200
#define STEP_DELAY 100  // microseconds

// Variabelen voor timing
unsigned long previousMicros = 0;
int currentMotor = 0;  // 0=X, 1=Y, 2=Z
int currentStep = 0;
bool stepState = false;
bool forward = true;

void setup() {
  // Alle pins configureren
  pinMode(X_STEP, OUTPUT);
  pinMode(X_DIR, OUTPUT);
  pinMode(Y_STEP, OUTPUT);
  pinMode(Y_DIR, OUTPUT);
  pinMode(Z_STEP, OUTPUT);
  pinMode(Z_DIR, OUTPUT);
  pinMode(ENABLE, OUTPUT);
  
  digitalWrite(ENABLE, LOW);  // Drivers AAN
  
  // Start richting
  digitalWrite(X_DIR, HIGH);
  digitalWrite(Y_DIR, HIGH);
  digitalWrite(Z_DIR, HIGH);
  
  Serial.begin(9600);
  Serial.println("=== TEST ALLE 3 MOTOREN ===");
  Serial.println("Motor X -> Motor Y -> Motor Z -> herhaal");
  Serial.println("");
}

void loop() {
  unsigned long currentMicros = micros();
  
  // Check of het tijd is voor volgende stap
  if (currentMicros - previousMicros >= STEP_DELAY) {
    previousMicros = currentMicros;
    
    // Welke motor pin gebruiken?
    int stepPin;
    int dirPin;
    String motorName;
    
    switch(currentMotor) {
      case 0:
        stepPin = X_STEP;
        dirPin = X_DIR;
        motorName = "X";
        break;
      case 1:
        stepPin = Y_STEP;
        dirPin = Y_DIR;
        motorName = "Y";
        break;
      case 2:
        stepPin = Z_STEP;
        dirPin = Z_DIR;
        motorName = "Z";
        break;
    }
    
    // Maak stap
    if (stepState) {
      digitalWrite(stepPin, HIGH);
      stepState = false;
    } else {
      digitalWrite(stepPin, LOW);
      stepState = true;
      currentStep++;
      
      // Elke 800 stappen een update (elke 90 graden)
      if (currentStep % 800 == 0) {
        Serial.print("Motor ");
        Serial.print(motorName);
        Serial.print(" - Stap: ");
        Serial.println(currentStep);
      }
    }
    
    // Check of rotatie compleet is
    if (currentStep >= STEPS_PER_REV) {
      currentStep = 0;
      
      if (forward) {
        // Wissel richting
        Serial.print("Motor ");
        Serial.print(motorName);
        Serial.println(" - Wissel richting (achteruit)");
        digitalWrite(dirPin, LOW);
        forward = false;
        delay(1000);  // 1 sec pauze
        
      } else {
        // Ga naar volgende motor
        Serial.print("Motor ");
        Serial.print(motorName);
        Serial.println(" - KLAAR!");
        Serial.println("");
        
        forward = true;
        digitalWrite(dirPin, HIGH);
        
        currentMotor++;
        if (currentMotor > 2) {
          currentMotor = 0;  // Reset naar X
          Serial.println("=== ALLE MOTOREN GETEST - OPNIEUW ===");
          Serial.println("");
          delay(2000);  // 2 sec pauze tussen cycli
        } else {
          delay(1000);  // 1 sec tussen motoren
        }
      }
    }
  }
}
