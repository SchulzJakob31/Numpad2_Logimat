#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

uint8_t LCD_batterySymbol0[8] = { B01110, B11111, B10001, B10001, B10001, B10001, B10001, B11111 };  //Symbol Batterie 0%
uint8_t LCD_batterySymbol1[8] = { B01110, B11111, B10001, B10001, B10001, B10001, B11111, B11111 };  //Symbol Batterie 20%
uint8_t LCD_batterySymbol2[8] = { B01110, B11111, B10001, B10001, B10001, B11111, B11111, B11111 };  //Symbol Batterie 40%
uint8_t LCD_batterySymbol3[8] = { B01110, B11111, B10001, B10001, B11111, B11111, B11111, B11111 };  //Symbol Batterie 60%
uint8_t LCD_batterySymbol4[8] = { B01110, B11111, B10001, B11111, B11111, B11111, B11111, B11111 };  //Symbol Batterie 80%
uint8_t LCD_batterySymbol5[8] = { B01110, B11111, B11111, B11111, B11111, B11111, B11111, B11111 };  //Symbol Batterie 100%

const uint16_t LCD_batteryBlinkIntervall = 500;  //Blinkintervall der Balken der Batterie während des ladens
uint32_t LCD_batteryBlinkLastMillis;             //Letzte millis beim ändern des Blinkzustands
bool LCD_batteryBlinkState;                      //Ob der blinkende Balken gerade angezeigt wird oder nicht
uint8_t LCD_batteryChargePostBlink;              //Variable enthält Batteriestand der beim laden hoch und runter blinkt
bool LCD_batteryIsCharging = 0;                  //Ob die Batterie gerade läd oder nicht
uint8_t LCD_batteryCharge = 0;                   //Batteriestand der Batterie

void LCD_battery() {
  if (LCD_batteryIsCharging == 0) LCD_batteryChargePostBlink = LCD_batteryCharge;  //Wenn die Batterie nicht läd nicht blinken lassen
  else {                                                                           //Wenn die Batterie läd blinken lassen
    if (millis() - LCD_batteryBlinkLastMillis >= LCD_batteryBlinkIntervall) {      //Variable LCD_batteryBlinkState blinken lassen
      LCD_batteryBlinkState = !LCD_batteryBlinkState;
      LCD_batteryBlinkLastMillis = millis();
    }
    if (LCD_batteryBlinkState == 0) LCD_batteryChargePostBlink = LCD_batteryCharge;  // Bei 0 von Variable LCD_batteryBlinkState normalen Batteriestand (ohne Blink an) anzeigen
    else LCD_batteryChargePostBlink = LCD_batteryCharge + 1;                         // Bei 1 von Variable LCD_batteryBlinkState Batteriestand (mit Blink an) anzeigen
  }
  switch (LCD_batteryChargePostBlink) {  //Symbol je nach Batteriestand neu zuweisen
    case 0:
      lcd.createChar(0, LCD_batterySymbol0);
      break;
    case 1:
      lcd.createChar(0, LCD_batterySymbol1);
      break;
    case 2:
      lcd.createChar(0, LCD_batterySymbol2);
      break;
    case 3:
      lcd.createChar(0, LCD_batterySymbol3);
      break;
    case 4:
      lcd.createChar(0, LCD_batterySymbol4);
      break;
    case 5:
      lcd.createChar(0, LCD_batterySymbol5);
      break;
  }
  lcd.setCursor(0, 0);  //Cursor an gewünschte stelle für das Batterysymbol setzen
  lcd.write(0);         //Batteriesysmbol schreiben
}

void setup() {
  lcd.init();
  lcd.backlight();
}

void loop() {
  LCD_battery();

  LCD_batteryIsCharging = 1;
  LCD_batteryCharge = 1;
}
