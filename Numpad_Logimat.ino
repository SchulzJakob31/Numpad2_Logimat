
//========== BLE Keyboard Mouse ==========
#include <BleCombo.h>



//========== PISO ==========
const uint8_t PISO_pinData = 15;  // Pin vom PISO von dem die Daten Seriell empfangen werden
const uint8_t PISO_pinLatch = 5;  // Pin zum PISO mit dem die Daten in das Schieberegister geladen werden
const uint8_t PISO_pinClock = 4;  // Pin zum PISO mit dem die Daten weiter getaktet wird

const uint8_t PISO_anzahl = 4;                       // Anzahl der Schieberegister
const uint8_t PISO_anzahlKanaele = PISO_anzahl * 8;  // Anzahl der Eingangskanäle aller Schieberegister
bool PISO_data[PISO_anzahlKanaele];                  // Rohe daten die vom Schieberegister empfangen wurden


void PISO_read() {
  digitalWrite(PISO_pinLatch, LOW);
  digitalWrite(PISO_pinLatch, HIGH);  // Daten ins Schieberegister laden

  for (uint8_t a = 0; a < PISO_anzahlKanaele; a++) {
    PISO_data[a] = digitalRead(PISO_pinData);  // Zustand vom ersten Eingang vom Schieberegister lesen und abspeichern
    digitalWrite(PISO_pinClock, HIGH);         // Zum nächsten Eingang weiter Takten und wiederholen
    digitalWrite(PISO_pinClock, LOW);
  }
}

//========== Debounce ==========
const uint8_t PISO_debounceDelay = 30;
bool PISO_debouncedState[PISO_anzahlKanaele];  //So lang stabil High wie Taster gedrückt
bool PISO_pressed[PISO_anzahlKanaele];  //Variale zum Triggern!!!  (für eine Loopschleife High wenn gedrückt dann wieder Low)
bool PISO_lastData[PISO_anzahlKanaele];
uint32_t PISO_lastChange[PISO_anzahlKanaele];

void PISO_debounce() {
  for (uint8_t a = 0; a < PISO_anzahlKanaele; a++) {
    PISO_pressed[a] = 0;
    if (PISO_data[a] != PISO_lastData[a]) {
      PISO_lastData[a] = PISO_data[a];
      PISO_lastChange[a] = millis();
    }
    if ((millis() - PISO_lastChange[a]) >= PISO_debounceDelay) {
      if (PISO_debouncedState[a] != PISO_data[a]) {
        PISO_debouncedState[a] = PISO_data[a];
        if (PISO_debouncedState[a]) PISO_pressed[a] = 1;
      }
    }
  }
}



//========== LCD ==========
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

uint8_t LCD_batterySymbol[6][8]{
  { B01110, B11111, B10001, B10001, B10001, B10001, B10001, B11111 },  //Symbol Batterie 0%
  { B01110, B11111, B10001, B10001, B10001, B10001, B11111, B11111 },  //Symbol Batterie 20%
  { B01110, B11111, B10001, B10001, B10001, B11111, B11111, B11111 },  //Symbol Batterie 40%
  { B01110, B11111, B10001, B10001, B11111, B11111, B11111, B11111 },  //Symbol Batterie 60%
  { B01110, B11111, B10001, B11111, B11111, B11111, B11111, B11111 },  //Symbol Batterie 80%
  { B01110, B11111, B11111, B11111, B11111, B11111, B11111, B11111 }   //Symbol Batterie 100%
};
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
  lcd.createChar(0, LCD_batterySymbol[LCD_batteryChargePostBlink]);  // Symbol je nach Batteriestand neu zuweisen
  lcd.setCursor(0, 0);                                               // Cursor an gewünschte stelle für das Batterysymbol setzen
  lcd.write(0);                                                      // Batteriesysmbol schreiben
}

void setup() {
  //Allgemein
  Serial.begin(115200);
  // LCD
  lcd.init();       // LCD initialisieren
  lcd.backlight();  // LCD Hintergrundbeleuchtung anschalten
  // BLE Keyboard Mouse
  Keyboard.begin();  // BLE Keyboard starten
  Mouse.begin();     // BLE Mouse starten

  //PISO
  pinMode(PISO_pinData, INPUT);      // PISO Shiftregister Data pin
  pinMode(PISO_pinLatch, OUTPUT);    // PISO Shiftregister Latch pin
  pinMode(PISO_pinClock, OUTPUT);    // PISO Shiftregister Clock pin
  digitalWrite(PISO_pinClock, LOW);  // ein Clockzyklus zum Initialisieren
  digitalWrite(PISO_pinClock, HIGH);
  digitalWrite(PISO_pinClock, LOW);
  // Debouncing konfigurieren
  for (uint8_t a = 0; a < PISO_anzahlKanaele; a++) {
    PISO_debouncedState[a] = PISO_data[a];
    PISO_lastData[a] = PISO_data[a];
    PISO_lastChange[a] = millis();
    PISO_pressed[a] = 0;
  }

  while (!Keyboard.isConnected()) {
  }
}

void loop() {
  PISO_read();      // Daten von Schieberegister empfangen
  PISO_debounce();  // Daten vom Schieberegister zu einzigen Impuls debouncen
  //LCD_battery();  // Batteriestand auf Display anzeigen
  //LCD_batteryIsCharging = 1;
  //LCD_batteryCharge = 1;

  if (PISO_pressed[0]) Keyboard.println("STAN");    //STAN schreiben dann Enter
  if (PISO_pressed[1]) Keyboard.print("STAN");      //STAN schreiben ohne Enter danach
  if (PISO_pressed[2]) Keyboard.write(KEY_RETURN);  //Enter drückent
  if (PISO_pressed[3]) Mouse.move(10, 10);
  if (PISO_pressed[4]) Mouse.move(-10, -10);
}

/*
  Keyboard.println("Hello World");  Text mit enter
  Keyboard.print("Hello World");  Text ohne enter
  Keyboard.write(KEY_RETURN);
  Keyboard.press(KEY_LEFT_CTRL);
  Keyboard.press(KEY_LEFT_ALT);
  Keyboard.press(KEY_DELETE);
  Keyboard.releaseAll();
  Mouse.move(10,10); Maus nach unten rechts bewegen
  Mouse.move(0,0,-1); Runter scrollen
  Mouse.click(MOUSE_LEFT);

*/
