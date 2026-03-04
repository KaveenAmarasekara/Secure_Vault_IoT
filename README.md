# Secure_Vault_IoT
For the Electronics and Physical Computing Module
##

Project Plan: IoT Zero-Trust Hardware Vault

Objective: Proof of Concept for a multi-layered security system with MQTT logging.

Core Logic & Working Principle
The system operates on a State Machine to track what is happening in real-time.
State 0 (IDLE): The outer door is closed. The LDR reads darkness. The system is quietly waiting.
State 1 (TRIGGERED): The outer door is opened. Light hits the LDR.
The ESP32 starts a strict millis() timer (e.g., 15 seconds).
The LCD Display prompts: "ENTER CODE".
State 2 (AUTHENTICATION): The user must turn the Potentiometer to a specific secret value and press the Switch to confirm.
State 3A (SUCCESS): If the switch is pressed at the right value before 15 seconds, the LCD reads "ACCESS GRANTED", and an MQTT message logs a successful entry.
State 3B (TIMEOUT / BREACH): If 15 seconds pass with no input, OR the wrong code is entered, the Buzzer sounds, a Red LED flashes, and an MQTT "ACCESS TIMEOUT / INTRUSION" alert is published to the cloud.
Implementation Guide: Scratch to End
Circuitry & Hardware Wiring
Keep the breadboard wiring clean to avoid short circuits.
Input 1 (Outer Door): Connect the LDR to an analog pin using a voltage divider with a Resistor.
Input 2 (Authentication Dial): Connect the Potentiometer's middle pin to another analog pin.
Input 3 (Enter Button): Connect a Switch to a digital pin. Use the ESP32's internal pull-up resistor in your code (INPUT_PULLUP).
Outputs: Connect the Buzzer and an LED to digital pins. Connect the LCD (using standard pins or I2C if available).
Coding the Logic in IDE 
Do not use delay() for your timeout feature, as it pauses the whole microcontroller and stops it from reading the potentiometer or switch. Use millis().
Step 1: Sensor Test. Read the LDR and print it to the Serial Monitor to find the threshold value between "door closed" (dark) and "door open" (light).
Commit 2: git commit -m "Added LDR threshold and basic inputs"
Step 2: The Timer Logic. ```cpp if (LDR_Value > threshold && !timerStarted) { startTime = millis(); timerStarted = true; } if (timerStarted && (millis() - startTime >= 15000)) { triggerAlarm(); // 15 seconds passed! }
Step 3: Add Wi-Fi & MQTT. Include the WiFi.h and PubSubClient.h libraries. Set up the connection to the public HiveMQ broker.
Commit 3: git commit -m "Implemented millis timer and MQTT connection"
Testing & Presentation Prep
Testing: Open the HiveMQ public web socket client on your laptop. Trigger the LDR with your hand, wait 15 seconds, and watch the dashboard light up with your timeout alert.


