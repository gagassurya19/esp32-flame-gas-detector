#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Define I2C pins for ESP32
#define SDA_PIN 21
#define SCL_PIN 22

// MQ-2 Sensor Pins
#define MQ2_DIGITAL_PIN 35 // D0 (Digital Output) from MQ-2

// Buzzer and LED Pins
#define BUZZER_PIN 13
#define LED_PIN 12

// LCD I2C Address (0x27 or 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 MQ-2 Gas Detector");

  // Initialize I2C for LCD
  Wire.begin(SDA_PIN, SCL_PIN);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("MQ-2 Gas Sensor");

  // Set pin modes
  pinMode(MQ2_DIGITAL_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  // Ensure LED & Buzzer are OFF at startup
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  delay(2000);
}

void loop() {
  int gasDetected = digitalRead(MQ2_DIGITAL_PIN); // Read D0 signal (HIGH/LOW)

  Serial.print("Gas Status: ");
  Serial.println(gasDetected == HIGH ? "DANGER" : "SAFE");

  // Update LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Gas Status:");

  if (gasDetected == HIGH) { // Gas detected
    Serial.println("WARNING: High Gas Level!");
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    lcd.setCursor(0, 1);
    lcd.print("ALERT!");
  } else { // Safe
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    lcd.setCursor(0, 1);
    lcd.print("Safe");
  }

  delay(1000); // Update every second
}
