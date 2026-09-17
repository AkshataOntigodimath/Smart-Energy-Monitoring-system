# ⚡ Smart Energy Monitoring System

An **IoT-based Smart Energy Monitoring System** using **ESP32** to monitor electrical parameters, detect abnormal energy usage, and control electrical loads remotely.

---

## 📌 Overview

The system monitors the electrical parameters of an AC supply using **ZMPT101B** and **ACS712** sensors connected to an **ESP32 microcontroller**.

The measured data is displayed on an **I2C LCD** and transmitted through **Wi-Fi** to the **ThingSpeak cloud platform** for remote monitoring. The system can also generate alerts and automatically disconnect the load using a relay when abnormal conditions are detected.

---

## 🏗️ System Architecture

<p align="center">
  <img src="Circuit-Diagram/system_architecture.png" width="800">
</p>

---

## 🔲 Block Diagram

<p align="center">
  <img src="Circuit-Diagram/block_diagram.png" width="800">
</p>

---

## ✨ Features

- ⚡ **Real-time Voltage Monitoring**
- 🔌 **Current Monitoring**
- 📊 **Power and Energy Consumption Monitoring**
- 📺 **I2C LCD Display**
- 🕵️ **Abnormal Usage / Theft Detection**
- 🔄 **Automatic Load Control**
- 🚨 **Real-time Alerts**
- 📡 **Wi-Fi Connectivity**
- ☁️ **ThingSpeak Cloud Monitoring**
- 📱 **Remote Monitoring**

---

## 🔧 Hardware Components

| Component | Purpose |
|------------|---------|
| 🔹 **ESP32** | Main microcontroller and Wi-Fi communication |
| ⚡ **ZMPT101B** | AC voltage measurement |
| 🔌 **ACS712** | Current measurement |
| 🔄 **Relay Module** | Load switching and automatic power cut-off |
| 📺 **I2C LCD** | Display of measured parameters |
| 🚨 **Buzzer** | Alert indication |
| 💡 **AC Load** | Electrical load being monitored |

---

## 💻 Software & Technologies

- **Arduino IDE**
- **Embedded C/C++**
- **ESP32**
- **I2C Communication**
- **Wi-Fi**
- **ThingSpeak**
- **Sensor Interfacing**

---

## ⚙️ Working Principle

1. 🔌 The **AC supply** is connected to the monitored electrical load.
2. ⚡ The **ZMPT101B sensor** measures the supply voltage.
3. 📊 The **ACS712 sensor** measures the current consumed by the load.
4. 🧠 The **ESP32** receives and processes the sensor readings.
5. 📺 The measured parameters are displayed on the **I2C LCD**.
6. 🕵️ The ESP32 checks for **abnormal usage conditions**.
7. 🚨 The **buzzer** provides an alert when an abnormal condition is detected.
8. 🔄 The **relay** can automatically disconnect the load when required.
9. ☁️ The ESP32 sends monitoring data to **ThingSpeak through Wi-Fi**.
10. 📱 The data can be monitored remotely through a **mobile interface**.

---

## 🎯 Applications

- 🏠 **Smart Homes**
- 🏭 **Industrial Energy Monitoring**
- 🏢 **Commercial Buildings**
- ⚡ **Energy Management Systems**
- 📡 **Remote Load Monitoring**
- 🛡️ **Electrical Safety and Protection**

---

## 🔮 Future Enhancements

- 📱 Dedicated mobile application
- 🤖 AI/ML-based energy consumption prediction
- 💰 Electricity bill estimation
- 🔔 Push notifications
- 🔌 Multi-load monitoring
- 📈 Advanced energy analytics
- 🌐 Web-based monitoring dashboard

---

## ⚠️ Safety

> **Warning:** AC mains voltage can be dangerous. Proper electrical isolation, protection, insulation, and supervision should be used when working with mains-connected circuits.

---

## 👩‍💻 Author

### **Akshata Ontigodimath**

**ECE Undergraduate**  
VLSI • Embedded Systems • IoT • Python
