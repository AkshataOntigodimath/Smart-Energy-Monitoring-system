# Smart-Energy-Monitoring-system
⚡ Smart Energy Monitoring System

An IoT-based Smart Energy Monitoring System designed using ESP32 to monitor electrical parameters, detect abnormal usage, provide real-time alerts, and remotely control loads. The system integrates sensors, relay-based control, LCD display, Wi-Fi connectivity, and cloud monitoring for efficient energy management.

🚀 Features

⚡ Real-time Voltage Monitoring using ZMPT101B

🔌 Current Monitoring using ACS712

📊 Energy Usage Monitoring

📺 Real-time LCD Display through I2C

🔄 Automatic Load Control using Relay Module

🚨 Real-time Alerts using Buzzer

🕵️ Abnormal Usage / Theft Detection

☁️ Live Cloud Data Monitoring using ThingSpeak

📱 Remote Monitoring through Mobile App

🔴 Automatic Power Cut-off during detected abnormal conditions


🧩 System Architecture

AC MAINS SUPPLY
                    │
          ┌─────────┴─────────┐
          │                   │
   Voltage Sensor       Current Sensor
    (ZMPT101B)             (ACS712)
          │                   │
          └─────────┬─────────┘
                    ↓
              ┌───────────┐
              │   ESP32   │
              │Controller │
              └─────┬─────┘
          ┌──────────┼───────────┐
          ↓          ↓           ↓
       LCD        Buzzer       Relay
       I2C                       │
                                ↓
                               Load
                                
              ESP32 Wi-Fi
                   │
                   ↓
             ThingSpeak
                   │
                   ↓
              Mobile App

🔧 Hardware Components

Component	Purpose

ESP32	Main microcontroller and Wi-Fi communication
ZMPT101B	AC voltage measurement
ACS712	AC current measurement
Relay Module	Load switching and automatic cut-off
I2C LCD	Local parameter display
Buzzer	Alert indication
AC Load	Device being monitored/controlled
Power Supply	Provides required low-voltage power


💻 Software & Technologies

Arduino IDE

Embedded C/C++

ESP32

I2C Communication

Wi-Fi

ThingSpeak IoT Cloud

Sensor interfacing

Relay-based automation


⚙️ Working Principle

1. The AC mains supply powers the monitored electrical load.


2. The ZMPT101B voltage sensor measures the AC voltage.


3. The ACS712 current sensor measures the current consumed by the load.


4. Sensor readings are sent to the ESP32.


5. The ESP32 processes the measured electrical parameters.


6. Important readings are displayed locally on the I2C LCD.


7. If abnormal consumption or a possible theft condition is detected, the system activates the buzzer.


8. The ESP32 can operate the relay module to disconnect the load automatically.


9. Through built-in Wi-Fi, monitoring data is transmitted to ThingSpeak.


10. The data can be accessed remotely through a mobile application/dashboard.



📁 Project Structure

Smart-Energy-Monitoring-System/
│
├── Arduino/
│   └── Smart_Energy_Monitoring.ino
│
├── Circuit-Diagram/
│   ├── system_architecture.png
│   └── block_diagram.png
│
├── Documentation/
│   └── Project_Report.pdf
│
├── README.md
└── LICENSE

🌐 IoT Monitoring

The ESP32 uses Wi-Fi to send monitoring data to ThingSpeak, enabling remote visualization of electrical parameters and historical data.

Possible parameters include:

Voltage

Current

Power

Energy consumption

Abnormal usage status

Load status


🔮 Future Enhancements

📱 Dedicated Android application

🤖 AI/ML-based electricity consumption prediction

📈 Advanced energy analytics

💰 Electricity bill estimation

🔔 SMS/Push notifications

🏠 Multi-load monitoring

📊 Web-based monitoring dashboard

🔐 Improved theft-detection algorithms


🎯 Applications

Smart homes

Industrial energy monitoring

Commercial buildings

Energy management systems

Electrical safety monitoring

Remote load management


👩‍💻 Author

Akshata Ontigodimath

ECE Undergraduate | VLSI • Embedded Systems • IoT • Python


---

> ⚠️ Safety Note: AC mains voltage is hazardous. Use appropriate isolation, fuses, enclosures, and qualified supervision when working with mains-connected circuits. The ESP32 and low-voltage electronics should be properly isolated from hazardous mains voltages.



Available next action: Create a downloadable DOCX file here in this chat containing the editable prose above
