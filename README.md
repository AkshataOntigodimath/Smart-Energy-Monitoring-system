⚡ Smart Energy Monitoring System

An IoT-based smart energy monitoring and management system using ESP32 to monitor electrical parameters, detect abnormal energy usage, and control electrical loads remotely.

📌 Overview

The system measures voltage and current from an AC supply using sensors connected to an ESP32 microcontroller. The measured data is displayed locally on an I2C LCD and transmitted through Wi-Fi to the ThingSpeak cloud platform for remote monitoring.

The system can also provide alerts and automatically disconnect the load through a relay when abnormal conditions are detected.

🏗️ System Architecture

"System Architecture" (Circuit-Diagram/system_architecture.png)

🔲 Block Diagram

"Block Diagram" (Circuit-Diagram/block_diagram.png)

✨ Features

- ⚡ Real-time voltage and current monitoring
- 📊 Power and energy consumption monitoring
- 📺 LCD-based local display
- 🕵️ Abnormal usage and theft detection
- 🔌 Automatic load control using relay
- 🚨 Buzzer-based alerts
- 📡 Wi-Fi connectivity
- ☁️ Cloud monitoring using ThingSpeak
- 📱 Remote monitoring through a mobile interface

🔧 Hardware Components

Component| Purpose
🔹 ESP32| Main controller and Wi-Fi communication
⚡ ZMPT101B| AC voltage measurement
🔌 ACS712| Current measurement
🔄 Relay Module| Load switching and automatic power cut-off
📺 I2C LCD| Display of measured parameters
🚨 Buzzer| Alert indication
💡 AC Load| Electrical load being monitored

💻 Software & Technologies

- Arduino IDE
- Embedded C/C++
- ESP32
- I2C Communication
- Wi-Fi
- ThingSpeak
- Sensor Interfacing

⚙️ Working

1. 🔌 The AC supply is connected to the monitored electrical load.
2. ⚡ The ZMPT101B sensor measures the supply voltage.
3. 📊 The ACS712 sensor measures the current consumed by the load.
4. 🧠 The ESP32 receives and processes the sensor readings.
5. 📺 The measured parameters are displayed on the I2C LCD.
6. 🕵️ The ESP32 detects abnormal usage conditions based on the programmed logic.
7. 🚨 A buzzer provides an alert when an abnormal condition is detected.
8. 🔄 The relay can disconnect the load automatically when required.
9. ☁️ Monitoring data is transmitted to ThingSpeak through Wi-Fi.
10. 📱 The data can be monitored remotely using a mobile interface.

🎯 Applications

- 🏠 Smart homes
- 🏭 Industrial energy monitoring
- 🏢 Commercial buildings
- ⚡ Energy management systems
- 📡 Remote load monitoring
- 🛡️ Electrical safety and protection

🔮 Future Enhancements

- 📱 Dedicated mobile application
- 🤖 AI/ML-based energy consumption prediction
- 💰 Electricity bill estimation
- 🔔 Push notifications
- 🔌 Multi-load monitoring
- 📈 Advanced energy analytics
- 🌐 Web-based monitoring dashboard

⚠️ Safety

«Warning: AC mains voltage can be dangerous. Proper electrical isolation, protection, insulation, and supervision should be used when working with mains-connected circuits.»

👩‍💻 Author

Akshata Ontigodimath

ECE Undergraduate | VLSI | Embedded Systems | IoT | Python
