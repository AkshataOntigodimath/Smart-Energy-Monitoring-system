/*
 * ╔══════════════════════════════════════════════════════════════════╗
 * ║         SMART GRID MANAGEMENT SYSTEM — ESP32 DEVKIT            ║
 * ║                    FINAL PRODUCTION CODE                       ║
 * ╠══════════════════════════════════════════════════════════════════╣
 * ║  FEATURES:                                                      ║
 * ║  • Real-time Voltage, Current, Power, Frequency monitoring      ║
 * ║  • Overload detection + auto relay trip                         ║
 * ║  • Short circuit detection                                      ║
 * ║  • Overvoltage & undervoltage detection                         ║
 * ║  • Electricity theft / bypass detection (reed switch)           ║
 * ║  • LCD 16x2 I2C live display with scrolling pages               ║
 * ║  • ThingSpeak cloud logging every 15 seconds                    ║
 * ║  • Email alert via ThingSpeak React when fault occurs           ║
 * ║  • Audible buzzer with different patterns per fault             ║
 * ║  • Red / Green LED status indicators                            ║
 * ║  • Auto relay reset after fault clears (configurable)           ║
 * ║  • Serial Monitor full debug output                             ║
 * ║  • WiFi auto-reconnect                                          ║
 * ╠══════════════════════════════════════════════════════════════════╣
 * ║  PIN MAP — ESP32 DevKit v1                                      ║
 * ║  ┌─────────────────────────────────────────────────────────┐   ║
 * ║  │ ZMPT101B OUT  → 10kΩ+20kΩ divider → GPIO 34 (ADC1_6)   │   ║
 * ║  │ ACS712 VOUT   → 10kΩ+20kΩ divider → GPIO 35 (ADC1_7)   │   ║
 * ║  │ Reed switch   → 1kΩ+1kΩ divider   → GPIO 33 (INPUT_PU) │   ║
 * ║  │ BC547 Base    → 1kΩ resistor      → GPIO 14             │   ║
 * ║  │ Buzzer (+)    → direct             → GPIO 26             │   ║
 * ║  │ Red LED       → 220Ω              → GPIO 27             │   ║
 * ║  │ Green LED     → 220Ω              → GPIO 25             │   ║
 * ║  │ LCD SDA       → direct             → GPIO 21             │   ║
 * ║  │ LCD SCL       → direct             → GPIO 22             │   ║
 * ║  │ All VCC       → 5V (ESP32 VIN)                           │   ║
 * ║  │ All GND       → Common GND rail                          │   ║
 * ║  └─────────────────────────────────────────────────────────┘   ║
 * ╠══════════════════════════════════════════════════════════════════╣
 * ║  THINGSPEAK CHANNEL FIELD MAP                                   ║
 * ║  Field 1 → Voltage (V RMS)                                      ║
 * ║  Field 2 → Current (A RMS)                                      ║
 * ║  Field 3 → Active Power (W)                                     ║
 * ║  Field 4 → Power Factor                                         ║
 * ║  Field 5 → Frequency (Hz)                                       ║
 * ║  Field 6 → Alert Code                                           ║
 * ║             0 = Normal                                          ║
 * ║             1 = Overload                                        ║
 * ║             2 = Short Circuit                                   ║
 * ║             3 = Overvoltage                                     ║
 * ║             4 = Undervoltage                                    ║
 * ║             5 = Theft / Bypass Detected                         ║
 * ╠══════════════════════════════════════════════════════════════════╣
 * ║  REQUIRED LIBRARIES (Arduino Library Manager)                   ║
 * ║  • LiquidCrystal_I2C  by Frank de Brabander                     ║
 * ║  • ThingSpeak          by MathWorks                             ║
 * ║  • WiFi.h              built-in ESP32                           ║
 * ║  • Wire.h              built-in ESP32                           ║
 * ╚══════════════════════════════════════════════════════════════════╝
 */

// ════════════════════════════════════════════════════════════
//  INCLUDES
// ════════════════════════════════════════════════════════════
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <ThingSpeak.h>
#include <math.h>

// ════════════════════════════════════════════════════════════
//  USER CONFIGURATION — FILL THESE IN
// ════════════════════════════════════════════════════════════
const char* WIFI_SSID         = "YOUR_WIFI_NAME";       // Your WiFi name
const char* WIFI_PASSWORD     = "YOUR_WIFI_PASSWORD";   // Your WiFi password
unsigned long TS_CHANNEL_ID   = 0000000;                // ThingSpeak Channel ID
const char*   TS_WRITE_API    = "XXXXXXXXXXXXXXXX";     // ThingSpeak Write API Key

// ════════════════════════════════════════════════════════════
//  PIN DEFINITIONS
// ════════════════════════════════════════════════════════════
#define PIN_VOLTAGE      34    // ZMPT101B output (via voltage divider)
#define PIN_CURRENT      35    // ACS712 output  (via voltage divider)
#define PIN_BYPASS       33    // Reed switch bypass detect (INPUT_PULLUP)
#define PIN_RELAY        14    // Relay control via BC547 transistor
#define PIN_BUZZER       26    // Active buzzer
#define PIN_LED_RED      27    // Red  LED (fault indicator)
#define PIN_LED_GREEN    25    // Green LED (normal indicator)
#define SDA_PIN          21    // I2C SDA for LCD
#define SCL_PIN          22    // I2C SCL for LCD

// ════════════════════════════════════════════════════════════
//  LCD CONFIGURATION
// ════════════════════════════════════════════════════════════
// If your LCD shows nothing, change 0x27 to 0x3F below
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ════════════════════════════════════════════════════════════
//  SENSOR CALIBRATION
// ════════════════════════════════════════════════════════════
// ── Voltage sensor (ZMPT101B) ──────────────────────────────
// Increase VOLTAGE_CAL if reading is too low, decrease if too high
// Compare serial output against a multimeter and tune this value
float VOLTAGE_CAL    = 0.5f;    // Calibration multiplier — TUNE THIS
float VOLTAGE_OFFSET = 1.65f;   // ADC mid-rail (3.3V / 2)

// ── Current sensor (ACS712-20A) ────────────────────────────
// ACS712-20A: 100mV per Amp sensitivity
// If idle current shows non-zero, adjust ACS_ZERO slightly
float ACS_SENSITIVITY = 0.100f; // V/A for 20A module
float ACS_ZERO        = 1.65f;  // Voltage at 0A — tune if idle != 0A

// ── ADC reference ──────────────────────────────────────────
const float ADC_REF   = 3.3f;
const int   ADC_MAX   = 4095;
const int   RMS_SAMPLES = 100;  // Samples per RMS calculation (100 = 1 full 50Hz cycle)

// ════════════════════════════════════════════════════════════
//  GRID PROTECTION THRESHOLDS
// ════════════════════════════════════════════════════════════
// India standard: 230V AC, 50Hz
// Change these to suit your actual installation
const float NOMINAL_VOLTAGE     = 230.0f;
const float OVERVOLTAGE_LIMIT   = 253.0f;  // 230V + 10%
const float UNDERVOLTAGE_LIMIT  = 207.0f;  // 230V - 10%
const float OVERLOAD_CURRENT    = 15.0f;   // Amps — trip threshold
const float SHORT_CIRCUIT_CURRENT = 20.0f; // Amps — short circuit threshold
const float MAX_POWER           = 3450.0f; // Watts (230V × 15A)

// Auto-reset relay after fault clears (true = yes, false = manual reset needed)
const bool AUTO_RESET_ON_CLEAR  = true;
// How long to wait before auto-reset after fault clears (ms)
const unsigned long RESET_DELAY_MS = 5000;

// ════════════════════════════════════════════════════════════
//  TIMING CONFIGURATION
// ════════════════════════════════════════════════════════════
const unsigned long SENSOR_INTERVAL     = 100;    // ms — sensor polling
const unsigned long THINGSPEAK_INTERVAL = 15000;  // ms — cloud upload (min 15s)
const unsigned long LCD_PAGE_INTERVAL   = 3000;   // ms — LCD page flip
const unsigned long SERIAL_INTERVAL     = 1000;   // ms — serial print interval
const unsigned long WIFI_RETRY_INTERVAL = 30000;  // ms — WiFi retry if lost

// ════════════════════════════════════════════════════════════
//  ALERT CODES
// ════════════════════════════════════════════════════════════
enum AlertCode : uint8_t {
  ALERT_NORMAL        = 0,
  ALERT_OVERLOAD      = 1,
  ALERT_SHORT_CIRCUIT = 2,
  ALERT_OVERVOLTAGE   = 3,
  ALERT_UNDERVOLTAGE  = 4,
  ALERT_THEFT         = 5
};

// ════════════════════════════════════════════════════════════
//  SYSTEM STATE STRUCTURE
// ════════════════════════════════════════════════════════════
struct GridState {
  float     voltage;        // V RMS
  float     current;        // A RMS
  float     power;          // Watts
  float     powerFactor;    // 0.0 – 1.0
  float     frequency;      // Hz
  float     energy_wh;      // Wh accumulated since boot
  AlertCode alertCode;      // Current alert
  bool      relayOn;        // Relay state
  bool      bypassActive;   // Bypass switch state
  unsigned long faultStart; // Millis when fault began
  unsigned long faultClearTime; // Millis when fault cleared
};

GridState grid = {
  .voltage       = 0,
  .current       = 0,
  .power         = 0,
  .powerFactor   = 0.92f,
  .frequency     = 50.0f,
  .energy_wh     = 0,
  .alertCode     = ALERT_NORMAL,
  .relayOn       = true,
  .bypassActive  = false,
  .faultStart    = 0,
  .faultClearTime = 0
};

// ════════════════════════════════════════════════════════════
//  RUNTIME VARIABLES
// ════════════════════════════════════════════════════════════
WiFiClient  wifiClient;

unsigned long lastSensorTime      = 0;
unsigned long lastThingSpeakTime  = 0;
unsigned long lastLCDTime         = 0;
unsigned long lastSerialTime      = 0;
unsigned long lastWiFiRetryTime   = 0;
unsigned long lastEnergyCalcTime  = 0;
unsigned long buzzerOffTime       = 0;

bool     buzzerActive      = false;
bool     buzzerPatternBusy = false;
int      lcdPage           = 0;
int      totalPages        = 3;
AlertCode lastUploadedAlert = ALERT_NORMAL;
AlertCode prevAlert         = ALERT_NORMAL;

// ════════════════════════════════════════════════════════════
//  CUSTOM LCD CHARACTERS
// ════════════════════════════════════════════════════════════
byte charLightning[8] = {0b00100,0b01100,0b11111,0b00110,0b00111,0b11111,0b00110,0b00010};
byte charTick[8]      = {0b00000,0b00001,0b00011,0b10110,0b11100,0b01000,0b00000,0b00000};
byte charBell[8]      = {0b00100,0b01110,0b01110,0b01110,0b11111,0b00000,0b00100,0b00000};
byte charWifi[8]      = {0b00000,0b01110,0b10001,0b00100,0b01010,0b00000,0b00100,0b00000};
byte charLock[8]      = {0b01110,0b10001,0b10001,0b11111,0b11011,0b11011,0b11111,0b00000};
byte charHeart[8]     = {0b00000,0b01010,0b11111,0b11111,0b01110,0b00100,0b00000,0b00000};
byte charUp[8]        = {0b00100,0b01110,0b11111,0b00100,0b00100,0b00100,0b00100,0b00000};
byte charDown[8]      = {0b00000,0b00100,0b00100,0b00100,0b00100,0b11111,0b01110,0b00100};

// ════════════════════════════════════════════════════════════
//  FORWARD DECLARATIONS
// ════════════════════════════════════════════════════════════
void     connectWiFi();
bool     isWiFiConnected();
void     readAllSensors();
float    readRMSVoltage();
float    readRMSCurrent();
float    measureFrequency();
AlertCode detectFaults();
void     handleAlertChange(AlertCode newAlert);
void     setRelay(bool on, const char* reason);
void     triggerBuzzer(unsigned long durationMs);
void     stopBuzzer();
void     buzzPattern(int beeps, int onMs, int offMs);
void     updateLCD();
void     lcdPage0_readings();
void     lcdPage1_power();
void     lcdPage2_status();
void     lcdShowFault(AlertCode code);
void     lcdShowBooting();
void     sendToThingSpeak();
void     accumulateEnergy();
void     printSerialStatus();
String   alertName(AlertCode code);
String   alertEmoji(AlertCode code);
void     blinkLED(int pin, int times, int delayMs);
void     setStatusLEDs(AlertCode code);

// ════════════════════════════════════════════════════════════
//   ███████ ███████ ████████ ██    ██ ██████
//   ██      ██         ██   ██    ██ ██   ██
//   ███████ █████      ██   ██    ██ ██████
//        ██ ██         ██   ██    ██ ██
//   ███████ ███████    ██    ██████  ██
// ════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  delay(300);

  Serial.println();
  Serial.println(F("╔══════════════════════════════════════════════════╗"));
  Serial.println(F("║     SMART GRID MANAGEMENT SYSTEM  v3.0          ║"));
  Serial.println(F("║     ESP32 DevKit — Full Production Build         ║"));
  Serial.println(F("╚══════════════════════════════════════════════════╝"));
  Serial.println();

  // ── GPIO setup ──────────────────────────────────────────
  pinMode(PIN_RELAY,     OUTPUT);
  pinMode(PIN_BUZZER,    OUTPUT);
  pinMode(PIN_LED_RED,   OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_BYPASS,    INPUT_PULLUP);

  // Safe startup state
  digitalWrite(PIN_RELAY,     HIGH);  // Relay ON (power flowing)
  digitalWrite(PIN_BUZZER,    LOW);
  digitalWrite(PIN_LED_RED,   LOW);
  digitalWrite(PIN_LED_GREEN, LOW);

  // ── ADC setup ───────────────────────────────────────────
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);  // Full 0–3.3V range

  // ── LCD setup ───────────────────────────────────────────
  Wire.begin(SDA_PIN, SCL_PIN);
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, charLightning);
  lcd.createChar(1, charTick);
  lcd.createChar(2, charBell);
  lcd.createChar(3, charWifi);
  lcd.createChar(4, charLock);
  lcd.createChar(5, charHeart);
  lcd.createChar(6, charUp);
  lcd.createChar(7, charDown);

  lcdShowBooting();

  // ── Startup blink ────────────────────────────────────────
  blinkLED(PIN_LED_GREEN, 2, 150);
  blinkLED(PIN_LED_RED,   2, 150);

  // ── WiFi connect ─────────────────────────────────────────
  connectWiFi();
  ThingSpeak.begin(wifiClient);

  // ── Ready ────────────────────────────────────────────────
  grid.relayOn = true;
  digitalWrite(PIN_LED_GREEN, HIGH);

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("System Ready    ");
  lcd.setCursor(0, 1); lcd.write(3); lcd.print(" ");
  lcd.print(WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : "No WiFi");
  delay(2000);

  Serial.println(F("[SYSTEM] Boot complete. Monitoring active.\n"));
  Serial.println(F("  Time(ms)   Voltage    Current    Power      Freq    Alert      Relay  Bypass"));
  Serial.println(F("  ─────────  ─────────  ─────────  ─────────  ──────  ─────────  ─────  ──────"));
}

// ════════════════════════════════════════════════════════════
//   ██       ██████   ██████  ██████
//   ██      ██    ██ ██    ██ ██   ██
//   ██      ██    ██ ██    ██ ██████
//   ██      ██    ██ ██    ██ ██
//   ███████  ██████   ██████  ██
// ════════════════════════════════════════════════════════════
void loop() {
  unsigned long now = millis();

  // ── 1. READ SENSORS ─────────────────────────────────────
  if (now - lastSensorTime >= SENSOR_INTERVAL) {
    lastSensorTime = now;

    readAllSensors();
    accumulateEnergy();

    AlertCode newAlert = detectFaults();

    // Handle alert state change
    if (newAlert != prevAlert) {
      handleAlertChange(newAlert);
      prevAlert = newAlert;
    }
    grid.alertCode = newAlert;

    setStatusLEDs(grid.alertCode);
  }

  // ── 2. AUTO-RESET CHECK ──────────────────────────────────
  if (AUTO_RESET_ON_CLEAR &&
      !grid.relayOn &&
      grid.alertCode == ALERT_NORMAL &&
      grid.faultClearTime > 0 &&
      now - grid.faultClearTime >= RESET_DELAY_MS) {
    setRelay(true, "Auto-reset after fault cleared");
    grid.faultClearTime = 0;
    lcd.clear();
    lcd.setCursor(0,0); lcd.write(1); lcd.print(" Fault Cleared");
    lcd.setCursor(0,1); lcd.print("Relay restored  ");
    delay(1500);
  }

  // ── 3. BUZZER AUTO-OFF ───────────────────────────────────
  if (buzzerActive && now >= buzzerOffTime) {
    stopBuzzer();
  }

  // ── 4. LCD UPDATE ────────────────────────────────────────
  if (now - lastLCDTime >= LCD_PAGE_INTERVAL) {
    lastLCDTime = now;
    updateLCD();
  }

  // ── 5. SERIAL PRINT ──────────────────────────────────────
  if (now - lastSerialTime >= SERIAL_INTERVAL) {
    lastSerialTime = now;
    printSerialStatus();
  }

  // ── 6. THINGSPEAK UPLOAD ─────────────────────────────────
  if (now - lastThingSpeakTime >= THINGSPEAK_INTERVAL) {
    lastThingSpeakTime = now;
    if (!isWiFiConnected()) {
      if (now - lastWiFiRetryTime >= WIFI_RETRY_INTERVAL) {
        lastWiFiRetryTime = now;
        connectWiFi();
      }
    } else {
      sendToThingSpeak();
    }
  }
}

// ════════════════════════════════════════════════════════════
//  SENSOR READING FUNCTIONS
// ════════════════════════════════════════════════════════════

void readAllSensors() {
  grid.voltage     = readRMSVoltage();
  grid.current     = readRMSCurrent();
  grid.frequency   = measureFrequency();
  grid.bypassActive = (digitalRead(PIN_BYPASS) == LOW);

  // Power factor: fixed 0.92 estimate
  // For accurate PF, replace ZMPT101B+ACS712 with ATM90E32 IC
  grid.powerFactor = 0.92f;
  grid.power       = grid.voltage * grid.current * grid.powerFactor;
}

// ── RMS Voltage (ZMPT101B) ───────────────────────────────────
float readRMSVoltage() {
  float sumSq = 0.0f;
  for (int i = 0; i < RMS_SAMPLES; i++) {
    float raw     = (float)analogRead(PIN_VOLTAGE);
    float v       = (raw / ADC_MAX) * ADC_REF;
    float centered = v - VOLTAGE_OFFSET;
    sumSq += centered * centered;
    delayMicroseconds(200); // 100 samples × 200µs = 20ms = one 50Hz cycle
  }
  float rms = sqrtf(sumSq / RMS_SAMPLES);
  return rms * VOLTAGE_CAL * 1000.0f;
  // NOTE: Multiply result by calibration factor until it matches your multimeter
}

// ── RMS Current (ACS712-20A) ──────────────────────────────────
float readRMSCurrent() {
  float sumSq = 0.0f;
  for (int i = 0; i < RMS_SAMPLES; i++) {
    float raw     = (float)analogRead(PIN_CURRENT);
    float v       = (raw / ADC_MAX) * ADC_REF;
    float current = (v - ACS_ZERO) / ACS_SENSITIVITY;
    sumSq += current * current;
    delayMicroseconds(200);
  }
  float rms = sqrtf(sumSq / RMS_SAMPLES);
  return (rms < 0.05f) ? 0.0f : rms; // Zero-threshold filter: ignore noise below 50mA
}

// ── Frequency (zero-crossing count) ─────────────────────────
float measureFrequency() {
  unsigned long startMs = millis();
  int   crossings       = 0;
  bool  lastAbove       = false;
  float midPoint        = ADC_MAX / 2.0f;

  while (millis() - startMs < 100) { // Count over 100ms window
    bool above = ((float)analogRead(PIN_VOLTAGE) > midPoint);
    if (above && !lastAbove) crossings++; // Rising edge only
    lastAbove = above;
    delayMicroseconds(500);
  }

  float freq = crossings * 10.0f; // Rising edges per 100ms × 10 = Hz
  return (freq >= 45.0f && freq <= 65.0f) ? freq : 50.0f; // Clamp to valid range
}

// ── Energy accumulation (Wh) ──────────────────────────────────
void accumulateEnergy() {
  unsigned long now = millis();
  if (lastEnergyCalcTime > 0) {
    float hours = (now - lastEnergyCalcTime) / 3600000.0f;
    grid.energy_wh += grid.power * hours;
  }
  lastEnergyCalcTime = now;
}

// ════════════════════════════════════════════════════════════
//  FAULT DETECTION
// ════════════════════════════════════════════════════════════

AlertCode detectFaults() {
  // Priority order: Short circuit > Overload > Theft > Voltage faults

  if (grid.current >= SHORT_CIRCUIT_CURRENT) {
    return ALERT_SHORT_CIRCUIT;
  }
  if (grid.current >= OVERLOAD_CURRENT) {
    return ALERT_OVERLOAD;
  }
  if (grid.bypassActive) {
    return ALERT_THEFT;
  }
  if (grid.voltage > 10.0f && grid.voltage >= OVERVOLTAGE_LIMIT) {
    return ALERT_OVERVOLTAGE;
  }
  if (grid.voltage > 10.0f && grid.voltage <= UNDERVOLTAGE_LIMIT) {
    return ALERT_UNDERVOLTAGE;
  }
  return ALERT_NORMAL;
}

// ════════════════════════════════════════════════════════════
//  ALERT HANDLING
// ════════════════════════════════════════════════════════════

void handleAlertChange(AlertCode newAlert) {
  if (newAlert == ALERT_NORMAL && prevAlert != ALERT_NORMAL) {
    // ── Fault just cleared ──────────────────────────────
    Serial.printf("\n[ALERT] Fault CLEARED: %s → Normal\n\n", alertName(prevAlert).c_str());
    grid.faultClearTime = millis();
    // LEDs handled by setStatusLEDs in loop
    return;
  }

  if (newAlert == ALERT_NORMAL) return;

  // ── New fault detected ────────────────────────────────
  grid.faultStart = millis();
  Serial.printf("\n[ALERT] *** %s FAULT DETECTED ***\n", alertName(newAlert).c_str());
  Serial.printf("        V=%.1fV  I=%.3fA  P=%.1fW\n\n",
    grid.voltage, grid.current, grid.power);

  // ── Trip relay for critical faults ───────────────────
  switch (newAlert) {
    case ALERT_SHORT_CIRCUIT:
      setRelay(false, "Short circuit protection");
      break;
    case ALERT_OVERLOAD:
      setRelay(false, "Overload protection");
      break;
    case ALERT_THEFT:
      setRelay(false, "Theft detected — circuit isolated");
      break;
    default:
      // Overvoltage/undervoltage: warn but don't trip (optional: change to setRelay(false))
      break;
  }

  // ── Buzzer pattern per fault type ────────────────────
  switch (newAlert) {
    case ALERT_SHORT_CIRCUIT:
      triggerBuzzer(6000);  // 6 second continuous
      break;
    case ALERT_THEFT:
      triggerBuzzer(4000);  // 4 second continuous
      break;
    case ALERT_OVERLOAD:
      triggerBuzzer(3000);  // 3 second continuous
      break;
    case ALERT_OVERVOLTAGE:
    case ALERT_UNDERVOLTAGE:
      triggerBuzzer(1000);  // 1 second warning beep
      break;
    default:
      break;
  }

  // ── Immediate LCD alert ───────────────────────────────
  lcdShowFault(newAlert);
}

// ════════════════════════════════════════════════════════════
//  RELAY CONTROL
// ════════════════════════════════════════════════════════════

void setRelay(bool on, const char* reason) {
  grid.relayOn = on;
  digitalWrite(PIN_RELAY, on ? HIGH : LOW);
  Serial.printf("[RELAY] %s — reason: %s\n", on ? "ON" : "OFF", reason);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(on ? "Relay: ON       " : "Relay: OFF      ");
  lcd.setCursor(0, 1);
  lcd.print(reason);
  delay(800);
}

// ════════════════════════════════════════════════════════════
//  STATUS LEDs
// ════════════════════════════════════════════════════════════

void setStatusLEDs(AlertCode code) {
  if (code == ALERT_NORMAL && grid.relayOn) {
    digitalWrite(PIN_LED_GREEN, HIGH);
    digitalWrite(PIN_LED_RED,   LOW);
  } else if (code == ALERT_NORMAL && !grid.relayOn) {
    // Fault cleared but waiting for auto-reset — slow blink green
    static unsigned long lastBlink = 0;
    static bool blinkState = false;
    if (millis() - lastBlink > 500) {
      blinkState = !blinkState;
      lastBlink = millis();
      digitalWrite(PIN_LED_GREEN, blinkState);
      digitalWrite(PIN_LED_RED, LOW);
    }
  } else {
    // Active fault — red on, green off
    digitalWrite(PIN_LED_GREEN, LOW);
    digitalWrite(PIN_LED_RED,   HIGH);
  }
}

// ════════════════════════════════════════════════════════════
//  BUZZER CONTROL
// ════════════════════════════════════════════════════════════

void triggerBuzzer(unsigned long durationMs) {
  if (!buzzerActive) {
    digitalWrite(PIN_BUZZER, HIGH);
    buzzerOffTime  = millis() + durationMs;
    buzzerActive   = true;
  }
}

void stopBuzzer() {
  digitalWrite(PIN_BUZZER, LOW);
  buzzerActive = false;
}

// ════════════════════════════════════════════════════════════
//  LCD DISPLAY
// ════════════════════════════════════════════════════════════

void updateLCD() {
  if (grid.alertCode != ALERT_NORMAL) {
    lcdShowFault(grid.alertCode);
    return;
  }

  // Cycle through normal info pages
  switch (lcdPage % totalPages) {
    case 0: lcdPage0_readings(); break;
    case 1: lcdPage1_power();    break;
    case 2: lcdPage2_status();   break;
  }
  lcdPage++;
}

// Page 0: Voltage + Current
void lcdPage0_readings() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(0); // lightning
  lcd.print("V:");
  lcd.print(grid.voltage, 1);
  lcd.print("V  ");

  lcd.setCursor(0, 1);
  lcd.print("  I:");
  lcd.print(grid.current, 2);
  lcd.print("A  ");
}

// Page 1: Power + Frequency
void lcdPage1_power() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("P:");
  lcd.print(grid.power, 0);
  lcd.print("W");
  // Right-align frequency
  lcd.setCursor(9, 0);
  lcd.print("F:");
  lcd.print(grid.frequency, 0);
  lcd.print("Hz");

  lcd.setCursor(0, 1);
  lcd.print("PF:");
  lcd.print(grid.powerFactor, 2);
  lcd.setCursor(7, 1);
  lcd.print("E:");
  lcd.print(grid.energy_wh / 1000.0f, 2); // Show as kWh
  lcd.print("kWh");
}

// Page 2: Status + WiFi
void lcdPage2_status() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(1); // tick
  lcd.print("Relay:");
  lcd.print(grid.relayOn ? "ON " : "OFF");
  lcd.setCursor(10, 0);
  lcd.write(3); // wifi
  lcd.print(isWiFiConnected() ? "ON " : "OFF");

  lcd.setCursor(0, 1);
  lcd.write(5); // heart
  lcd.print("Normal ");
  lcd.print(millis() / 60000);
  lcd.print("min");
}

// Fault display — takes over LCD immediately
void lcdShowFault(AlertCode code) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(2); // bell
  lcd.print(" FAULT ALERT!");
  lcd.setCursor(0, 1);
  switch (code) {
    case ALERT_OVERLOAD:
      lcd.print("OVERLOAD!");
      lcd.setCursor(10, 1);
      lcd.print(grid.current, 1); lcd.print("A");
      break;
    case ALERT_SHORT_CIRCUIT:
      lcd.print("SHORT CIRCUIT!! ");
      break;
    case ALERT_OVERVOLTAGE:
      lcd.print("OVERVOLTAGE!");
      lcd.setCursor(12, 1);
      lcd.print((int)grid.voltage);
      break;
    case ALERT_UNDERVOLTAGE:
      lcd.print("UNDERVOLTAGE!");
      lcd.setCursor(13, 1);
      lcd.print((int)grid.voltage);
      break;
    case ALERT_THEFT:
      lcd.print("THEFT DETECTED! ");
      break;
    default:
      lcd.print("UNKNOWN FAULT   ");
      break;
  }
}

// Boot screen
void lcdShowBooting() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Smart Grid v3.0 ");
  lcd.setCursor(0, 1); lcd.print("Initialising... ");
  delay(1200);
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Connecting WiFi ");
}

// ════════════════════════════════════════════════════════════
//  THINGSPEAK CLOUD UPLOAD
// ════════════════════════════════════════════════════════════

void sendToThingSpeak() {
  Serial.print(F("[ThingSpeak] Uploading... "));

  ThingSpeak.setField(1, (float)grid.voltage);
  ThingSpeak.setField(2, (float)grid.current);
  ThingSpeak.setField(3, (float)grid.power);
  ThingSpeak.setField(4, (float)grid.powerFactor);
  ThingSpeak.setField(5, (float)grid.frequency);
  ThingSpeak.setField(6, (int)grid.alertCode);

  // Status string visible in ThingSpeak dashboard
  String status = "Alert:" + alertName(grid.alertCode)
                + " Relay:" + (grid.relayOn ? "ON" : "OFF")
                + " Energy:" + String(grid.energy_wh / 1000.0f, 3) + "kWh";
  ThingSpeak.setStatus(status);

  int httpCode = ThingSpeak.writeFields(TS_CHANNEL_ID, TS_WRITE_API);

  if (httpCode == 200) {
    Serial.println(F("OK"));
    // If alert changed since last upload, note it
    if (grid.alertCode != lastUploadedAlert) {
      Serial.printf("[ThingSpeak] Alert state changed: %s → %s\n",
        alertName(lastUploadedAlert).c_str(),
        alertName(grid.alertCode).c_str());
      lastUploadedAlert = grid.alertCode;
    }
  } else {
    Serial.printf("FAILED (HTTP %d)\n", httpCode);
  }
}

// ════════════════════════════════════════════════════════════
//  WIFI CONNECTION
// ════════════════════════════════════════════════════════════

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.printf("[WiFi] Connecting to %s", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Connecting WiFi ");

  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 40) {
    delay(500);
    Serial.print(F("."));
    lcd.setCursor(tries % 16, 1);
    lcd.print("-");
    tries++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("[WiFi] Connected! IP: %s  RSSI: %d dBm\n",
      WiFi.localIP().toString().c_str(), WiFi.RSSI());
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("WiFi Connected! ");
    lcd.setCursor(0, 1); lcd.print(WiFi.localIP().toString());
    blinkLED(PIN_LED_GREEN, 3, 100);
    delay(1500);
  } else {
    Serial.println(F("[WiFi] Connection FAILED — running offline"));
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("WiFi FAILED!    ");
    lcd.setCursor(0, 1); lcd.print("Offline mode    ");
    blinkLED(PIN_LED_RED, 5, 100);
    delay(1000);
  }
}

bool isWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

// ════════════════════════════════════════════════════════════
//  SERIAL DEBUG OUTPUT
// ════════════════════════════════════════════════════════════

void printSerialStatus() {
  Serial.printf("  %9lu  %6.1fV     %6.3fA     %7.1fW    %5.1fHz  %-12s %s    %s\n",
    millis(),
    grid.voltage,
    grid.current,
    grid.power,
    grid.frequency,
    alertName(grid.alertCode).c_str(),
    grid.relayOn    ? "ON " : "OFF",
    grid.bypassActive ? "ACTIVE" : "clear "
  );
}

// ════════════════════════════════════════════════════════════
//  UTILITY FUNCTIONS
// ════════════════════════════════════════════════════════════

String alertName(AlertCode code) {
  switch (code) {
    case ALERT_NORMAL:        return "Normal";
    case ALERT_OVERLOAD:      return "Overload";
    case ALERT_SHORT_CIRCUIT: return "ShortCircuit";
    case ALERT_OVERVOLTAGE:   return "Overvoltage";
    case ALERT_UNDERVOLTAGE:  return "Undervoltage";
    case ALERT_THEFT:         return "TheftDetected";
    default:                  return "Unknown";
  }
}

String alertEmoji(AlertCode code) {
  switch (code) {
    case ALERT_NORMAL:        return "[OK]";
    case ALERT_OVERLOAD:      return "[!!]";
    case ALERT_SHORT_CIRCUIT: return "[SC]";
    case ALERT_OVERVOLTAGE:   return "[OV]";
    case ALERT_UNDERVOLTAGE:  return "[UV]";
    case ALERT_THEFT:         return "[TH]";
    default:                  return "[??]";
  }
}

void blinkLED(int pin, int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH); delay(delayMs);
    digitalWrite(pin, LOW);  delay(delayMs);
  }
}

/*
 * ════════════════════════════════════════════════════════════
 *  CALIBRATION GUIDE
 * ════════════════════════════════════════════════════════════
 *
 *  VOLTAGE CALIBRATION:
 *  1. Power up with AC connected
 *  2. Read voltage on Serial Monitor
 *  3. Measure actual voltage with a multimeter
 *  4. New VOLTAGE_CAL = current VOLTAGE_CAL × (multimeter / serial reading)
 *  Example: Serial=150V, Multimeter=228V → new CAL = 0.5 × (228/150) = 0.76
 *
 *  CURRENT ZERO CALIBRATION:
 *  1. Power up with NO load connected
 *  2. Read current on Serial Monitor — should be 0.00A
 *  3. If it shows e.g. 0.30A, adjust: new ACS_ZERO = 1.65 + (0.30 × 0.100)
 *  Example: idle reads 0.30A → new ACS_ZERO = 1.65 + 0.030 = 1.680
 *
 *  THINGSPEAK REACT ALERT SETUP:
 *  1. Go to ThingSpeak → Apps → React
 *  2. New React: if Field 6 > 0 → send email
 *  3. Add your email address
 *  4. Set "Run Action" = "Each time condition is met"
 *  This gives you email notification on every fault
 *
 *  LCD ADDRESS:
 *  If LCD shows nothing, run I2C scanner sketch first.
 *  If scanner shows 0x3F, change lcd(0x27...) to lcd(0x3F...)
 *
 *  THRESHOLD TESTING (temporarily lower thresholds):
 *  To test overload: change OVERLOAD_CURRENT to 1.0, plug in any load
 *  To test theft: bring a magnet near the reed switch
 *  To test overvoltage: change OVERVOLTAGE_LIMIT to 220.0 temporarily
 *  Restore all values after testing.
 * ════════════════════════════════════════════════════════════
 */
