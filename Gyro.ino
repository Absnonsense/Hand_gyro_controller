#include <Wire.h>
#include <esp_now.h>
#include <WiFi.h>
typedef struct struct_message {
  float roll;
  float pitch;
} struct_message;
struct_message angleData;
uint8_t receiverMAC[] = {0xE0, 0x8C, 0xFE, 0x62, 0x22, 0x30};
esp_now_peer_info_t peerInfo;
float RateRoll, RatePitch, RateYaw;
float RateCalibrationRoll = 0, RateCalibrationPitch = 0, RateCalibrationYaw = 0;
float AccX, AccY, AccZ;
float AccAngleRoll, AccAnglePitch;
float AngleRoll = 0;
float AnglePitch = 0;
uint32_t prevTime;
void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000);
  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.beginTransmission(0x68);
  Wire.write(0x1A); Wire.write(0x03);
  Wire.endTransmission();
  Wire.beginTransmission(0x68);
  Wire.write(0x1B); Wire.write(0x08);
  Wire.endTransmission();
  Wire.beginTransmission(0x68);
  Wire.write(0x1C); Wire.write(0x10);
  Wire.endTransmission();
  delay(500);
  for (int i = 0; i < 2000; i++) {
    readIMU();
    RateCalibrationRoll += RateRoll;
    RateCalibrationPitch += RatePitch;
    RateCalibrationYaw += RateYaw;
    delay(1);
  }
  RateCalibrationRoll /= 2000;
  RateCalibrationPitch /= 2000;
  RateCalibrationYaw /= 2000;
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
  prevTime = micros();
}
void readIMU() {
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 6);
  int16_t AccXLSB = Wire.read() << 8 | Wire.read();
  int16_t AccYLSB = Wire.read() << 8 | Wire.read();
  int16_t AccZLSB = Wire.read() << 8 | Wire.read();
  Wire.beginTransmission(0x68);
  Wire.write(0x43);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 6);
  int16_t GyroX = Wire.read() << 8 | Wire.read();
  int16_t GyroY = Wire.read() << 8 | Wire.read();
  int16_t GyroZ = Wire.read() << 8 | Wire.read();
  RateRoll  = (float)GyroX / 65.5;
  RatePitch = (float)GyroY / 65.5;
  RateYaw   = (float)GyroZ / 65.5;
  AccX = (float)AccXLSB / 4096;
  AccY = (float)AccYLSB / 4096;
  AccZ = (float)AccZLSB / 4096;
  AccAngleRoll  = atan(AccY / sqrt(AccX*AccX + AccZ*AccZ)) * 57.2958;
  AccAnglePitch = -atan(AccX / sqrt(AccY*AccY + AccZ*AccZ)) * 57.2958;
}
void loop() {
  uint32_t now = micros();
  float dt = (now - prevTime) / 1000000.0;
  prevTime = now;
  readIMU();
  RateRoll  -= RateCalibrationRoll;
  RatePitch -= RateCalibrationPitch;
  RateYaw   -= RateCalibrationYaw;
  AngleRoll  = 0.98 * (AngleRoll + RateRoll * dt) + 0.02 * AccAngleRoll;
  AnglePitch = 0.98 * (AnglePitch + RatePitch * dt) + 0.02 * AccAnglePitch;
  static int counter = 0;
  if (++counter % 10 == 0) {
    Serial.print("Roll: ");
    Serial.print(AngleRoll);
    Serial.print(" Pitch: ");
    Serial.println(AnglePitch);
  }
  static uint32_t lastSend = 0;
  if (millis() - lastSend > 10) {
    angleData.roll = AngleRoll;
    angleData.pitch = AnglePitch;
    esp_err_t result = esp_now_send(receiverMAC, (uint8_t *) &angleData, sizeof(angleData));
    if (result != ESP_OK) {
      Serial.println("Send error");
    }
    lastSend = millis();
  }
}