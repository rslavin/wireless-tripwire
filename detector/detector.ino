#include <esp_now.h>
#include <WiFi.h>

#define INIT_BUTTON_PIN 4
#define LED_PIN 5
#define TRIGGER_PIN 6
#define ECHO_PIN 7


#define SENSITIVITY 1500
#define MIN_POS_READS 2
#define MAX_PEER_CONNECT_RETRIES 20
#define READS_PER_SECOND 10

int positiveRead = 0;
int delayMS = 0;
float initialDuration;
float duration;
bool wireTripped = false;
uint8_t remoteAddress[] = {0x34, 0x85, 0x18, 0x03, 0x66, 0xB0};
esp_now_peer_info_t peerInfo;


void setup() {
  // initialize pins
  pinMode(INIT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

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

  // ESP-NOW peer information  
  memcpy(peerInfo.peer_addr, remoteAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // add peer
  while (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Unable to connect to peer. Retrying...");
  }

  Serial.println("Successfully connected to peer.");

  delayMS = 1000 / READS_PER_SECOND;

  tareDuration();
}

void loop() {
  duration = getDuration();  
  Serial.printf("Duration: %f\n", duration);

  // Increment positiveRead when the sensitivity threshold is met. Once it hits MIN_POS_READS, 
  // set wireTripped to true. As there are no positive reads, decrement positiveRead to a min 
  // of zero, at which wireTripped is set back to false.

  // positive read
  if (initialDuration - duration > SENSITIVITY) {
    Serial.printf("POSITIVE READ (%f)", positiveRead++);
    // positiveRead++;

    // only trip the wire if there have been MIN_POS_READS consecutive detections
    if (positiveRead >= MIN_POS_READS){
      Serial.printf("\nWIRE TRIPPED\n");
      Serial.printf("Initial: %f\n", initialDuration);
      Serial.printf("Difference: %f\n", (initialDuration - duration));
      wireTripped = true;
      // blink light once
      digitalWrite(LED_PIN, HIGH);
      delay(500);
      digitalWrite(LED_PIN, LOW);
    }
  } else if (positiveRead > 0){ // decrement unless already at 0
    positiveRead--;
  } else if (positiveRead == 0){
    wireTripped = false;
  }
  
  // send wireTripped value to peer
  esp_err_t result = esp_now_send(remoteAddress, (uint8_t *) &wireTripped, sizeof(bool));

  // reset duration/distance when button is pressed
  if(digitalRead(INIT_BUTTON_PIN) == LOW) {
    Serial.println("button pressed");
    tareDuration();
  }
  delay(delayMS);
}

int getDuration(){
  Serial.println("getDuration");
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  return pulseIn(ECHO_PIN, HIGH);
}

// calibrate the duration (distance)
void tareDuration(){
  Serial.println("tareDuration");
  initialDuration = getDuration();
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  wireTripped = false;
  Serial.printf("Initial duration set: %f\n", initialDuration);
}
