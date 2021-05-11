#include "esp32-mqtt.h"
String device_name;
double temp_shower;
double temp_hot_shower;
double temp_out_shower;
double temp_pump;
double flux_hot_pump;
double flux_out_pump;
double flux_pump;
double battery_shower;
int state_shower;
int state_pump;
double latitude;
double longitude;

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

  // publish a message roughly every 15 second.
  if (millis() - lastMillis > 15000) {
    lastMillis = millis();
    device_name = "HSC001";
    temp_shower = random(0, 50);
    temp_hot_shower = random(0, 50);
    temp_out_shower = random(0, 50);
    temp_pump = random(0, 50);
    flux_hot_pump = random(0, 500);
    flux_out_pump = random(0, 500);
    flux_pump = random(0, 500);
    battery_shower = random(0, 100);
    state_shower = random(0, 1);
    state_pump = random(0, 1);
    latitude = random(0, 250);
    longitude = random(250, 500);

    pushData();
    Serial.println("Publishing Data!...");
    Serial.println("tempShower: " + (String)temp_shower);
  }
}
