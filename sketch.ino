#include <Arduino.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define NUM_BITS 8

int leds[NUM_BITS] = {13, 12, 14, 27, 26, 25, 33, 32}; // 8 LEDs

uint8_t registerData = 0b00000000; // Valor inicial (bit 0 encendido)
String inputNumber = "";           // buffer para el número ingresado

// --- Configuración del Keypad ---
const uint8_t ROWS = 4;
const uint8_t COLS = 4;
char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

uint8_t colPins[COLS] = { 16, 17, 5, 18 }; // C1–C4
uint8_t rowPins[ROWS] = { 15, 2, 0, 4 }; // R1–R4

// --- LCD I2C ---
LiquidCrystal_I2C lcd(0x27, 16, 2); // Dirección I2C típica: 0x27

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// --- Función para mostrar los bits en los LEDs ---
void showBits(uint8_t value) {
  for (int i = 0; i < NUM_BITS; i++) {
    int bitState = (value >> i) & 0x01;
    digitalWrite(leds[i], bitState);
  }
}

// --- Función para desplazar a la izquierda ---
void shiftLeft() {
  registerData <<= 1;
  if (registerData == 0)
    registerData = 0b00000001; // Reinicia al llegar al final
  showBits(registerData);
  Serial.print("Shift Left -> ");
  Serial.println(registerData, BIN);
  updateLCD();
}

// --- Función para desplazar a la derecha ---
void shiftRight() {
  registerData >>= 1;
  if (registerData == 0)
    registerData = 0b10000000; // Reinicia al llegar al final
  showBits(registerData);
  Serial.print("Shift Right -> ");
  Serial.println(registerData, BIN);
  updateLCD();
}

// --- Cargar número ingresado ---
void loadFromKeypad() {
  if (inputNumber.length() == 0) return;
  int value = inputNumber.toInt(); // convertir a número
  if (value < 0 || value > 255) {
    Serial.println("Error: el número debe estar entre 0 y 255");
    lcd.clear();
    lcd.print("Err: 0-255");
    delay(1000);
    inputNumber = "";
    registerData = 0b00000000;
    updateLCD();
    return;
  }
  registerData = (uint8_t)value;
  showBits(registerData);
  Serial.print("Load -> ");
  Serial.println(registerData, BIN);
  updateLCD();
  inputNumber = ""; // limpiar el buffer
}

// --- Mostrar valor en LCD ---
void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Dec:");
  lcd.print(registerData);
  lcd.setCursor(0, 1);
  lcd.print("Bin:");
  for (int i = 7; i >= 0; i--) {
    lcd.print((registerData >> i) & 0x01);
  }
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();

  // Configurar LEDs como salida
  for (int i = 0; i < NUM_BITS; i++) {
    pinMode(leds[i], OUTPUT);
  }

  Serial.println("Registro de 8 bits");

  lcd.setCursor(0, 0);
  lcd.print("Shift Register");
  lcd.setCursor(0, 1);
  lcd.print("Ready!");
  delay(1500);
  updateLCD();
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    if (isdigit(key)) {
      inputNumber += key;
      Serial.print("Digit: ");
      Serial.println(inputNumber);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Input: ");
      lcd.print(inputNumber);
    }
    else if (key == '#') {
      loadFromKeypad();
    }
    else if (key == 'A') {
      shiftLeft();
    }
    else if (key == 'B') {
      shiftRight();
    }
    else if (key == '*') {
      inputNumber = "";
      registerData = 0b00000000;
      Serial.println("Buffer limpiado.");
      lcd.clear();
      lcd.print("Input cleared");
      delay(500);
      updateLCD();
    }
  }
}
