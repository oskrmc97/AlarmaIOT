#include <Arduino.h>
#include <confwifi.cpp>

confwifi wifi;
void setup() {
  Serial.begin(115200);
  wifi.wificonect();
  wifi.mqttconexion();
  Serial.println(wifi.mqttconexion().state());
}

void loop() {
}