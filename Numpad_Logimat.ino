//========== BLE Keyboard Mouse ==========
#include <BleCombo.h>

//========== NumPad allgemein ==========
bool Shift_state = 0;            // Zustand von Shift (an oder aus)
const uint8_t Shift_button = 7;  //PISO_Kanal an der Shift angeschlossen ist

//========== PISO ==========
const uint8_t PISO_pinData = 10;   // Pin vom PISO von dem die Daten Seriell empfangen werden
const uint8_t PISO_pinLatch = 11;  // Pin zum PISO mit dem die Daten in das Schieberegister geladen werden
const uint8_t PISO_pinClock = 12;  // Pin zum PISO mit dem die Daten weiter getaktet wird

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

bool Button_pressed[PISO_anzahlKanaele];

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
  Return,
  Loeschen,
  F1,
  F2,
  F3,
  F4,
  F5,
  F6,
  F7,
  F8,
  F9,
  F10,
  Empty1,
  Empty2,
  Empty3,
  Empty4,
  Empty5,
  Empty6,
  Empty7,
  Empyt8,
  Empty9,
  Empty10
};

void Button_pressedTranslate() {
  Button_pressed[Num0] = PISO_pressed[23];
  Button_pressed[Num1] = PISO_pressed[22];
  Button_pressed[Num2] = PISO_pressed[11];
  Button_pressed[Num3] = PISO_pressed[14];
  Button_pressed[Num4] = PISO_pressed[21];
  Button_pressed[Num5] = PISO_pressed[20];
  Button_pressed[Num6] = PISO_pressed[7];
  Button_pressed[Num7] = PISO_pressed[8];
  Button_pressed[Num8] = PISO_pressed[9];
  Button_pressed[Num9] = PISO_pressed[3];
  Button_pressed[Return] = PISO_pressed[1];
  Button_pressed[Loeschen] = PISO_pressed[2];
  Button_pressed[F1] = PISO_pressed[18];
  Button_pressed[F2] = PISO_pressed[19];
  Button_pressed[F3] = PISO_pressed[13];
  Button_pressed[F4] = PISO_pressed[0];
  Button_pressed[F5] = PISO_pressed[15];
  Button_pressed[F6] = PISO_pressed[10];
  Button_pressed[F7] = PISO_pressed[12];
  Button_pressed[F8] = PISO_pressed[6];
  Button_pressed[F9] = PISO_pressed[4];
  Button_pressed[F10] = PISO_pressed[5];
}


// ========== LCD ==========
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Batterie
uint8_t LCD_batterySymbol[6][8]{
  { B01110, B11111, B10001, B10001, B10001, B10001, B10001, B11111 },  // Symbol Batterie 0%
  { B01110, B11111, B10001, B10001, B10001, B10001, B11111, B11111 },  // Symbol Batterie 20%
  { B01110, B11111, B10001, B10001, B10001, B11111, B11111, B11111 },  // Symbol Batterie 40%
  { B01110, B11111, B10001, B10001, B11111, B11111, B11111, B11111 },  // Symbol Batterie 60%
  { B01110, B11111, B10001, B11111, B11111, B11111, B11111, B11111 },  // Symbol Batterie 80%
  { B01110, B11111, B11111, B11111, B11111, B11111, B11111, B11111 }   // Symbol Batterie 100%
};
const uint16_t LCD_batteryBlinkIntervall = 600;  // Blinkintervall der Balken der Batterie während des ladens
uint32_t LCD_batteryBlinkLastMillis;             // Letzte millis beim ändern des Blinkzustands
bool LCD_batteryBlinkState;                      // Ob der blinkende Balken gerade angezeigt wird oder nicht
uint8_t LCD_batteryChargePostBlink;              // Variable enthält Batteriestand der beim laden hoch und runter blinkt
bool LCD_batteryIsCharging = 0;                  // Ob die Batterie gerade läd oder nicht
uint8_t LCD_batteryCharge = 0;                   // Batteriestand der Batterie
uint8_t LCD_batterySymbolPosX = 0;               // X Position der Batterieanzeige (manipuliert durch LCD_battery Funktion)
uint8_t LCD_batterySymbolPosY = 0;               // Y Position der Batterieanzeige (manipuliert durch LCD_battery Funktion)
bool LCD_batterySymbolShow = 0;                  // Ob das Batterysymbol angezeigt wird (manipuliert durch LCD_battery Funktion)
bool LCD_batterySymbolReset = 0;                 // Ob nach dem nicht mehr angezeigt wird das Zeichen bereits blank gemacht wurde (wird intern Manipuliert)

void LCD_battery(uint8_t Charge, bool Charging, uint8_t Pos_X, uint8_t Pos_Y, bool show) {
  LCD_batterySymbolShow = show;
  LCD_batteryCharge = Charge;
  LCD_batteryIsCharging = Charging;
  LCD_batterySymbolPosX = Pos_X;
  LCD_batterySymbolPosY = Pos_Y;
}

void LCD_batteryDisplay() {
  if (!LCD_batteryIsCharging) {                                   // Code wenn die Batterie nicht geladen wird
    lcd.createChar(0, LCD_batterySymbol[LCD_batteryCharge]);      // Ladezustand 1 zu 1 auf display übertragen (nur Custom Zeichen erstellen)
    lcd.setCursor(LCD_batterySymbolPosX, LCD_batterySymbolPosY);  // Zeichen an gewünschte Position setzten
    lcd.write(0);                                                 // Batteriesymbol schreiben
  }

  if (LCD_batteryIsCharging) {                                                 // Code wenn die Batterie geladen wird
    if (millis() - LCD_batteryBlinkLastMillis >= LCD_batteryBlinkIntervall) {  // Code alle [intervall]ms ausführen
      LCD_batteryChargePostBlink++;                                            // Anzuzeigende Batterieladung sequenziell inkrementieren
      if (LCD_batteryChargePostBlink > 5) LCD_batteryChargePostBlink = 0;      //Sobald höchste Batterieladung angezeigt wird wieder auf 0 setzen
      LCD_batteryBlinkLastMillis = millis();
    }
    lcd.createChar(0, LCD_batterySymbol[LCD_batteryChargePostBlink]);  // Ladezustand nach Lauflicht auf display übertragen (nur Custom Zeichen erstellen)
    lcd.setCursor(LCD_batterySymbolPosX, LCD_batterySymbolPosY);       // Zeichen an gewünschte Position setzten
    lcd.write(0);                                                      // Batteriesymbol schreiben
  }
  LCD_batterySymbolReset = 0;  // Variable setzen damit bei umschalten zu -kein Batteriesymbol- die Funktion weiß dass das Symbol noch zurückgesetzt werden muss
}

// Allgemein
const uint8_t LCD_updateRate = 200;  // Intervall mit dem das LCD geupdated wird
uint32_t LCD_lastUpdate = 0;         // Wann das LCD das letzte mal geupdated wurde

// Funktion zum ausführen der Batterieanzeige
void LCD_update() {
  if (millis() - LCD_lastUpdate >= LCD_updateRate) {  // Code nur alle [LCD_updateRate]ms ausführen
    LCD_lastUpdate = millis();                        // Allgemein für Intervall

    // Code für Batterieanzeige
    if (LCD_batterySymbolShow)                                      // Wenn das Symbol angezeigt werden soll
      LCD_batteryDisplay();                                         // Batteriestand auf Display anzeigen
    if (!LCD_batterySymbolShow && !LCD_batterySymbolReset) {        // Wenn das Symbol nicht angezeigt werden soll UND das Symbol noch nicht zurückgesetzt wurde
      lcd.setCursor(LCD_batterySymbolPosX, LCD_batterySymbolPosY);  // Cursor an Position des Symbols bewegen
      lcd.print(" ");                                               // Symbol zurücksetzen
      LCD_batterySymbolReset = 1;                                   // Speichern dass das Symbol zyrückgesetzt wurde
    }
  }
}

void setup() {
  //Allgemein
  Serial.begin(115200);
  pinMode(13, OUTPUT);  // Statusanzeige für Shift / Bluetooth im Setup verbunden

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
  digitalWrite(13, HIGH);
  while (!Keyboard.isConnected()) {  // Erst in den Loop übergehen wenn Bluetooth verbunden ist
  }
  digitalWrite(13, LOW);
}

void loop() {
  PISO_read();      // Daten von Schieberegister empfangen
  PISO_debounce();  // Daten vom Schieberegister zu einzigen Impuls debouncen
  Button_pressedTranslate();
  LCD_update();

  LCD_battery(1, 1, 15, 0, 1);  //Batteriesymbol: (Ladestand, Ladevorgang, X Position, Y Position, Anzeigen)
  
  if (Button_pressed[Num0]) Keyboard.print("0");
  if (Button_pressed[Num1]) Keyboard.print("1");
  if (Button_pressed[Num2]) Keyboard.print("2");
  if (Button_pressed[Num3]) Keyboard.print("3");
  if (Button_pressed[Num4]) Keyboard.print("4");
  if (Button_pressed[Num5]) Keyboard.print("5");
  if (Button_pressed[Num6]) Keyboard.print("6");
  if (Button_pressed[Num7]) Keyboard.print("7");
  if (Button_pressed[Num8]) Keyboard.print("8");
  if (Button_pressed[Num9]) Keyboard.print("9");
  if (Button_pressed[Return]) Keyboard.println();

}
  /*
  for (uint8_t a = 0; a < PISO_anzahlKanaele; a++)
    if (PISO_pressed[a] && a != Shift_button) Shift_state = 0;  // Wenn belibige Taste bis auf Shift gedrückt wird Shift deaktivieren
  if (PISO_pressed[Shift_button]) Shift_state = !Shift_state;   // Wenn Shift gedrückt wird Shift umschalten
  digitalWrite(13, Shift_state);                                // Zustand von Schift mit LED anzeigen




  if (!Shift_state) {
    if (PISO_pressed[Num0]) {
      Keyboard.println("STAN");  // STAN schreiben dann Enter
      Serial.println("STAN sent");
    }
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
  //}

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
