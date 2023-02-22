#include <Arduino.h>
#include <confwifi.cpp>



confwifi wifi;
WiFiClient espclient;
PubSubClient client(espclient);
int activar = 50;
void callback(char* topic, byte* payload, unsigned int length) {
   activar = (int)payload[0];
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
          client.publish("prueba2",(byte*)message,length,retained);
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
  pinMode(2,OUTPUT);
  wifi.wificonect();
  mqttconnect("broker.mqttdashboard.com",1883, espclient,"racso","bimborico22D");
  client.setCallback(callback);
}

void loop() {
  if(activar == 49){
    Serial.println("Alarma activada");
    digitalWrite(2,HIGH);
    activar = 50;
  }
  if(activar == 48){
    Serial.println("Alarma desactivada");
    digitalWrite(2,LOW);
    activar = 50;
  }
  client.loop();
}