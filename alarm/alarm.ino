#include <esp_now.h>
#include <WiFi.h>

#define LED_CONNECTION_PIN 4
#define LED_PIN 5
#define BUZZER_PIN 10
#define TIMEOUT_LOOPS 10

bool wireTripped = false;
int timeoutCount = TIMEOUT_LOOPS; // begin with no connection

// this device: 34:85:18:03:66:B0

void setup() {
  // initialize pins
  pinMode(LED_CONNECTION_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Set up serial
  Serial.begin(9600);
  // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
  delay(1500); 

  // set up wifi
  WiFiClass::mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP_NOW. Exiting.");
    return;
  }  

  esp_now_register_recv_cb(onDataRecv);

  Serial.println("Successfully connected to peer.");
}

void loop() {
  timeoutCount++;
  Serial.println(timeoutCount);

  if (timeoutCount > TIMEOUT_LOOPS) {
    digitalWrite(LED_CONNECTION_PIN, LOW); 
    digitalWrite(LED_PIN, LOW);
    buzz(500);
  } else {
    Serial.println("connected");
    digitalWrite(LED_CONNECTION_PIN, HIGH);
  }

  if (wireTripped) {
    digitalWrite(LED_PIN, HIGH);
    buzz(500);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
  delay(100);
    
}

void onDataRecv(const uint8_t *mac_addr, const uint8_t *data, int len) {
    memcpy(&wireTripped, data, sizeof(wireTripped));
    timeoutCount = 0;
}

void buzz(int microSeconds) {
  tone(BUZZER_PIN, 500);
  delay(microSeconds);
  noTone(BUZZER_PIN);
}

