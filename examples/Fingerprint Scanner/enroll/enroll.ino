#include <Adafruit_Fingerprint.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
HardwareSerial fpSerial(2);
Adafruit_Fingerprint finger(&fpSerial);

const int ledVerde = 2; // Pino D2
const int ledVerm = 4; // Pino D4

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  pinMode(ledVerde, OUTPUT);
  pinMode(ledVerm, OUTPUT);
  fpSerial.begin(57600, SERIAL_8N1, 16, 17);
  finger.begin(57600);

  if (!finger.verifyPassword()) {
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Sensor not found!");
    Serial.println("Sensor not found!"); while(1);
  }
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Sensor ready");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Type slot ID");
  lcd.setCursor(0, 1); lcd.print("ID 0-299 + Enter:");
  delay(3000);
  Serial.println("Sensor ready. Type slot ID (0-299) + Enter:");
}

void loop() {
  if (!Serial.available()) return;
  uint16_t id = Serial.parseInt();
  Serial.read();
  if (id > 299) { Serial.println("ID must be 0-299"); return; }
  lcd.clear();
  Serial.printf("\nEnrolling at slot #%d\n", id);
  lcd.setCursor(0, 0); lcd.print("Enrolling at");
  lcd.setCursor(0, 1); lcd.print("at slot #");
  lcd.setCursor(0, 10); lcd.print(id);
  delay(3000);

  // ── Scan 1 ──────────────────────────────────
  Serial.println("Place finger on sensor...");
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Place finger");
  lcd.setCursor(0, 1); lcd.print("on sensor...");
  delay(3000);
  while (finger.getImage() != FINGERPRINT_OK);
  if (finger.image2Tz(1) != FINGERPRINT_OK) {
    Serial.println("Image too blurry, try again"); return;
  }

  Serial.println("Lift finger...");
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Lift finger...");
  delay(3000);
  while (finger.getImage() != FINGERPRINT_NOFINGER);
  delay(200);

  // ── Scan 2 ──────────────────────────────────
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Place SAME");
  lcd.setCursor(0, 1); lcd.print("finger again...");
  Serial.println("Place SAME finger again...");
  delay(3000);
  while (finger.getImage() != FINGERPRINT_OK);
  if (finger.image2Tz(2) != FINGERPRINT_OK) {
    Serial.println("Image too blurry, try again"); return;
  }

  // ── Merge + store ────────────────────────────
  if (finger.createModel() != FINGERPRINT_OK) {
    Serial.println("Scans didn't match! Try again."); 
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Scans didn't");
    lcd.setCursor(0, 1); lcd.print("match!");
    digitalWrite(ledVerm, HIGH);
    delay(3000);
    digitalWrite(ledVerm, LOW);
    return;
  }
  if (finger.storeModel(id) == FINGERPRINT_OK) {
    Serial.printf("Enrolled successfully at slot #%d\n", id);
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Enrolled ");
    lcd.setCursor(0, 1); lcd.print("successfully");
    digitalWrite(ledVerde, HIGH);
    delay(3000);
    digitalWrite(ledVerde, LOW);
  } else {
    Serial.println("Store failed!");
  }
}