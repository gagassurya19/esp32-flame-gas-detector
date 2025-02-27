#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// Define pins
#define GAS_PIN A0
#define FLAME_PIN A2
#define BUZZER_PIN 2
#define ESP_RX 3
#define ESP_TX 4

// Initialize LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set LCD I2C address (0x27 is common, adjust if needed)

// Initialize software serial for communication with ESP8266
SoftwareSerial espSerial(ESP_RX, ESP_TX); // RX, TX

// Variables for sensor readings
int gasRead, flameRead;
bool gasAlarm = false;
bool flameAlarm = false;
bool prevGasAlarm = false;
bool prevFlameAlarm = false;

void setup() {
  // Initialize pins
  pinMode(GAS_PIN, INPUT);       // Gas sensor pin
  pinMode(FLAME_PIN, INPUT);     // Flame sensor pin
  pinMode(BUZZER_PIN, OUTPUT);   // Buzzer pin
  
  // Initialize serial communications
  Serial.begin(9600);            // For debugging
  espSerial.begin(9600);         // For communication with ESP8266
  
  // Initialize LCD
  lcd.init();                    // Initialize the LCD
  lcd.backlight();               // Turn on the LCD backlight
  
  // Display startup message
  lcd.setCursor(0, 0);
  lcd.print("System starting");
  lcd.setCursor(0, 1);
  lcd.print("Please wait...");
  
  delay(2000);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Gas & Flame");
  lcd.setCursor(0, 1);
  lcd.print("Detection System");
  
  delay(1000);
  lcd.clear();
}

void loop() {
  // Read sensor values
  gasRead = digitalRead(GAS_PIN);
  flameRead = analogRead(FLAME_PIN);
  
  // Debug info
  Serial.print("Gas: ");
  Serial.print(gasRead);
  Serial.print("   Flame: ");
  Serial.println(flameRead);
  
  // Determine alarm states
  gasAlarm = (gasRead == 1);
  flameAlarm = (flameRead > 200);
  
  // Update LCD and set buzzer based on alarm conditions
  updateAlarms();
  
  // Check if alarm state changed and send to ESP8266 if it did
  if (gasAlarm != prevGasAlarm || flameAlarm != prevFlameAlarm) {
    sendAlertToESP();
    prevGasAlarm = gasAlarm;
    prevFlameAlarm = flameAlarm;
  }
  
  delay(500); // Short delay to prevent continuous sending
}

// Update LCD and buzzer based on current alarm states
void updateAlarms() {
  lcd.setCursor(0, 0);
  lcd.print("Gas alarm: ");
  lcd.print(gasAlarm ? "ON " : "OFF");
  
  lcd.setCursor(0, 1);
  lcd.print("Flame alarm: ");
  lcd.print(flameAlarm ? "ON " : "OFF");
  
  if (gasAlarm && flameAlarm) {
    // Both alarms
    tone(BUZZER_PIN, 1836);
  } else if (gasAlarm) {
    // Gas alarm only
    tone(BUZZER_PIN, 1258);
  } else if (flameAlarm) {
    // Flame alarm only
    tone(BUZZER_PIN, 2568);
  } else {
    // No alarms
    noTone(BUZZER_PIN);
  }
}

// Send current alarm status to ESP8266
void sendAlertToESP() {
  String alertMsg = "A:";
  
  if (gasAlarm && flameAlarm) {
    alertMsg += "3"; // Both gas and flame
  } else if (gasAlarm) {
    alertMsg += "1"; // Gas only
  } else if (flameAlarm) {
    alertMsg += "2"; // Flame only
  } else {
    alertMsg += "0"; // All clear
  }
  
  espSerial.println(alertMsg);
  Serial.println("Sent to ESP: " + alertMsg);
}
