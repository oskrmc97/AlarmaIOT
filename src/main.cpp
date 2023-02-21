#include <Arduino.h>
#include <confwifi.cpp>


confwifi wifi;
void setup() {
  Serial.begin(115200);
  wifi.wificonect();
  wifi.mqttconnect("broker.mqttdashboard.com",1883);
}
void loop() {
}