#include <Arduino.h>
#include <ESP_WiFiManager.h> 
#include "mqtt_client.h"
#include "PubSubClient.h"
#include <esp_task_wdt.h>
#ifdef ESP32
#include <esp_wifi.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiMulti.h>
#include <WiFiClient.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// #define ESP_getChipId()   ((uint32_t)ESP.getEfuseMac())
#define BOTtoken "5909477841:AAGeXd50ajjAVVPjPJw7hN27IxZAiQi6bpo"  // your Bot Token (Get from Botfather)
#define CHAT_ID "-1001595822537"
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
WiFiClientSecure espclient_telegram;
WiFiMulti wifimulti;
String Router_SSID;
String Router_Pass;
PubSubClient client(espclient);
UniversalTelegramBot bot(BOTtoken, espclient_telegram);
boolean conexion = false;
boolean AP_mode = false;
boolean reset_esp = false;
boolean policia = true;
static ulong timecontrolprueba =  0;

int activar = 50;
int i = 0;
unsigned int mqttstate;


int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

  void handleNewMessages(int numNewMessages) {
    Serial.println("handleNewMessages");
    Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
      String chat_id = String(bot.messages[i].chat_id);
      if (chat_id != CHAT_ID){
        bot.sendMessage(chat_id, "Unauthorized user", "");
        Serial.println("El chat id es:"+chat_id);
        continue;
      }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/comando") {
      String welcome = "La comunidad necesita de ti, " + from_name + ".\n";
      welcome += "Este es un sistema de alarma comunitaria con aviso directo a las autoridades.\n\n";
      welcome += "/Policia_en_camino para detener mensajes y acudir a la emergencia \n";
      welcome += "/Apagar para detener alarma \n";
      welcome += "/Estado para detectar el estado de la alarma \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/Policia_en_camino@Caicedo_15a_bot") {
      bot.sendMessage(chat_id, "Autoridades fueron notifcadas", "");
      policia = false;
    }
    
    if (text == "/Apagar@Caicedo_15a_bot") {
      bot.sendMessage(chat_id, "Alarma desactivada", "");
      activar = 48;
    }
    if (text == "/Estado@Caicedo_15a_bot") {
      if(activar == 49 || activar == 51)
        bot.sendMessage(chat_id, "Comunidad activo alarma!!!", "");
       if(activar == 48 || activar == 50)
        bot.sendMessage(chat_id, "Alarma desactivada", "");
    }
  }
}


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
        digitalWrite(17,HIGH);
        digitalWrite(16,LOW);
        if(wifimulti.run() == WL_CONNECTED){
          Serial.println("me conecte a otra red papu");
          Serial.print(WiFi.SSID());
          reset_esp = true;
        }
        digitalWrite(17,LOW);
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

    #define HEARTBEAT_INTERVAL    1000L
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
  if(AP_mode){
     ESP_wifiManager.resetSettings();
     esp_task_wdt_init(240, true);
  }
  digitalWrite(18,HIGH);
  if(ESP_wifiManager.autoConnect(AP_SSID.c_str(), AP_PASS.c_str())){
      conexion = true;
      Serial.println("WiFi connected la ip es:" + WiFi.localIP().toString());
      digitalWrite(18,LOW);
      }
}


void mqttconnect(const char* mqttServer,const int mqttPort, WiFiClient espclient, const char* mqttUser,const char* mqttPassword){  
  client.setServer(mqttServer,mqttPort);
  unsigned long checkstatus_timeout = 0;
  unsigned long checkstatus_time = 0;
  
  #define MQTT_INTERVAL    1000L
  #define MQTT_INTERVAL_LED    1000L
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
        Serial.println("Resetting WDT...");
        esp_task_wdt_reset();
      }

      if(WiFi.status() != WL_CONNECTED){
          while(true){
            digitalWrite(17,HIGH);
          }
        }
        if(WiFi.status() == WL_CONNECTED){
          digitalWrite(17,LOW);
        }

      if(millis() > checkstatus_time ){
        digitalWrite(16, HIGH);
        checkstatus_time = millis()+(MQTT_INTERVAL_LED/4);
        Serial.println(checkstatus_time/1000);
      }
      if(millis() > checkstatus_time){
        digitalWrite(16, LOW);
        checkstatus_time = millis() + MQTT_INTERVAL_LED*2;
        Serial.println(checkstatus_time);
      }
  }
}
void encapsuladas(){
  autoconnectap();
  if(conexion){
    esp_task_wdt_init(20, true);
    mqttconnect("broker.mqttdashboard.com",1883, espclient,"racso","bimborico22D");
    client.setCallback(callback);
  }
}
  
void activacion(int i){
   if(activar == 49){
    Serial.println("Alarma activada");
    digitalWrite(4,HIGH);
    activar = 51;
  }
  if(activar == 48){
    Serial.println("Alarma desactivada");
    digitalWrite(4,LOW);
    activar = 50;
    i = 0;
  }
}
void actualizarbot(){
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  while(numNewMessages) {
    Serial.println("msm recibido");
    handleNewMessages(numNewMessages);
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
    lastTimeBotRan = millis();
  }
}
void setup() {
  wifimulti.addAP("Nasus_p1","oscar1234@");
  wifimulti.addAP("NASUS","gvp3165504228");
  wifimulti.addAP("alarma","123456789");
  espclient_telegram.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  esp_task_wdt_add(NULL);
  Serial.begin(115200);
  pinMode(2,INPUT);
  pinMode(4,OUTPUT);
  pinMode(16,OUTPUT);
  pinMode(17,OUTPUT);
  pinMode(18,OUTPUT);
  Serial.println("\nStarting AutoConnectAP");
  esp_task_wdt_init(240, true);
  encapsuladas();
}

int last = millis();

void loop() {

  activacion(i);
  check_status();
  if(AP_mode){
    encapsuladas();
    AP_mode = false;
    }
  client.loop();
  actualizarbot();

   if((activar == 49 || activar == 51) && policia == true){
        bot.sendMessage(CHAT_ID, "Comunidad activo alarma!!!", "");
      }

  if (millis() - last >= 2000 && reset_esp == false) {
      Serial.println("Resetting WDT...");
      esp_task_wdt_reset();
      last = millis();
      if (reset_esp == true) {
        Serial.println("Stopping WDT reset. CPU should reboot in 3s");
        reset_esp == false;
      }
  }


}