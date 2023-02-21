#include <Arduino.h>
#include <confwifi.cpp>



confwifi wifi;
WiFiClient espclient;
PubSubClient client(espclient);


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
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
  wifi.wificonect();
  mqttconnect("broker.mqttdashboard.com",1883, espclient,"racso","bimborico22D");
  client.setCallback(callback);
}

void loop() {
  client.loop();
}