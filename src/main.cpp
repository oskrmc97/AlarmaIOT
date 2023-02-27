#include <Arduino.h>
#include <confwifi.cpp>
#include <ESP_WiFiManager.h> 
#include "mqtt_client.h"
#include "PubSubClient.h"



confwifi wifi;
WiFiClient espclient;
String Router_SSID;
String Router_Pass;
PubSubClient client(espclient);

int activar = 50;
void callback(char* topic, byte* payload, unsigned int length) {
   activar = (int)payload[0];
  }

void autoconnectap(){
  ESP_WiFiManager ESP_wifiManager("AutoConnectAP");
  ESP_wifiManager.setDebugOutput(true);
  ESP_wifiManager.setAPStaticIPConfig(IPAddress(192, 168, 0, 120), IPAddress(192, 168, 0, 1), IPAddress(255, 255, 255, 0));
  ESP_wifiManager.setMinimumSignalQuality(-1);
  // ESP_wifiManager.setSTAStaticIPConfig(IPAddress(192, 168, 2, 114), IPAddress(192, 168, 2, 1), IPAddress(255, 255, 255, 0),IPAddress(192, 168, 2, 1), IPAddress(8, 8, 8, 8));
  Router_SSID = ESP_wifiManager.WiFi_SSID();
  Router_Pass = ESP_wifiManager.WiFi_Pass();
  Serial.println("Stored: SSID = " + Router_SSID + ", Pass = " + Router_Pass);

  if (Router_SSID != "")
      {
          ESP_wifiManager.setConfigPortalTimeout(300); //If no access point name has been previously entered disable timeout.
          Serial.println("Got stored Credentials. Timeout 60s");
      }
      else
      {
          Serial.println("No stored Credentials. No timeout");
      }

  String chipID = String(ESP_getChipId(), HEX);
  chipID.toUpperCase();
  String AP_SSID = "Alarma" + chipID;
  String AP_PASS = "ESP_" + chipID;
  ESP_wifiManager.resetSettings();
  ESP_wifiManager.autoConnect(AP_SSID.c_str(), AP_PASS.c_str());
  Serial.println("WiFi connected la ip es:" + WiFi.localIP().toString());
}


void mqttconnect(const char* mqttServer,const int mqttPort, WiFiClient espclient, const char* mqttUser,const char* mqttPassword){  
  client.setServer(mqttServer,mqttPort);
  while(!client.connected()){
      Serial.println("Connecting to MQTT...");
      delay(2000);
      if(client.connect("espprincipal",mqttUser,mqttPassword)){
          Serial.println("connected");
          const char* message = "Hello World i am connected to MQTT";
          int length = strlen(message);
          boolean retained = true;
          client.publish("oscarmelo/prueba2",(byte*)message,length,retained);
          Serial.println("mensaje sent");
          if(client.subscribe("prueba1"))
              Serial.println("Suscribed to topic!");
          else
              Serial.println("Error to subscribe!");
          delay(2000);
      }
      else{
          Serial.println("Error connecting to MQTT");
          Serial.println(client.state());
          delay(2000);
      }
  }
}


void setup() {
  Serial.begin(115200);
  pinMode(2,INPUT);
  pinMode(3,OUTPUT);
  Serial.println("\nStarting AutoConnectAP");
  autoconnectap();
  // wifi.wificonect();
  mqttconnect("broker.mqttdashboard.com",1883, espclient,"racso","bimborico22D");
  client.setCallback(callback);
}

void loop() {
  if(activar == 49){
    Serial.println("Alarma activada");
    digitalWrite(3,HIGH);
    activar = 50;
  }
  if(activar == 48){
    Serial.println("Alarma desactivada");
    digitalWrite(3,LOW);
    activar = 50;
  }
  client.loop();
  wifi.check_status();
}