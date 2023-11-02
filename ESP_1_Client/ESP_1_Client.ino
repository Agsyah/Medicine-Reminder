// ESP32 #1: TCP Client + Display

#include <WiFi.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>
#include <Keypad.h>
#include <Preferences.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1

const char* ssid = "WIFI_SSID";                 // Ganti WiFi yang digunakan
const char* password = "WIFI_PASSWORD";         // Ganti sesuai password WiFi
const char* serverAddress = "WIFI_ADDRESS";     // Isi IP sesuai yang didapat pada ESP_2_Server

const int serverPort = 4080;                    // Default (4080)

const int buzzerPin = 0;                       // Ganti pin Buzzer
bool buzzerOn = false;

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {14, 27, 26, 25};
byte colPins[COLS] = {33, 32, 18, 19};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RTC_DS3231 rtc;

byte alarmHour;
byte alarmMinute;
byte alarmSecond;
byte currentDigit = 0;
bool alarmActive = false;

char days[7][12] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};

Preferences preferences;

WiFiClient TCPclient;

void setup() {
  Serial.begin(9600);

  Wire.begin();
  rtc.begin();

  // ==============================================================================================
  // Koneksi Wi-Fi

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Menghubungkan WiFi...");
  }
  Serial.println("Terhubung ke WiFi");

  // Menghubungkan ke perangkat server (ESP_2_Server)
  if (TCPclient.connect(serverAddress, serverPort)) {
    Serial.println("Terhubung ke TCP server (ESP_2_Server)");
  } else {
    Serial.println("Gagal terhubung ke TCP server, cek kembali ip pada (ESP_2_Server).");
  }

  // ==============================================================================================
  if (rtc.lostPower()) 
  {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Buzzer
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, HIGH);

  // Display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Alarm
  preferences.begin("alarm", false);
  alarmHour = preferences.getUChar("hour", 0);
  alarmMinute = preferences.getUChar("minute", 0);
  alarmSecond = preferences.getUChar("second", 0);
  alarmActive = preferences.getBool("active", false);
  preferences.end();

  Serial.print("Loaded alarm data: ");
  Serial.print(formatDigits(alarmHour));
  Serial.print(':');
  Serial.print(formatDigits(alarmMinute));
  Serial.print(':');
  Serial.println(formatDigits(alarmSecond));

  display.setCursor(0, 0);
  display.println("Alarm Pengingat");
  display.println("Minum Obat");
  display.display();
  delay(3000);
  display.clearDisplay();

  // Menampilkan informasi akses point pada layar OLED
  display.setCursor(0, 0);
  display.println("Access Point");
  display.print("SSID: ");
  display.println(ssid);
  display.print("IP: ");
  display.println(WiFi.localIP());
  display.display();
}

void loop() {

  // Koneksi WiFi
  if (!TCPclient.connected()) {
    Serial.println("Koneksi terputus");
    TCPclient.stop();

    if (TCPclient.connect(serverAddress, serverPort)) {
      Serial.println("Mencoba terhubung ke server");
    } else {
      Serial.println("Gagal terhubung ke server");
    }
  }

  // Vibration
  char key = keypad.getKey();

  DateTime now = rtc.now();
  display.clearDisplay();

  display.setCursor(0, 0);
  display.println(days[now.dayOfTheWeek()]);
  
  display.print(formatDigits(now.day()));
  display.print('/');
  display.print(formatDigits(now.month()));
  display.print('/');
  display.println(now.year(), DEC);

  display.print(formatDigits(now.hour()));
  display.print(':');
  display.print(formatDigits(now.minute()));
  display.print(':');
  display.println(formatDigits(now.second()));
  display.print("Buat Alarm (#)");

  if (key == '#') {
    setAlarm();
  }

  byte currentHour = now.hour();
  byte currentMinute = now.minute();
  byte currentSecond = now.second();
  
  if (alarmHour == currentHour && alarmMinute == currentMinute && alarmSecond == currentSecond && !alarmActive && !buzzerOn) {
    alarmActive = true;
    displayAlarmMessage();
  }

  display.display();
  delay(1000);
}

String formatDigits(byte digits) {
  String formatted = String(digits);
  if (digits < 10) {
    formatted = '0' + formatted;
  }
  return formatted;
}

void displayAlarmMessage() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Minum Obat!");
  display.println("Matikan Alarm (D)");

  display.display();
  Serial.println("Waktunya Minum Obat!");

  // Mengaktifkan buzzer
  digitalWrite(buzzerPin, LOW);
  buzzerOn = true; // Mengaktifkan flag buzzer
  Serial.println("Buzzer turned on."); // Print to serial monitor
  
  // Kirim perintah ke server
  TCPclient.write('1');
  TCPclient.flush();

  while (alarmActive) {
    char key = keypad.getKey();
    if (key == 'D') {
      alarmActive = false;

      // Mematikan buzzer
      digitalWrite(buzzerPin, HIGH);
      buzzerOn = false; // Menonaktifkan flag buzzer
      Serial.println("Buzzer turned off."); // Print to serial monitor

      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.println("Alarm dimatikan"); // Menambahkan tulisan "Alarm dimatikan"
      display.display();

      Serial.println("Alarm dimatikan");
    }
  }
}

void setAlarm() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Masukkan jam (HH)");
  display.display();

  currentDigit = 0;
  while (currentDigit < 6) {
    char key = keypad.getKey();
    if (key >= '0' && key <= '9') {
      if (currentDigit == 0) {
        alarmHour = (key - '0') * 10;
        currentDigit = 1;
        display.print(key);
        display.display();
      } else if (currentDigit == 1) {
        alarmHour += (key - '0');
        currentDigit = 2;
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Masukkan menit (MM)");
        display.display();
      } else if (currentDigit == 2) {
        alarmMinute = (key - '0') * 10;
        currentDigit = 3;
        display.print(key);
        display.display();
      } else if (currentDigit == 3) {
        alarmMinute += (key - '0');
        currentDigit = 4;
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Masukkan detik (SS)");
        display.display();
      } else if (currentDigit == 4) {
        alarmSecond = (key - '0') * 10;
        currentDigit = 5;
        display.print(key);
        display.display();
      } else if (currentDigit == 5) {
        alarmSecond += (key - '0');
        currentDigit = 6;
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Alarm Tersimpan");
        display.display();

        preferences.begin("alarm", false);
        preferences.putUChar("hour", alarmHour);
        preferences.putUChar("minute", alarmMinute);
        preferences.putUChar("second", alarmSecond);
        preferences.end();

        Serial.print("Stored alarm data: ");
        Serial.print(formatDigits(alarmHour));
        Serial.print(':');
        Serial.print(formatDigits(alarmMinute));
        Serial.print(':');
        Serial.println(formatDigits(alarmSecond));

        delay(2000);
        display.clearDisplay();
      }
    }
  }
}
