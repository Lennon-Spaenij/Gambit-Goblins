#include <LCD_I2C.h>
LCD_I2C lcd(0x27, 20, 4);

void setup() {
  lcd.begin(); // initialize the lcd
  lcd.backlight(); // backlight ON
  lcd.clear(); // clear LCD

  // Test 1: Power-up and text display on line 0
  lcd.setCursor(0, 0);
  lcd.print(" Hello World ");
  delay(1500);

  // Test 2: Verify all 4 lines
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Line 0: OK");
  lcd.setCursor(0, 1);
  lcd.print("Line 1: OK");
  lcd.setCursor(0, 2);
  lcd.print("Line 2: OK");
  lcd.setCursor(0, 3);
  lcd.print("Line 3: OK");
  delay(2000);

  // Test 3: Verify edge characters 
  lcd.clear();
  lcd.setCursor(19, 0);
  lcd.print("0");
  lcd.setCursor(19, 1);
  lcd.print("1");
  lcd.setCursor(19, 2);
  lcd.print("2");
  lcd.setCursor(19, 3);
  lcd.print("3");
  delay(2000);

  // Test 4; Dynamic update (will continue in loop)
  lcd.clear();
  lcd.setCursor(0, 0);

}

unsigned long lastUpdate = 0;
int counter = 0;

void loop() {
  // Test 4 (continued): Dynamic update test
  if (millis() - lastUpdate >= 500) {
    lastUpdate = millis();

    lcd.setCursor(0, 1);
    lcd.print("Count: ");
    lcd.print(counter);
    lcd.print("    ");

    counter++;
  }

}
