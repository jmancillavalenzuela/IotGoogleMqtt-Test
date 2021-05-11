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
  object["_id"]         = chipId ;
  object["device_name"]         = "HSC001";
  object["temp_shower"]          = "35.4";
  object["temp_hot_shower"]          = "15.4";
  object["temp_out_shower"]          = "15.4";
  object["temp_pump"]          = "35.4";
  object["flux_hot_pump"]          = "15.4";
  object["flux_out_pump"]          = "15.4";
  object["flux_pump"]          = "15.4";
  object["battery_shower"]          = "35.4";
  object["state_shower"]          = "35.4";
  object["state_pump"]          = "35.4";
  object["latitude"]          = "35.4";
  object["longitude"]          = "35.4";

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

void pushData() {
  DynamicJsonDocument data(198);
  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  data["_id"]         = chipId ;
  data["device_name"]         = "HSC001";
  data["temp_shower"]          = "35.4";
  data["temp_hot_shower"]          = "15.4";
  data["temp_out_shower"]          = "15.4";
  data["temp_pump"]          = "35.4";
  data["flux_hot_pump"]          = "15.4";
  data["flux_out_pump"]          = "15.4";
  data["flux_pump"]          = "15.4";
  data["battery_shower"]          = "35.4";
  data["state_shower"]          = "35.4";
  data["state_pump"]          = "35.4";
  data["latitude"]          = "35.4";
  data["longitude"]          = "35.4";
  String json_object;
  Serial.println(json_object);
  serializeJsonPretty(data, json_object);
  publishTelemetry("/iotcore-topic", json_object);
  Serial.println("Datos Pusheados");
}

void pushData(double temp_shower, double temp_hot_shower, double temp_out_shower, double temp_pump, double flux_hot_pump, double flux_out_pump, double flux_pump, double battery_shower, double state_shower, double pump_shower, double latitude, double longitude) {
  DynamicJsonDocument data(198);
    uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  data["_id"]         = chipId ;
  data["device_name"]         = "HSC001";
  data["temp_shower"]          = temp_shower;
  data["temp_hot_shower"]          = "15.4";
  data["temp_out_shower"]          = "15.4";
  data["temp_pump"]          = "35.4";
  data["flux_hot_pump"]          = "15.4";
  data["flux_out_pump"]          = "15.4";
  data["flux_pump"]          = "15.4";
  data["battery_shower"]          = "35.4";
  data["state_shower"]          = "35.4";
  data["state_pump"]          = "35.4";
  data["latitude"]          = "35.4";
  data["longitude"]          = "35.4";
  String json_object;
  Serial.println(json_object);
  serializeJsonPretty(data, json_object);
  publishTelemetry("/iotcore-topic", json_object);
  Serial.println("Datos Pusheados");
}
bool publishState(String data) {
  return mqtt->publishState(data);
}

void connect() {
  connectWifi();
  mqtt->mqttConnect();
}

void setupCloudIoT() {
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
