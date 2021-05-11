#include "esp32-mqtt.h"
long temperature;
long waterSave;
long waterUse;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  setupCloudIoT();
  randomSeed(analogRead(0));
}

unsigned long lastMillis = 0;
void loop() {
  mqtt->loop();
  delay(10000);  // <- fixes some issues with WiFi stability

  if (!mqttClient->connected()) {
    connect();
    }

  // publish a message roughly every second.
  if (millis() - lastMillis > 15000) {
    lastMillis = millis();
    temperature = random(18, 30);
    waterSave = random(100, 300);
    waterUse = random(230, 500);
    pushData();
    Serial.println("temperature: " + String(temperature) + ", waterSave: " + String(waterSave) + ", WaterUse: " + String(waterUse));
  }
}
