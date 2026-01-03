#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);  

int analogPin = A0;
int raw = 0;
float Vin = 5.0;
float Vout = 0;
float R1 = 950.0;   
float R2 = 0;
float buffer = 0;

void setup() {
  lcd.init();        //initialize your LCD
  lcd.backlight();
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Resistance Meter");
}

void loop() {
  raw = analogRead(analogPin);
  lcd.setCursor(0, 1);

  if (raw == 0) {
    lcd.print("     Open       ");
  } else {
    buffer = raw * Vin;
    Vout = buffer / 1023.0;
    buffer = (Vin / Vout) - 1.0;
    R2 = R1 * buffer;

    if (R2 > 999.99) {
      lcd.print(R2 / 1000.0);
      lcd.print(" K Ohm    ");
    } else {
      lcd.print(R2);
      lcd.print(" Ohm      ");
    }
  }

  delay(1000);
}
