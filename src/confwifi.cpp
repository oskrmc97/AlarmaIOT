#ifdef ESP32
#include <esp_wifi.h>
#include <WiFi.h>
#include <WiFiClient.h>

#define ESP_getChipId()   ((uint32_t)ESP.getEfuseMac())

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


class confwifi{

    
    public:
    void heartBeatPrint(void){
    static int num = 1;

    if (WiFi.status() == WL_CONNECTED)
        Serial.print("H");        // H means connected to WiFi
    else
        Serial.print("F");        // F means not connected to WiFi

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
    public:
    void check_status()
    {
    static ulong checkstatus_timeout = 0;

    #define HEARTBEAT_INTERVAL    10000L
    // Print hearbeat every HEARTBEAT_INTERVAL (10) seconds.
    if ((millis() > checkstatus_timeout) || (checkstatus_timeout == 0))
    {
        heartBeatPrint();
        checkstatus_timeout = millis() + HEARTBEAT_INTERVAL;
    }
    }

    public:
    void wificonect(){

        // WiFi.begin(ssid, password);
        Serial.println("\nconnecting....");

        while(WiFi.status() !=  WL_CONNECTED){
            Serial.println("\nconnecting....");
            delay(1000);
        }
        Serial.println("\n Connected");
        Serial.println("SSID: Nasus_p1");
    }
};

