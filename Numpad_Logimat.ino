
//========== BLE Keyboard Mouse ==========
#include <BleCombo.h>

//========== NumPad allgemein ==========
bool Shift_state = 0;            // Zustand von Shift (an oder aus)
const uint8_t Shift_button = 7;  //PISO_Kanal an der Shift angeschlossen ist

//========== PISO ==========
const uint8_t PISO_pinData = 15;  // Pin vom PISO von dem die Daten Seriell empfangen werden
const uint8_t PISO_pinLatch = 5;  // Pin zum PISO mit dem die Daten in das Schieberegister geladen werden
const uint8_t PISO_pinClock = 4;  // Pin zum PISO mit dem die Daten weiter getaktet wird

const uint8_t PISO_anzahl = 4;                       // Anzahl der Schieberegister
const uint8_t PISO_anzahlKanaele = PISO_anzahl * 8;  // Anzahl der Eingangskanäle aller Schieberegister
bool PISO_data[PISO_anzahlKanaele];                  // Rohe daten die vom Schieberegister empfangen wurden

enum PISO_anzahlKanaele {
  Num0,
  Num1,
  Num2,
  Num3,
  Num4,
  Num5,
  Num6,
  Num7,
  Num8,
  Num9,
  Num10,
  Num11,
  Num12,
  Num13,
  Num14,
  Num15,
  Num16,
  Num17,
  Num18,
  Num19,
  Num20,
  Num21,
  Num22,
  Num23,
  Num24,
  Num25,
  Num26,
  Num27,
  Num28,
  Num29,
  Num30,
  Num31
};


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
bool PISO_debouncedState[PISO_anzahlKanaele];  // So lang stabil High wie Taster gedrückt
bool PISO_pressed[PISO_anzahlKanaele];         // Variale zum Triggern!!!  (für eine Loopschleife High wenn gedrückt dann wieder Low)
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
  { B01110, B11111, B10001, B10001, B10001, B10001, B10001, B11111 },  // Symbol Batterie 0%
  { B01110, B11111, B10001, B10001, B10001, B10001, B11111, B11111 },  // Symbol Batterie 20%
  { B01110, B11111, B10001, B10001, B10001, B11111, B11111, B11111 },  // Symbol Batterie 40%
  { B01110, B11111, B10001, B10001, B11111, B11111, B11111, B11111 },  // Symbol Batterie 60%
  { B01110, B11111, B10001, B11111, B11111, B11111, B11111, B11111 },  // Symbol Batterie 80%
  { B01110, B11111, B11111, B11111, B11111, B11111, B11111, B11111 }   // Symbol Batterie 100%
};
const uint16_t LCD_batteryBlinkIntervall = 500;  // Blinkintervall der Balken der Batterie während des ladens
uint32_t LCD_batteryBlinkLastMillis;             // Letzte millis beim ändern des Blinkzustands
bool LCD_batteryBlinkState;                      // Ob der blinkende Balken gerade angezeigt wird oder nicht
uint8_t LCD_batteryChargePostBlink;              // Variable enthält Batteriestand der beim laden hoch und runter blinkt
bool LCD_batteryIsCharging = 0;                  // Ob die Batterie gerade läd oder nicht
uint8_t LCD_batteryCharge = 0;                   // Batteriestand der Batterie

void LCD_battery() {
  if (LCD_batteryIsCharging == 0) LCD_batteryChargePostBlink = LCD_batteryCharge;  // Wenn die Batterie nicht läd nicht blinken lassen
  else {                                                                           // Wenn die Batterie läd blinken lassen
    if (millis() - LCD_batteryBlinkLastMillis >= LCD_batteryBlinkIntervall) {      // Variable LCD_batteryBlinkState blinken lassen
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
  pinMode(2, OUTPUT);  // Statusanzeige für Shift
  
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

  while (!Keyboard.isConnected()) {  // Erst in den Loop übergehen wenn Bluetooth verbunden ist
  }
}

void loop() {
  PISO_read();      // Daten von Schieberegister empfangen
  PISO_debounce();  // Daten vom Schieberegister zu einzigen Impuls debouncen
  //LCD_battery();  // Batteriestand auf Display anzeigen
  //LCD_batteryIsCharging = 1;
  //LCD_batteryCharge = 1;

  for (uint8_t a = 0; a < PISO_anzahlKanaele; a++)
    if (PISO_pressed[a] && a != Shift_button) Shift_state = 0;  // Wenn belibige Taste bis auf Shift gedrückt wird Shift deaktivieren
  if (PISO_pressed[Shift_button]) Shift_state = !Shift_state;   // Wenn Shift gedrückt wird Shift umschalten
  digitalWrite(2, Shift_state);                                 // Zustand von Schift mit LED anzeigen


  if(!Shift_state) {
  if (PISO_pressed[Num0]) Keyboard.println("STAN");    // STAN schreiben dann Enter
  if (PISO_pressed[Num1]) Keyboard.print("STAN");      // STAN schreiben ohne Enter danach
  if (PISO_pressed[Num2]) Keyboard.write(KEY_RETURN);  // Enter drückent
  if (PISO_pressed[Num3]) Mouse.move(10, 10);
  if (PISO_pressed[Num4]) Mouse.move(-10, -10);
  if (PISO_pressed[Num5]) Keyboard.print("STAN0");
  if (PISO_pressed[Num6]) Keyboard.print("STAN00");
  //if (PISO_pressed[Num7])  // nicht verwendbar da Shift Taste
  /*
  if (PISO_pressed[Num8])
  if (PISO_pressed[Num9])
  if (PISO_pressed[Num10])
  if (PISO_pressed[Num11])
  if (PISO_pressed[Num12])
  if (PISO_pressed[Num13])
  if (PISO_pressed[Num14])
  if (PISO_pressed[Num15])
  if (PISO_pressed[Num16])
  if (PISO_pressed[Num17])
  if (PISO_pressed[Num18])
  if (PISO_pressed[Num19])
  if (PISO_pressed[Num20])
  if (PISO_pressed[Num21])
  if (PISO_pressed[Num22])
  if (PISO_pressed[Num23])
  if (PISO_pressed[Num24])
  if (PISO_pressed[Num25])
  if (PISO_pressed[Num26])
  if (PISO_pressed[Num27])
  if (PISO_pressed[Num28])
  if (PISO_pressed[Num29])
  if (PISO_pressed[Num30])
  if (PISO_pressed[Num31])
  */
  }
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
