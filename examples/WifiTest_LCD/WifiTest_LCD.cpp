#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  
  lcd.setCursor(0,0); 
  lcd.print("Connecting...");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println("\nConnected!");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  
  // Inicializa o LCD
  lcd.init();
  lcd.backlight();
  
  initWiFi(); 
  
  
  
  
}

void loop() {
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
  // Limpa o LCD e mostra o sinal
  lcd.clear();
  lcd.setCursor(0,0); 
  lcd.print("RSSI: ");
  lcd.print(WiFi.RSSI()); // Imprime o valor logo após o texto
  lcd.print(" dBm");
  delay(2000);
}