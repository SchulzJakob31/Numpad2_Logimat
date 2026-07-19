#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

uint8_t Battery_0[8] = { B01110, B11111, B10001, B10001, B10001, B10001, B10001, B11111 };
uint8_t Battery_1[8] = { B01110, B11111, B10001, B10001, B10001, B10001, B11111, B11111 };
uint8_t Battery_2[8] = { B01110, B11111, B10001, B10001, B10001, B11111, B11111, B11111 };
uint8_t Battery_3[8] = { B01110, B11111, B10001, B10001, B11111, B11111, B11111, B11111 };
uint8_t Battery_4[8] = { B01110, B11111, B10001, B11111, B11111, B11111, B11111, B11111 };
uint8_t Battery_5[8] = { B01110, B11111, B11111, B11111, B11111, B11111, B11111, B11111 };

bool blink = 0;
uint32_t last = 0;
const uint16_t intervall = 500;

void LCD_battery(bool charging, uint8_t charge) {
  if (charging == 0) {
    switch (charge) {
      case 0:
        lcd.createChar(0, Battery_0);
        break;
      case 1:
        lcd.createChar(0, Battery_1);
        break;
      case 2:
        lcd.createChar(0, Battery_2);
        break;
      case 3:
        lcd.createChar(0, Battery_3);
        break;
      case 4:
        lcd.createChar(0, Battery_4);
        break;
      case 5:
        lcd.createChar(0, Battery_5);
        break;
    }
  } else if (charging == 1) {

    if (millis() - last >= intervall) {
      blink = !blink;
      last = millis();
    }

    switch (charge) {
      case 0:
        if (blink == 0) lcd.createChar(0, Battery_0);
        else lcd.createChar(0, Battery_1);
        break;
      case 1:
        if (blink == 0) lcd.createChar(0, Battery_1);
        else lcd.createChar(0, Battery_2);
        break;
      case 2:
        if (blink == 0) lcd.createChar(0, Battery_2);
        else lcd.createChar(0, Battery_3);
        break;
      case 3:
        if (blink == 0) lcd.createChar(0, Battery_3);
        else lcd.createChar(0, Battery_4);
        break;
      case 4:
        if (blink == 0) lcd.createChar(0, Battery_4);
        else lcd.createChar(0, Battery_5);
        break;
      case 5:
        if (blink == 0) lcd.createChar(0, Battery_5);
        else lcd.createChar(0, Battery_1);
        break;
    }
  }
  lcd.setCursor(0, 0);
  lcd.write(0);
}

void setup() {
  lcd.init();
  lcd.backlight();
}

void loop() {
  LCD_battery(1, 0);
}
