// ESP32 #2: TCP SERVER + VIBRATOR

#include <WiFi.h>


#define PIN_BUZZER      0                   // Masukkan pin untuk buzzer
#define PIN_VIBRATOR    0                   // Masukkan pin untuk vibrator

#define SERVER_PORT     4080

const char* ssid = "WIFI_SSID";             // Ganti SSID WiFi
const char* password = "WIFI_PASSWORD";     // Ganti PASSWORD WiFi

WiFiServer TCPserver(SERVER_PORT);

void setup() {
  Serial.begin(9600);
  
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_VIBRATOR, OUTPUT);
  
  digitalWrite(PIN_BUZZER, HIGH);           // Set buzzer pada gelang ke HIGH sebagai default
  digitalWrite(PIN_VIBRATOR, LOW);          // Matikan motor getar pada awal

  // Koneksi Wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Menghubungkan ke WiFi...");
  }
  Serial.println("Terhubung ke WiFi");

  // Print alamat ip
  Serial.print(">>> Server IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println(">>> Isi ip ini pada ESP_1_Client");

  TCPserver.begin();
}

void loop() {
  // Menunggu perintah dari ESP_1_Client
  WiFiClient client = TCPserver.available();

  if (client) {
    // Read the command from the TCP client:
    char command = client.read();

    Serial.print("ESP_1_Server: - Menerima perintah: ");
    Serial.println(command);

    if (command == '1') {
      Vibrate_On();
    }

    client.stop();
  }
}

// List Fungsi

void Vibrate_On() {
  digitalWrite(PIN_BUZZER, LOW);      // Aktifkan Buzzer
  digitalWrite(PIN_VIBRATOR, HIGH);   // Aktifkan Vibrator
  
  delay(5000);                        // Durasi notifikasi 5 detik

  digitalWrite(PIN_BUZZER, HIGH);     // Stop Buzzer
  digitalWrite(PIN_VIBRATOR, LOW);    // Stop Vibrator
}
