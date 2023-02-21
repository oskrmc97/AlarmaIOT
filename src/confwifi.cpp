#include "WiFi.h"
#include "mqtt_client.h"
#include "PubSubClient.h"

class confwifi{
     
    const char* mqttUser = "racso";
    const char* mqttPassword = "bimborico22D";
    
    const char* ssid = "Nasus_p1";
    const char* password = "oscar1234@";

    public:
    void wificonect(){

        WiFi.begin(ssid, password);
        Serial.println("\nconnecting....");

        while(WiFi.status() !=  WL_CONNECTED){
            Serial.println("\nconnecting....");
            delay(1000);
        }
        Serial.println("\n Connected");
        Serial.println("SSID: Nasus_p1 ");
    }

    public:
    void mqttconnect(const char* mqttServer,const int mqttPort){
        WiFiClient espclient;   
        PubSubClient client(espclient);
        client.setServer(mqttServer,mqttPort);
        while(!client.connected()){
            Serial.println("Connecting to MQTT...");
            delay(2000);
            if(client.connect("espprincipal",mqttUser,mqttPassword)){
                Serial.println("connected");
                const char* message = "Hello World i am connected to MQTT";
                int length = strlen(message);
                boolean retained = true;
                client.publish("prueba1",(byte*)message,length,retained);
                Serial.println("mensaje sent");
                delay(3000);
            }
        }
    }

    public:
    void mqtttopic(PubSubClient client){
        client.subscribe("prueba1");
        client.setCallback([](char* topic, byte* payload, unsigned int length) {
            Serial.print("Message arrived in topic: ");
            Serial.println(topic);
            Serial.print("Message:");
            for (int i = 0; i < length; i++) {
                Serial.print((char)payload[i]);
            }
            Serial.println();
            Serial.println("-----------------------");
        });
    }
};

