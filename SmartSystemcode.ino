#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino_HS300x.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- Pin definitions (Analog)---
const int SOIL_PIN = A0;
const int JOY_X    = A2;
const int LDR_PIN  = A1; 

const int LED_R = 5;     
const int LED_G = 4;
const int LED_B = 3;

// --- Joystick control ---
const int JOY_LOW         = 300;
const int JOY_HIGH        = 700;
const int JOY_CENTER_LOW  = 400;
const int JOY_CENTER_HIGH = 600;
bool joystickMoved = false;

// --- Screen st ---
int currentScreen = 0;
const int NUM_SCREENS = 2;

// --- Sensor state ---
bool sensorOK = false;
unsigned long lastReconnect = 0;
const unsigned long RECONNECT_INTERVAL = 5000;

// --- Timing ---
unsigned long lastUpdate = 0;
const unsigned long UPDATE_INTERVAL = 2000;

// -------------------------------------------------------
void setRGB(int r, int g, int b) {
  analogWrite(LED_R, r);
  analogWrite(LED_G, g);
  analogWrite(LED_B, b);
}

// -------------------------------------------------------
void updateLDR() {
  int ldr = analogRead(LDR_PIN);
  Serial.print("LDR: "); Serial.println(ldr);

  if (ldr < 200) {
    setRGB(0, 255, 0);   // bright — green
  } else if (ldr < 420) {
    setRGB(0, 0, 255);   // mid — blue
  } else {
    setRGB(255, 0, 0);   // dark — red
  }
}

// -------------------------------------------------------
bool initSensor() {
  if (HS300x.begin()) {
    Serial.println("HS3003 connected OK");
    return true;
  }
  Serial.println("HS3003 not found");
  return false;
}

// -------------------------------------------------------
void showTempHumidity() {
  if (!sensorOK) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor offline");
    lcd.setCursor(0, 1);
    lcd.print("Reconnecting...");
    return;
  }

  float temp = HS300x.readTemperature();
  float hum  = HS300x.readHumidity();

  if (isnan(temp) || temp < -40 || temp > 125) {
    sensorOK = false;
    return;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temp, 1);
  lcd.print(" C");

  lcd.setCursor(0, 1);
  lcd.print("Hum:  ");
  lcd.print(hum, 1);
  lcd.print(" %");

  Serial.print("Temp: "); Serial.print(temp);
  Serial.print(" C  Hum: "); Serial.println(hum);
}

// -------------------------------------------------------
void showSoilHumidity() {
  int raw = analogRead(SOIL_PIN);
  int soilPct = map(raw, 1023, 0, 0, 100);
  soilPct = constrain(soilPct, 0, 100);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Soil Humidity");
  lcd.setCursor(0, 1);
  lcd.print("Level: ");
  lcd.print(soilPct);
  lcd.print(" %");

  Serial.print("Soil: "); Serial.println(soilPct);
}

// -------------------------------------------------------
void handleJoystick() {
  int xVal = analogRead(JOY_X);

  if (xVal < JOY_LOW && !joystickMoved) {
    currentScreen = (currentScreen - 1 + NUM_SCREENS) % NUM_SCREENS;
    joystickMoved = true;
    lastUpdate = 0;
    Serial.println("Joystick LEFT");

  } else if (xVal > JOY_HIGH && !joystickMoved) {
    currentScreen = (currentScreen + 1) % NUM_SCREENS;
    joystickMoved = true;
    lastUpdate = 0;
    Serial.println("Joystick RIGHT");

  } else if (xVal >= JOY_CENTER_LOW && xVal <= JOY_CENTER_HIGH) {
    joystickMoved = false;
  }
}

// -------------------------------------------------------
void tryReconnectSensor() {
  unsigned long now = millis();
  if (!sensorOK && now - lastReconnect >= RECONNECT_INTERVAL) {
    lastReconnect = now;
    Serial.println("Attempting HS3003 reconnect...");
    Wire.end();
    delay(100);
    Wire.begin();
    Wire.setClock(100000);
    sensorOK = initSensor();
  }
}

// -------------------------------------------------------
void setup() {
  Serial.begin(115200);  // ← your working baud rate
  Wire.begin();
  Wire.setClock(100000);

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Smart Home v1.0");
  delay(2000);

  sensorOK = initSensor();
  if (!sensorOK) {
    lcd.clear();
    lcd.print("Sensor Error!");
    lcd.setCursor(0, 1);
    lcd.print("Will retry...");
    delay(2000);
  } else {
    lcd.clear();
    lcd.print("System Ready!");
    delay(1500);
  }
}

// -------------------------------------------------------
void loop() {
  tryReconnectSensor();
  handleJoystick();
  updateLDR();        

  unsigned long now = millis();
  if (now - lastUpdate >= UPDATE_INTERVAL) {
    lastUpdate = now;

    switch (currentScreen) {
      case 0: showTempHumidity(); break;
      case 1: showSoilHumidity(); break;
    }

    lcd.setCursor(14, 1);
    lcd.print(currentScreen + 1);
    lcd.print("/");
    lcd.print(NUM_SCREENS);
  }
}
