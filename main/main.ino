#include "esp32-mqtt.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  setupCloudIoT();
}

unsigned long lastMillis = 0;
void loop() {
  mqtt->loop();
  delay(10000);  // <- fixes some issues with WiFi stability

  if (!mqttClient->connected()) {
    connect();
    }

  // publish a message roughly every second.
  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    pushData(1,2,3);
    Serial.println("datos enviados");
  }
}
