#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

// Software Serial pins to communicate with Arduino
#define ARDUINO_RX D1 // Connect to Arduino TX
#define ARDUINO_TX D2 // Connect to Arduino RX

// Network credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "YOUR_TELEGRAM_BOT_TOKEN"

// Chat ID to send messages to (use @myidbot to find out your chat ID)
#define CHAT_ID "YOUR_CHAT_ID"

// Initialize Telegram bot
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

// Initialize software serial
SoftwareSerial arduinoSerial(ARDUINO_RX, ARDUINO_TX); // RX, TX

// Variables to track alarm states
int currentAlarmState = -1;
unsigned long lastMsgTime = 0;
const unsigned long MSG_INTERVAL = 60000; // Minimum time between repeated alerts (60 seconds)

void setup() {
  // Initialize serial communications
  Serial.begin(9600);
  arduinoSerial.begin(9600);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  
  // Configure secured client
  configTime(0, 0, "pool.ntp.org");      // Get UTC time via NTP
  secured_client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  
  // Send a startup message
  String welcomeMsg = "🚨 *Safety Monitoring System Online* 🚨\n\n";
  welcomeMsg += "The gas and flame detection system has been activated. You will receive alerts when dangerous conditions are detected.";
  bot.sendMessage(CHAT_ID, welcomeMsg, "Markdown");
}

void loop() {
  // Check if there's data from Arduino
  if (arduinoSerial.available() > 0) {
    String data = arduinoSerial.readStringUntil('\n');
    
    Serial.print("Received from Arduino: ");
    Serial.println(data);
    
    // Process the data if it's in the expected format (A:X)
    if (data.startsWith("A:") && data.length() >= 3) {
      int alarmState = data.substring(2, 3).toInt();
      
      // Only send alert if state has changed or enough time has passed since last alert
      unsigned long currentTime = millis();
      if (alarmState != currentAlarmState || 
          (alarmState != 0 && currentTime - lastMsgTime >= MSG_INTERVAL)) {
        
        sendTelegramAlert(alarmState);
        currentAlarmState = alarmState;
        lastMsgTime = currentTime;
      }
    }
  }
  
  // Small delay
  delay(100);
}

// Send appropriate alert message to Telegram based on alarm state
void sendTelegramAlert(int alarmState) {
  String message = "";
  
  switch (alarmState) {
    case 0:
      message = "✅ *ALL CLEAR* ✅\n\nAll sensors have returned to normal levels.";
      break;
    case 1:
      message = "⚠️ *GAS DETECTED!* ⚠️\n\nDangerous gas levels have been detected. Please check the area immediately!";
      break;
    case 2:
      message = "🔥 *FIRE ALERT!* 🔥\n\nFlame detected by sensors. Please check the area immediately!";
      break;
    case 3:
      message = "🚨 *EMERGENCY: FIRE AND GAS DETECTED!* 🚨\n\nBoth flame and gas detected. This is extremely dangerous! Evacuate immediately!";
      break;
    default:
      message = "⚠️ *SYSTEM ERROR* ⚠️\n\nUnknown alarm state received. Please check the monitoring system.";
  }
  
  if (bot.sendMessage(CHAT_ID, message, "Markdown")) {
    Serial.println("Telegram alert sent successfully");
  } else {
    Serial.println("Failed to send Telegram alert");
  }
}
