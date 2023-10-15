#include <Arduino.h> //necesario para el funcionamiento en vsc
#include <ESP_WiFiManager.h>  //libreria para la conexion wifi Auotoconect aP en este caso
#include "mqtt_client.h" // conexion al broker
#include "PubSubClient.h" // publicar en el broker y suscribir en el broker
#include <esp_task_wdt.h> // WatchDog
#ifdef ESP32 // se define la esp a usar segun eso las librerias que se utilizan
#include <esp_wifi.h> // conexion wifi, con esto se crea un cliente wifi (No usada)
#include <WiFi.h> // igual al anterior
#include <WiFiClientSecure.h> // necesario para telegram
#include <WiFiMulti.h> // sirve para conectar a multiples APS
#include <WiFiClient.h> // cliente wifi
#include <UniversalTelegramBot.h> // bot de telegram
#include <ArduinoJson.h> // telegram 

// #define ESP_getChipId()   ((uint32_t)ESP.getEfuseMac())
#define BOTtoken "5909477841:AAGeXd50ajjAVVPjPJw7hN27IxZAiQi6bpo"  // your Bot Token (Get from Botfather)
#define CHAT_ID "-1001595822537" //id grupo de telegram
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

const int boton = 2;
const int senal = 4;
const int led_verde = 16;
const int led_rojo = 17;
const int led_azul = 18;


int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

  void handleNewMessages(int numNewMessages) { //funcion que espera mensajes de telegram
    Serial.println("handleNewMessages"); 
    Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) { // recibe el numero de mensajes, es necesario, auqnue sea 1
    // Chat id of the requester
      String chat_id = String(bot.messages[i].chat_id); //guarda el identificador del chat en formato string
      if (chat_id != CHAT_ID){ // comparamos los identificadores
        bot.sendMessage(chat_id, "Unauthorized user", ""); // funcion para mandar un mensaje al chat identificado
        Serial.println("El chat id es:"+chat_id);
        continue; //Al no ser autorizado, sale del ciclo "for" por ende no ejecuta el codigo restante
      }
    
    // Print the received message
    String text = bot.messages[i].text; //obtiene el texto del chat iterando la cantidad de mensajes
    Serial.println(text);

    String from_name = bot.messages[i].from_name; // obtenemos el nombre del usuario de telegram (Nikname)

    if (text == "/comando") { // nos muestra los comandos a utilizar en el chat de telegram
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


void callback(char* topic, byte* payload, unsigned int length) { // recibe los mensajes del broker y el topico suscrito
   activar = (int)payload[0]; // convierte los bits en enteros, con lo cual cada caracter es un numero entero, obtiene solo el primero
   Serial.println("Mensaje "+activar); 
   Serial.println(topic);
   bot.sendMessage(CHAT_ID, topic, ""); // Evnaimos a telegram, el topico de el cual se recibio el mensaje
  }

void heartBeatPrint(void){ //revisa la conexion de el Wifi
  
  static int num = 1;
  if (WiFi.status() == WL_CONNECTED){
    if(mqttstate == 0){ // es el estado de la conexion mqtt 0 es correcta
      digitalWrite(led_verde,HIGH); //Si esta conectado a wifi y conectado al broker el led encendera, verde
    }
    digitalWrite(led_rojo,LOW);
    Serial.println("H");
      }        // H means connected to WiFi
  else{ 
        Serial.println("F");
        digitalWrite(led_rojo,HIGH);
        digitalWrite(led_verde,LOW);
        if(wifimulti.run() == WL_CONNECTED){ // si se cae la red actual, intenta conectar con las redes establecidas en el multiWifi
          Serial.println("me conecte a otra red papu");
          Serial.print(WiFi.SSID());
        }
        digitalWrite(led_rojo,LOW);
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

void check_status(){ //funcion que va en el loop para verificar la conexion

    byte apmode = digitalRead(boton); // lee la entrada digital y guarda su valor 1 o 0
    static ulong checkstatus_timeout = 0; // tiempo donde se compara el valor de millis()

    #define HEARTBEAT_INTERVAL    1000L // El "Delay" para aumentar a la funcion millis()
    // Print hearbeat every HEARTBEAT_INTERVAL (10) seconds.
    if ((millis() > checkstatus_timeout) || (checkstatus_timeout == 0))
    {
        heartBeatPrint(); //llama a la funcion de comprobacion del wifi
        checkstatus_timeout = millis() + HEARTBEAT_INTERVAL; // vuelve a entrar cuando millis sea mayor a 1seg
    }
    if(WiFi.status()!= WL_CONNECTED && apmode==1){ // Si no encuentra ninguna red permite evaluar el estado del boton
      AP_mode = true; // activa el modo AP, al reinicio del esp, entrara en este modo
        if(wifimulti.run() == WL_CONNECTED){ // Busca una red a la cual conectarse
          Serial.println("me conecte a otra red papu");
          Serial.print(WiFi.SSID());
        }
    }
  }

void autoconnectap(){
  ESP_WiFiManager ESP_wifiManager("AutoConnectAP");
  ESP_wifiManager.setDebugOutput(true);
  ESP_wifiManager.setAPStaticIPConfig(IPAddress(192, 168, 0, 120), IPAddress(192, 168, 0, 1), IPAddress(255, 255, 255, 0)); // Crea la configuracion para la red AP
  ESP_wifiManager.setMinimumSignalQuality(-1); // la minima n permitida
  Router_SSID = ESP_wifiManager.WiFi_SSID(); // Obtiene la ultima red coneectada eeprom
  Router_Pass = ESP_wifiManager.WiFi_Pass(); //obtiene la contraseña de la ultima red conectada, Eeprom
  Serial.println("Stored: SSID = " + Router_SSID + ", Pass = " + Router_Pass);

  if (Router_SSID != "") //existen credenciales y sigue la conexion
      {
          ESP_wifiManager.setConfigPortalTimeout(0); //If no access point name has been previously entered disable timeout.
          Serial.println("Got stored Credentials. Timeout 60s");
      }
      else
      {
          Serial.println("No stored Credentials. No timeout");
          if(wifimulti.run() == WL_CONNECTED){ //No encuentra ninguna conexion anterior por lo tanto accede por medio de las guardadas en multiwifi, este es un machete a corregir
            Serial.println("me conecte a otra red papu");
            Serial.print(WiFi.SSID());
        }
          
      }
  String chipID = String(ESP_getChipId(), HEX); //Obitene el id de la ESP
  chipID.toUpperCase();
  String AP_SSID = "Alarma" + chipID; //El nombre de la red AP
  String AP_PASS = "ESP_" + chipID; //Pass de la red AP
  if(AP_mode){ // No esta en este modo a menos que se especifique dadas las condiciones
     ESP_wifiManager.resetSettings(); // Resetea toda la informacion de las conexiones de la eeprom
     esp_task_wdt_init(240, true); // establece el watchdog en 4 min para asi tener tiempo de conectarse
  }
  digitalWrite(led_azul,HIGH);
  if(ESP_wifiManager.autoConnect(AP_SSID.c_str(), AP_PASS.c_str())){ // intenta conectarse con las credenciales guardadas en la eprom
      conexion = true; // la conexion fue correcta, sigue en el codigo
      Serial.println("WiFi connected la ip es:" + WiFi.localIP().toString());
      digitalWrite(led_azul,LOW); // Sale del modo AP
      }
}


void mqttconnect(const char* mqttServer,const int mqttPort, WiFiClient espclient, const char* mqttUser,const char* mqttPassword){  //funcion de mqtt
  client.setServer(mqttServer,mqttPort);
  unsigned long checkstatus_timeout = 0; //Timers para evitar delays
  unsigned long checkstatus_time = 0;
  
  #define MQTT_INTERVAL    1000L // el valor de la espera
  #define MQTT_INTERVAL_LED    1000L
  while(!client.connected()){ // el mqtt no esta conectado, se queda en este ciclo
      if((millis() > checkstatus_timeout)){ // entra al "Delay"
        Serial.println("Connecting to MQTT...");
         if(client.connect("espprincipal",mqttUser,mqttPassword)){ // la conexion al broker es correcta
          Serial.println("connected");
          const char* message = "Hello World i am connected to MQTT"; // guarda un mensaje en char*
          int length = strlen(message); // obtiene la longitud del mensaje
          boolean retained = true; // retiene el mensaje hasta ser enviado
          client.publish("oscarmelo/prueba",(byte*)message,length,retained); // publica el mensaje en el topic establecido, convirtiendo el mensaje en byte payload[]
          Serial.println("mensaje sent");
          if(client.subscribe("+/AlarmaComunitaria")){ // nos suscribimos a un topico, en este caso a cualquiera que tenga como topic final /alarmaComonitaria
            Serial.println("Suscribed to topic!");
            mqttstate = client.state(); // obtiene el estado de la conexion, si es 0, fue correcta
          }  
          else{ //evia mensaje de error al suscribirse al topico, de manera serial
            ulong timecontrol = 0;
            if(millis()> timecontrol){
              Serial.println("Error to subscribe!");
              timecontrol = millis() + 2000;}
          }
      }
      else{ // Error en la conexion mqtt
          ulong timecontrol = 0;
            if(millis()> timecontrol){
              // Serial.println("Error connecting to MQTT");
              // mqttstate = client.state(); 
             }
      } 
        checkstatus_timeout = millis() + MQTT_INTERVAL; // un seg de espera
        Serial.println("Resetting WDT...");
        esp_task_wdt_reset();  //resetea el watchdog para evitar que se reincie, cada seg
      }

      if(WiFi.status() != WL_CONNECTED){
          while(true){
            digitalWrite(led_rojo,HIGH); // si no hay conexion en el intento de la conexion mqtt, no sale del ciclo y la esp se reiniciara.
          }
        }
        if(WiFi.status() == WL_CONNECTED){
          digitalWrite(led_rojo,LOW);
        }

      if(millis() > checkstatus_time ){ // parpadea el led verde, en caso de haber wifi y no poder conectar con el broker
        digitalWrite(led_verde, HIGH);
        checkstatus_time = millis()+(MQTT_INTERVAL_LED/4);
        Serial.println(checkstatus_time/1000);
      }
      if(millis() > checkstatus_time){
        digitalWrite(led_verde, LOW);
        checkstatus_time = millis() + MQTT_INTERVAL_LED*2;
        Serial.println(checkstatus_time);
      }
  }
}
void encapsuladas(){ // funcion que trae las funciones principales conexion wifi y broker
  autoconnectap(); // llama a la conexion a internet
  if(conexion){ // si la conexion a internet fue exitosa, intenta la conexion con el broker
    esp_task_wdt_init(20, true); // si todo fue correcto, define el watchdogs en 20 seg. sacandolo de los 4 min establecidos anteriormente
    mqttconnect("140.238.178.88",1883, espclient,"racso","123456"); // conexion al broker
    // mqttconnect("broker.mqttdashboard.com",1883, espclient,"racso","bimborico22D");
    client.setCallback(callback); // funcion para escuchar los datos del broker
  }
}
  
void activacion(int i){
   if(activar == 49){ // es el valor en enteros del primer caracter del payload
    Serial.println("Alarma activada");
    digitalWrite(senal,HIGH);
    activar = 51;
  }
  if(activar == 48){ // es el valor de 0 en el primer caracter del payload
    Serial.println("Alarma desactivada");
    digitalWrite(senal,LOW);
    activar = 50;
    i = 0;
  }
}
void actualizarbot(){ // Funcion de escucha al bot de telegram
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  while(numNewMessages) {
    Serial.println("msm recibido");
    handleNewMessages(numNewMessages); // envia el numero de mensajes a la funcion de telehgram
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
    lastTimeBotRan = millis();
  }
}

void HandleMqtt() //revisa la conexion del broker constantemente
{
   if (!client.connected()) // si se descoencta
   {
    if(conexion){ // y hay wifi
      mqttconnect("140.238.178.88",1883, espclient,"",""); // intenta nuevamnete la conexion 
      client.setCallback(callback); // vuelve a escuchar al broker
  }
   }
    client.loop(); // llama al cliente mqtt de manera periodica
}
void setup() {
  wifimulti.addAP("pruebas","oscar1234");
  wifimulti.addAP("alarma","123456789");
  espclient_telegram.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  esp_task_wdt_add(NULL); // necesario para el watchdog
  Serial.begin(115200);
  pinMode(boton,INPUT);
  pinMode(senal,OUTPUT);
  pinMode(led_verde,OUTPUT);
  pinMode(led_rojo,OUTPUT);
  pinMode(led_azul,OUTPUT);
  Serial.println("\nStarting AutoConnectAP");
  esp_task_wdt_init(240, true); // establece de primer momento el valor de watcdog en 4 min para tener tiempo de la conexion AP
  encapsuladas(); // llama a las funciones principales de conexion
}

int last = millis();

void loop() {

  activacion(i); // esta en escucha de el valor de activacion
  check_status(); // chekea el estado del wifi
  if(AP_mode){ // en caso que se entre en modo AP vuelve a ejecutar las funciones principales
    encapsuladas();
    AP_mode = false;
    }
  HandleMqtt(); // Continia escucha y evaluacion de la conexion al broker
  actualizarbot(); // Esccuha del bot de telegram

   if((activar == 49 || activar == 51) && policia == true){ // manda mensaje de alerta hasta que se salga del ciclo por medio de policia = false
        bot.sendMessage(CHAT_ID, "Comunidad activo alarma!!!", "");
      }

  if (millis() - last >= 2000 && reset_esp == false) { // reset el watchdogs cada 2 seg, dado el caso no resete la esp se reinciiara
      Serial.println("Resetting WDT...");
      esp_task_wdt_reset();
      last = millis();
      if (reset_esp == true) { // con esto se asegura la reiniciada del esp cuabndo se requiera xD
        Serial.println("Stopping WDT reset. CPU should reboot in 3s");
        reset_esp == false;
      }
  }


}