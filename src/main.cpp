#include <Arduino.h>
#include <ESP_WiFiManager.h> 
#include "mqtt_client.h"
#include "PubSubClient.h"

#ifdef ESP32
#include <esp_wifi.h>
#include <WiFi.h>
#include <WiFiClient.h>

// #define ESP_getChipId()   ((uint32_t)ESP.getEfuseMac())

#define LED_ON      HIGH
#define LED_OFF     LOW
#else
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>

#define ESP_getChipId()   (ESP.getChipId())

#define LED_ON      LOW
#define LED_OFF     HIGH
#endif

WiFiClient espclient;
String Router_SSID;
String Router_Pass;
PubSubClient client(espclient);
boolean conexion = false;
boolean AP_mode = false;
static ulong timecontrolprueba =  0;

int activar = 50;
unsigned int mqttstate;

void callback(char* topic, byte* payload, unsigned int length) {
   activar = (int)payload[0];
  }

void heartBeatPrint(void){
  
  static int num = 1;
  if (WiFi.status() == WL_CONNECTED){
    if(mqttstate == 0){
      digitalWrite(16,HIGH);
    }
    digitalWrite(17,LOW);
    Serial.println("H");
      }        // H means connected to WiFi
  else{
        Serial.println("F");
        digitalWrite(16,LOW);
        digitalWrite(17,HIGH);
        }
            // F means not connected to WiFi
  if (num == 80)
  {
      Serial.println();
      num = 1;
  }
  else if (num++ % 10 == 0)
  {
      Serial.print(" ");
  }
  }

void check_status(){

    byte apmode = digitalRead(2);
    static ulong checkstatus_timeout = 0;

    #define HEARTBEAT_INTERVAL    10000L
    // Print hearbeat every HEARTBEAT_INTERVAL (10) seconds.
    if ((millis() > checkstatus_timeout) || (checkstatus_timeout == 0))
    {
        heartBeatPrint();
        checkstatus_timeout = millis() + HEARTBEAT_INTERVAL;
    }
    if(WiFi.status()!= WL_CONNECTED && apmode==1){
      AP_mode = true;
    }
  }

void autoconnectap(){
  ESP_WiFiManager ESP_wifiManager("AutoConnectAP");
  ESP_wifiManager.setDebugOutput(true);
  ESP_wifiManager.setAPStaticIPConfig(IPAddress(192, 168, 0, 120), IPAddress(192, 168, 0, 1), IPAddress(255, 255, 255, 0));
  ESP_wifiManager.setMinimumSignalQuality(-1);
  Router_SSID = ESP_wifiManager.WiFi_SSID();
  Router_Pass = ESP_wifiManager.WiFi_Pass();
  Serial.println("Stored: SSID = " + Router_SSID + ", Pass = " + Router_Pass);

  if (Router_SSID != "")
      {
          ESP_wifiManager.setConfigPortalTimeout(0); //If no access point name has been previously entered disable timeout.
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
  if(AP_mode)
    ESP_wifiManager.resetSettings();
  digitalWrite(18,HIGH);
  if(ESP_wifiManager.autoConnect(AP_SSID.c_str(), AP_PASS.c_str())){ 
      conexion = true;
      Serial.println("WiFi connected la ip es:" + WiFi.localIP().toString());
      digitalWrite(18,LOW);
      }
    else{
      ESP_wifiManager.resetSettings();
      if(ESP_wifiManager.autoConnect(AP_SSID.c_str(), AP_PASS.c_str())){ 
        conexion = true;
        Serial.println("WiFi connected la ip es:" + WiFi.localIP().toString());
        digitalWrite(18,LOW);
      }
    }
    
}


void mqttconnect(const char* mqttServer,const int mqttPort, WiFiClient espclient, const char* mqttUser,const char* mqttPassword){  
  client.setServer(mqttServer,mqttPort);
  unsigned long checkstatus_timeout = 0;
  unsigned long checkstatus_time = 0;
  
  #define MQTT_INTERVAL    1000L
  while(!client.connected()){
      if((millis() > checkstatus_timeout)){
        Serial.println("Connecting to MQTT...");
         if(client.connect("espprincipal",mqttUser,mqttPassword)){
          Serial.println("connected");
          const char* message = "Hello World i am connected to MQTT";
          int length = strlen(message);
          boolean retained = true;
          client.publish("oscarmelo/prueba2",(byte*)message,length,retained);
          Serial.println("mensaje sent");
          if(client.subscribe("prueba1")){
            Serial.println("Suscribed to topic!");
            mqttstate = client.state();
          }  
          else{
            ulong timecontrol = 0;
            if(millis()> timecontrol){
              Serial.println("Error to subscribe!");
              timecontrol = millis() + 2000;}
          }
      }
      else{
          ulong timecontrol = 0;
            if(millis()> timecontrol){
              // Serial.println("Error connecting to MQTT");
              // mqttstate = client.state(); 
             }
      } 
        checkstatus_timeout = millis() + MQTT_INTERVAL;
      }

      if(millis() > checkstatus_time ){
        digitalWrite(16, HIGH);
        checkstatus_time = millis() + MQTT_INTERVAL;
        Serial.println(checkstatus_time/1000);
      }
      if(millis() > checkstatus_time){
        digitalWrite(16, LOW);
        checkstatus_time = millis() + MQTT_INTERVAL;
        Serial.println(checkstatus_time);
      }


  }
}
void encapsuladas(){
  autoconnectap();
  if(conexion){
    mqttconnect("broker.mqttdashboard.com",1883, espclient,"racso","bimborico22D");
    client.setCallback(callback);
  }
}
  
void setup() {
  Serial.begin(115200);
  pinMode(2,INPUT);
  pinMode(4,OUTPUT);
  pinMode(16,OUTPUT);
  pinMode(17,OUTPUT);
  pinMode(18,OUTPUT);
  Serial.println("\nStarting AutoConnectAP");
  encapsuladas();
}

void loop() {

  if(activar == 49){
    Serial.println("Alarma activada");
    digitalWrite(4,HIGH);
    activar = 50;
  }
  if(activar == 48){
    Serial.println("Alarma desactivada");
    digitalWrite(4,LOW);
    activar = 50;
  }
  check_status();
  if(AP_mode){
    encapsuladas();
    AP_mode = false;
    }
  client.loop();
}