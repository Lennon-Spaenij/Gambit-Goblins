#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Set address to 0x27 (standard) or 0x3F
LiquidCrystal_I2C lcd(0x27, 20, 4); 

unsigned long startTime;
int counter = 0;

void setup() {
  startTime = millis(); // Start timing immediately for Test 1
  
  Serial.begin(9600);
  // Note: Uno Q serial might be slow to connect, but code runs immediately
  
  // Initialize I2C - High speed (400kHz) is better for <200ms boot, 
  // but 100kHz is safer for CNC shield noise. Let's use standard:
  Wire.begin();
  Wire.setClock(100000); 
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Test 1: Power-up & basic text (<200ms requirement)
  unsigned long bootTime = millis() - startTime;
  lcd.setCursor(0, 0);
  lcd.print("Hello World "); 
  lcd.print(bootTime);
  lcd.print("ms");

  // Test 2: All 4 lines
  lcd.setCursor(0, 1); lcd.print("Line 1: OK");
  lcd.setCursor(0, 2); lcd.print("Line 2: OK");
  lcd.setCursor(0, 3); lcd.print("Line 3: OK");

  // Test 3: Edge characters (Column 19 markers)
  for(int i = 0; i < 4; i++) {
    lcd.setCursor(19, i);
    lcd.print("*");
  }

  // Serial Logging
  Serial.println("--- Acceptance Test Data ---");
  Serial.print("Boot Time: "); Serial.print(bootTime); Serial.println("ms (Target <200ms)");
  Serial.println("Lines 0-3: Written");
  Serial.println("Col 19: Marked with *");
}

void loop() {
  // Test 4: Dynamic updates (500ms counter, no flicker)
  lcd.setCursor(12, 3);
  lcd.print("C:");
  if (counter < 10) lcd.print("0"); // Padding for stability
  lcd.print(counter);

  Serial.print("Dynamic Counter: ");
  Serial.println(counter);

  counter++;
  if(counter > 99) counter = 0; // Reset for visual stability
  
  delay(500); // Test 4 timing requirement
}
