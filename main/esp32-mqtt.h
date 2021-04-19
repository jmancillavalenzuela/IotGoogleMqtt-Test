#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <MQTT.h>

#include <CloudIoTCoreMqtt.h>
#include <CloudIoTCore.h>
#include "ciotc_config.h"

#include <Arduino.h>
#include <ArduinoJson.h>

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
}

// Initialize WiFi and MQTT for this board
WiFiClientSecure *netClient;
CloudIoTCoreDevice *device;
CloudIoTCoreMqtt *mqtt;
MQTTClient *mqttClient;
unsigned long iat = 0;
String jwt;

///////////////////////////////
// Helpers specific to this board
///////////////////////////////
String getDefaultSensor() {
  return "Wifi: " + String(WiFi.RSSI()) + "db";
}

String getDeviceState() {
  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  Serial.printf("ESP32 Chip model = %s Rev %d\n", ESP.getSdkVersion(), ESP.getChipRevision());
  Serial.printf("This chip has %d cores\n", ESP.getFlashChipSpeed());
  Serial.print("Chip ID: "); Serial.println(chipId);

  const int capacity = JSON_OBJECT_SIZE(5);
  StaticJsonDocument<capacity> edin_json_config_doc;
  // create an object
  JsonObject object = edin_json_config_doc.to<JsonObject>();

  char buf[16];
  sprintf(buf, "IP:%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3] );
  Serial.println(String(buf));
  object["chipID"]         = chipId ;
  object["IpNodo"]         = String (buf);
  object["State"]          = "ON";

  String output;
  serializeJson(object, output);                                                                  //SAve CPU cycles by calculatinf the size.
  Serial.println(F("publishing device manageTopic metadata:"));
  Serial.println(output);
  return output;
}

String getJwt() {
  iat = time(nullptr);
  Serial.println("Refreshing JWT");
  jwt = device->createJWT(iat, jwt_exp_secs);
  return jwt;
}

void setupWifi() {
  Serial.println("Starting wifi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(10000);
  }

  configTime(0, 0, ntp_primary, ntp_secondary);
  Serial.println("Waiting on time sync...");
  while (time(nullptr) < 1510644967) {
    delay(10000);
  }
}

void connectWifi() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(10000);
  }
}

///////////////////////////////
// Orchestrates various methods from preceeding code.
///////////////////////////////
bool publishTelemetry(String data) {
  return mqtt->publishTelemetry(data);
}

bool publishTelemetry(const char *data, int length) {
  return mqtt->publishTelemetry(data, length);
}

bool publishTelemetry(String subfolder, String data) {
  return mqtt->publishTelemetry(subfolder, data);
}

bool publishTelemetry(String subfolder, const char *data, int length) {
  return mqtt->publishTelemetry(subfolder, data, length);
}

void pushData(int temperature, int waterSave, int waterUse) {
  Serial.println("Iniciando Push de Datos: ");
  DynamicJsonDocument data(198);
  //    data["temperature"] = String(temperature);
  //    data["waterUse"] = String(waterUse);
  //    data["waterSave"] = String(waterSave);
  data["data"] = String(waterSave);
  String json_str;
  Serial.println(json_str);
  serializeJsonPretty(data, json_str);
  Serial.println("JsonData Created.");
  publishTelemetry("/iotcore-topic", json_str);
  Serial.println("Mensaje Publicado.");
  Serial.println(" :Fin Push de Datos");
}
bool publishState(String data) {
  return mqtt->publishState(data);
}

void connect() {
  connectWifi();
  mqtt->mqttConnect();
}

void setupCloudIoT(){
  device = new CloudIoTCoreDevice(
      project_id, location, registry_id, device_id,
      private_key_str);
    setupWifi();
    netClient = new WiFiClientSecure();
    netClient->setCACert(root_cert);
    mqttClient = new MQTTClient(512);
    mqttClient->setOptions(180, true, 1000); // keepAlive, cleanSession, timeout
    mqtt = new CloudIoTCoreMqtt(mqttClient, netClient, device);
    //mqtt->setLogConnect(true);
    mqtt->setUseLts(true);
    mqtt->startMQTT();
}
