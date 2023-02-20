#include "WiFi.h"
#include "mqtt_client.h"
#include "PubSubClient.h"

class confwifi{

    const char* mqttServer = "broker.mqttdashboard.com";
    const int mqttPort = 1883;
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
        PubSubClient mqttconexion(){
            
            WiFiClient espclient;
            PubSubClient client(espclient);

            client.setServer(mqttServer,mqttPort);
            while(!client.connected()){
                Serial.println("Connecting to MQTT...");
                if(client.connect("espprincipal",mqttUser,mqttPassword)){
                    Serial.println("connected");
                }
                else{
                    Serial.println("failed to connect");
                    Serial.println(client.state());
                    delay(2000);
                }
            }
            return client;
        }
};

