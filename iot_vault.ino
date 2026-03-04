#include <WiFi.h>
#include <PubSubClient.h>
#include <WebServer.h>
#include <LiquidCrystal.h>

// ==========================================
// 1. CONFIGURATION (CHANGE THESE FOR THE LAB)
// ==========================================
const char* ssid = "Visitha";         // e.g., your mobile hotspot
const char* password = "12345678"; // Hotspot password
const char* mqtt_server = "broker.hivemq.com"; // Public HiveMQ broker

// ==========================================
// 2. PIN DEFINITIONS (Matching your wiring perfectly)
// ==========================================
// LCD: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(22, 23, 5, 18, 19, 21);

// Inputs
const int LDR_PIN = 34;
const int POT_PIN = 35;
const int BUTTON_PIN = 32;
const int TRIG_PIN = 33;
const int ECHO_PIN = 27;

// Outputs
const int LED_PIN = 25;
const int BUZZER_PIN = 26;

// ==========================================
// 3. SYSTEM VARIABLES & THRESHOLDS
// ==========================================
int LDR_THRESHOLD = 2000;      // Tweak this based on the Serial Monitor debug output
int DISTANCE_THRESHOLD = 15;   // Trigger if closer than 15 cm
int SECRET_CODE = 2000;        // Target potentiometer value
int TOLERANCE = 500;           // Generous allowance to make the 5-min demo smooth

int currentState = 0; // 0: IDLE, 1: TRIGGERED, 2: SUCCESS, 3: BREACH
unsigned long timerStart1 = 0;
const unsigned long TIMEOUT_MS = 10000;
unsigned long lastDebugTime = 0;

// ==========================================
// 4. NETWORK OBJECTS
// ==========================================
WebServer server(80);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// ==========================================
// 5. HELPER FUNCTIONS
// ==========================================
int getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); 
  if (duration == 0) return 999; 
  return duration * 0.034 / 2;
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>Vault Dashboard</title>";
  html += "<meta http-equiv='refresh' content='2'>"; // Auto-refresh every 2 seconds
  
  // 1. Properly close the CSS styling
  html += "<style>body{font-family: Arial; text-align: center; margin-top: 50px; background-color: #f4f4f4;}";
  html += ".card{background: white; padding: 20px; border-radius: 10px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); display: inline-block;}";
  html += "h1{color: #333;} .status{font-size: 24px; font-weight: bold;}</style></head><body>";
  
  // 2. Start the visible Body and Card
  html += "<h1>IoT Zero-Trust Vault</h1>";
  html += "<div class='card'><h2>Current Status:</h2><br><span class='status' style='color: ";
  
  // 3. Dynamic text and colors based on State
  if (currentState == 0) {
    html += "green;'>SYSTEM ARMED (IDLE)";
  } else if (currentState == 1) {
    html += "orange;'>WARNING: Auth Required! (10s Timer)";
  } else if (currentState == 2) {
    html += "blue;'>ACCESS GRANTED";
  } else if (currentState == 3) {
    html += "red;'>BREACH / INTRUSION DETECTED!";
  } else if (currentState == 4) {
    // Added this for your Single-Run mode!
    html += "black;'>SYSTEM HALTED (Please press RST to restart demo)";
  }
  
  // 4. Close the HTML tags
  html += "</span></div></body></html>";
  
  server.send(200, "text/html", html);
}

void reconnectMQTT() {
  if (!mqttClient.connected()) {
    String clientId = "ESP32VaultClient-";
    clientId += String(random(0xffff), HEX);
    if (mqttClient.connect(clientId.c_str())) {
      mqttClient.publish("vault/status", "Vault System Online");
    }
  }
}

void resetSystem() {
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  currentState = 0;
  lcd.clear();
  lcd.print("SYSTEM ARMED");
  mqttClient.publish("vault/status", "System Reset & Armed");
}

// ==========================================
// 6. MAIN SETUP
// ==========================================
void setup() {
  Serial.begin(115200);
  
  // Pin Modes
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Init LCD
  lcd.begin(16, 2);
  lcd.print("Booting...");

  // Connect WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("Web UI IP Address: ");
  Serial.println(WiFi.localIP());

  // Setup Server & MQTT
  server.on("/", handleRoot);
  server.begin();
  mqttClient.setServer(mqtt_server, 1883);

  lcd.clear();
  lcd.print("SYSTEM ARMED");
}

// ==========================================
// 7. MAIN LOOP
// ==========================================
void loop() {
  server.handleClient();
  if (!mqttClient.connected()) reconnectMQTT();
  mqttClient.loop();

  // Read all sensors
  int ldrValue = analogRead(LDR_PIN);
  int potValue = analogRead(POT_PIN);
  int distance = getDistance();
  bool buttonPressed = (digitalRead(BUTTON_PIN) == LOW);

  // --- DIAGNOSTIC DEBUGGER (Prints to Serial Monitor every 1 second) ---
  if (millis() - lastDebugTime > 1000) {
    Serial.println("--- SENSOR DIAGNOSTICS ---");
    Serial.printf("LDR: %d (Threshold: %d)\n", ldrValue, LDR_THRESHOLD);
    Serial.printf("Dist: %d cm (Threshold: %d cm)\n", distance, DISTANCE_THRESHOLD);
    Serial.printf("Potentiometer: %d (Target: %d)\n", potValue, SECRET_CODE);
    Serial.printf("Button State: %s\n", buttonPressed ? "PRESSED" : "Open");
    Serial.printf("Current State: %d\n\n", currentState);
    lastDebugTime = millis();
  }

  // --- CORE STATE MACHINE ---
  // --- CORE STATE MACHINE ---
  switch (currentState) {
    case 0: // IDLE / ARMED
      if (ldrValue > LDR_THRESHOLD || distance < DISTANCE_THRESHOLD) {
        currentState = 1;
        timerStart1 = millis();
        lcd.clear();
        
        if (ldrValue > LDR_THRESHOLD) {
          lcd.print("DOOR OPENED!");
          mqttClient.publish("vault/status", "ALERT: Door Open");
        } else {
          lcd.print("MOTION DETECTED!");
          mqttClient.publish("vault/status", "ALERT: Motion");
        }
        lcd.setCursor(0, 1);
        lcd.print("ENTER CODE...");
      }
      break;

    case 1: // TRIGGERED (Waiting 10 seconds for code)
      // 1. Timeout Check
      if (millis() - timerStart1 >= TIMEOUT_MS) {
        currentState = 3; // Move to BREACH state
        break;
      }

      // 2. Check Authentication Button
      if (buttonPressed) {
        if (abs(potValue - SECRET_CODE) <= TOLERANCE) {
          currentState = 2; // Correct Code -> SUCCESS
        } else {
          currentState = 3; // Wrong Code -> BREACH
        }
      }
      break;

    case 2: // SUCCESS (End of run)
      lcd.clear();
      lcd.print("ACCESS GRANTED");
      mqttClient.publish("vault/status", "SUCCESS: Access Granted");
      
      currentState = 4; // Move to HALTED state
      break;

    case 3: // BREACH DETECTED (End of run)
      lcd.clear();
      lcd.print("INTRUSION ALARM");
      digitalWrite(LED_PIN, HIGH);
      digitalWrite(BUZZER_PIN, HIGH);
      mqttClient.publish("vault/status", "CRITICAL: INTRUSION DETECTED!");
      
      currentState = 4; // Move to HALTED state
      break;

    case 4: // HALTED (Single run finished )
      // The system does absolutely nothing here. 
      // The LED/Buzzer stay ON (if breached), or the LCD stays on "ACCESS GRANTED".
      // Press the physical 'RST' button on the ESP32 to run the demo again.
      break;
  }
}