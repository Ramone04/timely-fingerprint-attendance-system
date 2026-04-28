#pragma once

// ── WiFi ────────────────────────────────────────────────────
#define WIFI_SSID     "your_ssid"
#define WIFI_PASS     "your_password"

// ── HiveMQ ──────────────────────────────────────────────────
// Port 8883 = TLS/SSL (required for HiveMQ Cloud)
// The CA certificate (ISRG Root X1 / Let's Encrypt) is embedded in mqtt_manager.cpp
#define MQTT_HOST     "your-cluster.s1.eu.hivemq.cloud"
#define MQTT_PORT     8883
#define MQTT_USER     "ESP32"
#define MQTT_PASS     "your_mqtt_password"
#define MQTT_CLIENT   "ESP32_Client"

// ── MQTT Topics ─────────────────────────────────────────────
#define TOPIC_ENROLL_USERID   "Enroll/UserID"
#define TOPIC_ENROLL_NOME     "Enroll/Nome"

// ── Laravel API ───────────────────────────────────────────────
#define ENROLL_STATUS_URL     "https://timely.mindshaker.com/esp32/enroll-status"
#define SCAN_RESULT_URL       "https://timely.mindshaker.com/esp32/scan-result"

// ── Hardware pins ────────────────────────────────────────────
#define FP_RX_PIN     16      // Sensor TX → GPIO16 (ESP RX2)
#define FP_TX_PIN     17      // Sensor RX ← GPIO17 (ESP TX2)
#define FP_BAUD       57600

#define LCD_SDA_PIN   21
#define LCD_SCL_PIN   22
#define LCD_ADDRESS   0x27
#define LCD_COLS      16
#define LCD_ROWS      2
