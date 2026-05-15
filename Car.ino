#include <esp_now.h>
#include <WiFi.h>
typedef struct struct_message {
  float roll;
  float pitch;
} struct_message;
struct_message incomingData;
uint8_t senderMAC[] = {0xB0, 0xCB, 0xD8, 0xC0, 0xF6, 0x5C};
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingDataRaw, int len) {
  const uint8_t *mac = info->src_addr;
  memcpy(&incomingData, incomingDataRaw, sizeof(incomingData));
  Serial.println();
  Serial.print("Roll: ");
  Serial.print(incomingData.roll);
  analogWrite(12, 255);
  analogWrite(13, 255);
  if (incomingData.roll>25 && abs(incomingData.pitch) < 25){
    digitalWrite(14, HIGH);
    digitalWrite(27, LOW);
    digitalWrite(25, HIGH);
    digitalWrite(26, LOW);
  }
  else if (incomingData.roll<-25 && abs(incomingData.pitch < 25)){
    digitalWrite(14, LOW);
    digitalWrite(27, HIGH);
    digitalWrite(25, LOW);
    digitalWrite(26, HIGH);
  }
  else if (incomingData.pitch < -20 && abs(incomingData.roll) < 30){
    analogWrite(12, 176);
    analogWrite(13, 176);
    digitalWrite(14, HIGH);
    digitalWrite(27, LOW);
    digitalWrite(25, LOW);
    digitalWrite(26, HIGH);
  }
  else if (incomingData.pitch > 20 && abs(incomingData.roll) < 30){
    analogWrite(12, 176);
    analogWrite(13, 176);
    digitalWrite(14, LOW);
    digitalWrite(27, HIGH);
    digitalWrite(25, HIGH);
    digitalWrite(26, LOW);
  }
  else{
    digitalWrite(14, LOW);
    digitalWrite(27, LOW);
    digitalWrite(25, LOW);
    digitalWrite(26, LOW);
  }
  Serial.print("  Pitch: ");
  Serial.println(incomingData.pitch);
}
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(27, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(26, OUTPUT);
  pinMode(25, OUTPUT);
}
void loop() {
  esp_now_register_recv_cb(OnDataRecv);
}