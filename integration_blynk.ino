#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WebServer.h>
#include <BlynkSimpleEsp32.h>

// 🛜 WiFi Credentials
const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";

// 🔑 Blynk Authentication Token
#define BLYNK_AUTH_TOKEN "your_blynk_token"

// 🌐 Web Server
WebServer server(80);

// 🔌 I2C Pins for ESP32
#define SDA_PIN 21
#define SCL_PIN 22

// 🔥 Sensor Pins
#define MQ2_DIGITAL_PIN 35  // Gas Sensor
#define FLAME_SENSOR_PIN 19 // Flame Sensor

// 🔔 Buzzer and LED Pins
#define BUZZER_PIN 13
#define LED_PIN 12

// 📺 LCD I2C Address
LiquidCrystal_I2C lcd(0x27, 16, 2);

// 🚀 Blynk Timer
BlynkTimer timer;

// 📡 Send Sensor Data to Blynk
void sendSensorData() {
  int gasDetected = digitalRead(MQ2_DIGITAL_PIN);
  int flameDetected = digitalRead(FLAME_SENSOR_PIN);

  // Send Data to Blynk
  Blynk.virtualWrite(V0, gasDetected == HIGH ? "DANGER" : "SAFE");
  Blynk.virtualWrite(V1, flameDetected != LOW ? "FIRE" : "NO FIRE");
  Blynk.virtualWrite(V2, (gasDetected == HIGH || flameDetected != LOW) ? 255 : 0);
}

// 🖥️ Serve HTML Web Page
void handleRoot() {
  server.send(200, "text/html", R"rawliteral(
    <!DOCTYPE html>
    <html lang="en">
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Gas & Flame Monitor</title>
      <style>
        body { font-family: Arial, sans-serif; text-align: center; padding: 20px; }
        .container { max-width: 400px; margin: auto; padding: 20px; border: 1px solid #ccc; border-radius: 10px; }
        .status { font-size: 20px; font-weight: bold; padding: 10px; border-radius: 5px; }
        .safe { background: #4CAF50; color: white; }
        .danger { background: #FF5733; color: white; }
      </style>
    </head>
    <body>
      <h2>🔥 Gas & Flame Monitor</h2>
      <div class="container">
        <p>Gas Status: <span id="gasStatus" class="status safe">SAFE</span></p>
        <p>Flame Status: <span id="flameStatus" class="status safe">NO FIRE</span></p>
      </div>

      <script>
        function updateStatus() {
          fetch("/status")
            .then(response => response.json())
            .then(data => {
              document.getElementById("gasStatus").innerText = data.gas ? "DANGER" : "SAFE";
              document.getElementById("gasStatus").className = "status " + (data.gas ? "danger" : "safe");

              document.getElementById("flameStatus").innerText = data.flame ? "FIRE DETECTED" : "NO FIRE";
              document.getElementById("flameStatus").className = "status " + (data.flame ? "danger" : "safe");
            });
        }
        setInterval(updateStatus, 1000);
      </script>
    </body>
    </html>
  )rawliteral");
}

// 📡 Serve JSON Sensor Data
void handleSensorStatus() {
  int gasDetected = digitalRead(MQ2_DIGITAL_PIN);
  int flameDetected = digitalRead(FLAME_SENSOR_PIN);
  String json = "{\"gas\": " + String(gasDetected) + ", \"flame\": " + String(flameDetected) + "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Gas & Flame Detector");

  // 🔌 Initialize LCD
  Wire.begin(SDA_PIN, SCL_PIN);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Gas & Flame Sensor");

  // 🌐 Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
  Serial.println(WiFi.localIP()); // Print ESP32 IP Address

  // 🎛️ Set Pin Modes
  pinMode(MQ2_DIGITAL_PIN, INPUT);
  pinMode(FLAME_SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  // 🚀 Start Web Server
  server.on("/", handleRoot);
  server.on("/status", handleSensorStatus);
  server.begin();

  // 🔗 Connect to Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  timer.setInterval(1000L, sendSensorData);

  delay(2000);
}

void loop() {
  Blynk.run();
  timer.run();
  server.handleClient();

  int gasDetected = digitalRead(MQ2_DIGITAL_PIN);
  int flameDetected = digitalRead(FLAME_SENSOR_PIN);

  Serial.print("Gas Status: ");
  Serial.println(gasDetected == HIGH ? "DANGER" : "SAFE");

  Serial.print("Flame Status: ");
  Serial.println(flameDetected != LOW ? "FIRE DETECTED" : "NO FIRE");

  // 📺 Update LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Status:");

  if (gasDetected == HIGH || flameDetected != LOW) { // 🚨 ALERT!
    Serial.println("WARNING: Gas/Flame Detected!");
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    lcd.setCursor(0, 1);
    lcd.print(gasDetected == HIGH && flameDetected != LOW ? "Gas & Fire ON!" :
              gasDetected == HIGH ? "Gas ON, Fire OFF" : "Fire ON, Gas OFF");
  } else { // ✅ SAFE CONDITION
    Serial.println("SAFE: No Gas or Fire");
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    lcd.setCursor(0, 1);
    lcd.print("Gas & Fire OFF");
  }

  delay(1000);
}
