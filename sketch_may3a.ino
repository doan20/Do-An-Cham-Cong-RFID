#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ================= WIFI & GOOGLE SCRIPT =================
const char* ssid = "testwifi";
const char* password = "12345678";

String GOOGLE_SCRIPT_URL = "https://script.google.com/macros/s/AKfycbyn9kIf87qniFQr5OKnJ3QElPnSC91WlUFDDSg19PYoIt6TKpAR4XMPljpyRxkw4R8rJA/exec";

// ================= CHÂN KẾT NỐI =================
// RFID RC522 (SPI custom)
#define SS_PIN   5
#define RST_PIN  19
#define SCK_PIN  18
#define MISO_PIN 17
#define MOSI_PIN 16

// Buzzer
#define BUZZER_PIN 13

// LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);

// RFID
MFRC522 mfrc522(SS_PIN, RST_PIN);

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("He Thong Cham Cong");

  // SPI custom (QUAN TRỌNG)
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);

  // RFID init
  mfrc522.PCD_Init();

  delay(1000);
  Serial.println("RFID ready");

  // WiFi connect
  lcd.setCursor(0, 1);
  lcd.print("Ket noi WiFi...");
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Da Ket Noi!");
  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Hay Quet The...");
}

// ================= LOOP =================
void loop() {
  // Không có thẻ → bỏ qua
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // ===== ĐỌC UID =====
  String uidString = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uidString += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    uidString += String(mfrc522.uid.uidByte[i], HEX);
  }
  uidString.toUpperCase();
  uidString.trim();

  Serial.println("Ma the: " + uidString);

  // ===== BUZZER =====
  digitalWrite(BUZZER_PIN, HIGH);
  delay(200);
  digitalWrite(BUZZER_PIN, LOW);

  // ===== LCD =====
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Dang Gui Du Lieu");
  lcd.setCursor(0, 1);
  lcd.print(uidString);

  // ===== GỬI GOOGLE =====
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = GOOGLE_SCRIPT_URL + "?uid=" + uidString;

    http.begin(url);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println(httpCode);
      Serial.println(payload);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Cham Cong Xong!");
    } else {
      Serial.println("Loi HTTP");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Loi Mang!");
    }

    http.end();
  }

  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Hay Quet The...");

  // Dừng thẻ để tránh đọc lặp
  mfrc522.PICC_HaltA();
}