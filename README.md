# IoT Zero-Trust Hardware Vault

## Project Objective
This project demonstrates a **Proof of Concept (PoC)** for a **multi-layered IoT security system** that implements a **Zero-Trust model** with **real-time MQTT logging**.

The system detects physical access attempts, enforces authentication, and reports events to a cloud broker.

---

# Core Logic & Working Principle

The system operates using a **Finite State Machine (FSM)** to track system activity in real time.

## State 0 – IDLE
- The outer door is **closed**
- The **LDR sensor detects darkness**
- The system remains in standby waiting for activity

## State 1 – TRIGGERED
- The **outer door is opened**
- **Light hits the LDR sensor**
- The **ESP32 starts a strict `millis()` timer (15 seconds)**
- LCD displays:


ENTER CODE


## State 2 – AUTHENTICATION
The user must:
1. Rotate the **potentiometer** to a specific secret value
2. Press the **switch** to confirm the code

## State 3A – SUCCESS
If the correct value is entered within **15 seconds**:

- LCD displays:


ACCESS GRANTED


- ESP32 sends an **MQTT message** logging a **successful entry**

## State 3B – TIMEOUT / BREACH
If either of the following occurs:

- 15 seconds pass without authentication
- Wrong code is entered

The system triggers an alarm:

- **Buzzer sounds**
- **Red LED flashes**
- **MQTT alert is published**

Example alert message:


ACCESS TIMEOUT / INTRUSION


---

# Implementation Guide (Scratch to End)

## Circuitry & Hardware Wiring

Ensure the breadboard wiring is clean and secure to avoid short circuits.

### Input 1 – Outer Door Detection
- Use an **LDR sensor**
- Connect it to an **analog pin**
- Use a **voltage divider with a resistor**

Purpose:
- Detect light when the door opens

---

### Input 2 – Authentication Dial
- Connect the **middle pin of the potentiometer** to another **analog pin**

Purpose:
- Acts as a **secret dial-based authentication code**

---

### Input 3 – Enter Button
- Connect a **switch** to a **digital pin**
- Use the **ESP32 internal pull-up resistor**

```cpp
pinMode(buttonPin, INPUT_PULLUP);
```

Purpose:

Confirms the entered authentication value

Output Devices

Connect the following outputs to digital pins:

Buzzer – Intrusion alert

Red LED – Visual warning indicator

LCD Display – User interface

The LCD can be connected using:

Standard parallel pins
or

I2C interface (recommended)

Coding the Logic
Important Rule

Do not use delay() for timeout logic.

Why?

delay() blocks the microcontroller and prevents it from reading inputs.

Instead, use millis() for non-blocking timing.

Step 1 – Sensor Testing

Read the LDR value

Print values to the Serial Monitor

Determine the threshold between:

Condition	Sensor Value
Door Closed	Low light
Door Open	High light

Example code concept:

int LDR_Value = analogRead(LDR_Pin);
Serial.println(LDR_Value);

Commit your progress:

git commit -m "Added LDR threshold and basic inputs"
Step 2 – Timer Logic

Use millis() to create a 15-second authentication window.

if (LDR_Value > threshold && !timerStarted) {
    startTime = millis();
    timerStarted = true;
}

if (timerStarted && (millis() - startTime >= 15000)) {
    triggerAlarm(); // 15 seconds passed
}

This allows the system to:

Continue reading sensors

Avoid blocking execution

Step 3 – Wi-Fi & MQTT Integration

Include the required libraries:

#include <WiFi.h>
#include <PubSubClient.h>

Configure the ESP32 to connect to:

WiFi network

HiveMQ public MQTT broker

Purpose:

Send alerts and logs to the cloud.

Example MQTT messages:

ACCESS GRANTED
ACCESS TIMEOUT
INTRUSION DETECTED

Commit the changes:

git commit -m "Implemented millis timer and MQTT connection"
Testing & Demonstration
MQTT Monitoring

Open the HiveMQ WebSocket Client on your laptop

Subscribe to the project topic

Example topic:

vault/security/logs
Testing Procedure

Trigger the LDR sensor using your hand or light

Wait 15 seconds without authentication

Observe the system behavior:

Expected results:

Buzzer activates

LED flashes

MQTT alert appears on the dashboard

Technologies Used

ESP32 Microcontroller

MQTT Protocol

HiveMQ Public Broker

LDR Sensor

Potentiometer

Push Button

LCD Display

Buzzer & LED

Arduino IDE

Security Concept

This system demonstrates multi-layer security:

Physical Intrusion Detection – LDR sensor

Authentication Mechanism – Potentiometer code

Time-Limited Access – 15-second timer

Alert System – Buzzer + LED

Remote Logging – MQTT cloud alerts

This approach follows the Zero-Trust Security Model.

Future Improvements

Possible upgrades include:

Encrypted MQTT (TLS)

Keypad or biometric authentication

Mobile notification system

Secure password storage

Tamper detection sensors

License

This project is released for educational and research purposes.


---

✅ This version is **GitHub-ready** with:

- Proper markdown structure  
- Clean headings  
- Code blocks  
- Lists  
- Readable documentation  

---

If you want, I can also **upgrade this README to a professional GitHub README with:**

- Architecture diagram  
- Circuit diagram  
- State machine diagram  
- Images section  
- Demo GIF  
- Folder structure  

That version looks **much more impressive for lecturers and GitHub portfolios**.
