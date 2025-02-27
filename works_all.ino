#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Define I2C pins for ESP32
#define SDA_PIN 21
#define SCL_PIN 22

// Sensor Pins
#define MQ2_DIGITAL_PIN 35  // D0 (Digital Output) from MQ-2
#define FLAME_SENSOR_PIN 19 // D0 (Digital Output) from Flame Sensor

// Buzzer and LED Pins
#define BUZZER_PIN 13
#define LED_PIN 12

// LCD I2C Address (0x27 or 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Gas & Flame Detector");

  // Initialize I2C for LCD
  Wire.begin(SDA_PIN, SCL_PIN);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Gas & Flame Sensor");

  // Set pin modes
  pinMode(MQ2_DIGITAL_PIN, INPUT);
  pinMode(FLAME_SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  // Ensure LED & Buzzer are OFF at startup
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  delay(2000);
}

void loop() {
  int gasDetected = digitalRead(MQ2_DIGITAL_PIN);    // Read MQ-2 Gas Sensor
  int flameDetected = digitalRead(FLAME_SENSOR_PIN); // Read Flame Sensor

  Serial.print("Gas Status: ");
  Serial.println(gasDetected == HIGH ? "DANGER" : "SAFE");

  Serial.print("Flame Status: ");
  Serial.println(flameDetected != LOW ? "FIRE DETECTED" : "NO FIRE");

  // Update LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Status:");

  if (gasDetected == HIGH || flameDetected != LOW) { // Gas or Fire detected
    Serial.println("WARNING: Gas/Flame Detected!");
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    
    lcd.setCursor(0, 1);
    if (gasDetected == HIGH && flameDetected != LOW) {
      lcd.print("Gas & Fire ON!");
    } else if (gasDetected == HIGH) {
      lcd.print("Gas ON, Fire OFF");
    } else {
      lcd.print("Fire ON, Gas OFF");
    }
  } else { // Safe Condition
    Serial.println("SAFE: No Gas or Fire");
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    
    lcd.setCursor(0, 1);
    lcd.print("Gas & Fire OFF");
  }

  delay(1000); // Update every second
}
