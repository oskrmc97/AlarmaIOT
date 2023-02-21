#include "WiFi.h"
#include "mqtt_client.h"
#include "PubSubClient.h"

class confwifi{
        
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
};

