#include <ESP_WiFiManager.h>       //libreria para la conexion wifi Auotoconect aP en este caso
#include "mqtt_client.h"           // conexion al broker
#include "PubSubClient.h"          // publicar en el broker y suscribir en el broker
#include <esp_task_wdt.h>          // WatchDog
#include <Ticker.h>                // necesaria para activar funciones independientemente del programa
#ifdef ESP32                       // se define la esp a usar segun eso las librerias que se utilizan
#include <esp_wifi.h>              // conexion wifi, con esto se crea un cliente wifi (No usada)
#include <WiFi.h>                  // igual al anterior
#include <WiFiClientSecure.h>      // necesario para telegram
#include <WiFiMulti.h>             // sirve para conectar a multiples APS
#include <WiFiClient.h>            // cliente wifi
#include <UniversalTelegramBot.h>  // bot de telegram
#include <ArduinoJson.h>           // telegram

// #define ESP_getChipId()   ((uint32_t)ESP.getEfuseMac())
#define BOTtoken "5909477841:AAGeXd50ajjAVVPjPJw7hN27IxZAiQi6bpo"  // your Bot Token (Get from Botfather)
#define CHAT_ID "-1001595822537"                                   //id grupo de telegram

#define LED_ON HIGH
#define LED_OFF LOW
#define ONBOARD_LED 2
#else
#include <ESP8266WiFi.h>  //https://github.com/esp8266/Arduino
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>

#define ESP_getChipId() (ESP.getChipId())

#define LED_ON LOW
#define LED_OFF HIGH
#endif
#define triacPulse 33  // PIN DE SALIDA HACIA EL MOC3021   (AZUL)
#define ZVC 32         // PIN DE ENTRADA DESDE EL DETECTOR DE CRUCE POR CERO 4N25 (BLANCO)
// variables para configurar MQTT
const char *mqtt_server = "140.238.178.88";  //140.238.178.88
const int mqtt_port = 1883;
const char *mqtt_user = "rcaso";
const char *mqtt_pass = "";

//

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
String chipID = String(ESP_getChipId(), HEX);  //Obitene el id de la ESP
//chipID.toUpperCase();
const char *root_topic_subscribe = "Alarma/";
String Topico_sub = String(root_topic_subscribe) + chipID;
char msg[50];
const char *root_topic_publish = "/mensajes";
static ulong timecontrolprueba = 0;
int numeroEntero = 0;
int ctrlconn = 1;
String grupo = "";

int activar = -1;
String mensaje;
String alarmaMqtt;
int i = 0;
unsigned int mqttstate;
// parametros para variar el cruce por cero
int Slider_Value = 0;
int dimming = 7200;

// parametros para variar la alarma
//const int alarma = 23;             // PIN de salida
const int freq = 5000;             // frecuencia por defecto 5khz
const int ledChannel = 0;          // canal de salida de PWM 0 o 1
const int resolutionPWM = 8;       // resolucion de la salida PWM 8 por defecto
unsigned long tiempoAnterior = 0;  // paramtro para hacer la duracion de la alarma
bool Bandera1 = false;
bool Bandera2 = false;
bool Bandera3 = false;
bool Bandera4 = false;
unsigned long previousMillis = 0;
int cont1, cont2, cont3, cont4;
unsigned long time1;
unsigned long time2;
unsigned long time3;
int contadorestado;
String fullTopic;  // Ajusta el tamaño según tus necesidades

//int setAlarma = 0;
//
//const String root_topic_subscribe = cerbero/
const int boton = 3;
// const int led_verde = 32;
// const int led_rojo = 33;
// const int led_azul = 34;
boolean suscripcionTopico = true;

int botRequestDelay = 1000;
unsigned long lastTimeBotRan;
// funciones de activacion y desactivacion de alarmas
Ticker alarmaPerros;     // activa funcionPerros()
Ticker alarmaLadron;     // activa funcionLadron()
Ticker alarmaPelea;      // activa funcionPlea()
Ticker alarmaTerremoto;  // activa funcionTerremoto()

void funcionPerros() {
  Serial.println(contadorestado);
  if (contadorestado >= 4) {
    Serial.println("ACABO PERROS");
    Bandera1 = false;
    alarmaMqtt = "0";
    dimming = 7200;
    digitalWrite(triacPulse, LOW);
    alarmaPerros.detach();
    contadorestado = 0;
    esp_task_wdt_reset();
  }
  if (contadorestado % 2 == 0) {
    dimming = 7200;
    //Serial.println(dimming);
  } else {
    dimming = 200;
    //Serial.println(dimming);
  }

  contadorestado++;
}
void funcionLadron() {
  Serial.println(contadorestado);
  if (contadorestado >= 6) {
    Serial.println("ACABO LADRON");
    Bandera2 = false;
    alarmaMqtt = "0";
    dimming = 7200;
    digitalWrite(triacPulse, LOW);
    alarmaLadron.detach();
    contadorestado = 0;
    esp_task_wdt_reset();
  }
  if (contadorestado % 2 == 0) {
    dimming = 7200;
    //Serial.println(dimming);
  } else {
    dimming = 200;
    //Serial.println(dimming);
  }

  contadorestado++;
}

void funcionPelea() {  //
  Serial.println(contadorestado);
  if (contadorestado >= 6) {
    Serial.println("ACABO PELEA");
    Bandera3 = false;
    alarmaMqtt = "0";
    dimming = 7200;
    digitalWrite(triacPulse, LOW);
    alarmaPelea.detach();
    contadorestado = 0;
    esp_task_wdt_reset();
  }
  if (contadorestado % 2 == 0) {
    dimming = 7200;
    //Serial.println(dimming);
  } else {
    dimming = 200;
    //Serial.println(dimming);
  }

  contadorestado++;
}

void funcionTerremoto() {
  Serial.println(contadorestado);
  if (contadorestado >= 10) {
    Serial.println("ACABO TERREMOTO");
    Bandera4 = false;
    alarmaMqtt = "0";
    dimming = 7200;
    digitalWrite(triacPulse, LOW);
    alarmaTerremoto.detach();  // termina la funcion
    contadorestado = 0;
    esp_task_wdt_reset();
  }
  if (contadorestado % 2 == 0) {
    dimming = 7200;
    //Serial.println(dimming);
  } else {
    dimming = 200;
    //Serial.println(dimming);
  }

  contadorestado++;
}
void activacion() {  // aqui es para modelar el motor
  alarmaMqtt = mensaje;

  //Serial.print(alarmaMqtt);
  //Serial.println("ALARMA PARA PERROS");
  if (alarmaMqtt != 0) {
    actualizarbot();
    delay(1);
    //Serial.println(alarmaMqtt);
    if (alarmaMqtt == "1" || Bandera1 == true) {  // programa para alarma para perros
      // iniciamos la alarma
      Bandera1 = true;
      if (policia == true) {

        bot.sendMessage(CHAT_ID, "Comunidad activo alarma!!!", "");
        bot.sendMessage(CHAT_ID, "UN PERRO ORINANDO EN LA CALLE", "");
      }
      Serial.println("ALARMA PARA PERROS");
      alarmaPerros.attach(1, funcionPerros);
    }

    if (alarmaMqtt == "2" || Bandera2 == true) {  // alarma para ladrones
      // iniciamos la alarma
      delay(1);
      Bandera2 = true;
      if (policia == true) {
        bot.sendMessage(CHAT_ID, "Comunidad activo alarma!!!", "");
        bot.sendMessage(CHAT_ID, "HAY UN LADRON!!!", "");
      }
      Serial.println("ALARMA PARA LADRONES");
      alarmaPerros.attach(1.5, funcionLadron);  // activa el tiempo de la funcion
    }
    if (alarmaMqtt == "3" || Bandera3 == true) {  // alarma para PELEA// iniciamos la alarma
      delay(1);
      Bandera3 = true;
      if (policia == true) {  // llama al bot de telegram
        bot.sendMessage(CHAT_ID, "Comunidad activo alarma!!!", "");
        bot.sendMessage(CHAT_ID, "HAY UNA  PELEA EN EL SECTOR!!!", "");
      }

      Serial.println("ALARMA PARA PELEA");
      alarmaPelea.attach(1, funcionPelea);
    }
    if (alarmaMqtt == "5" || Bandera4 == true) {  // alarma para TERREMOTO
      // iniciamos la alarma
      delay(1);
      Bandera4 = true;
      if (policia == true) {
        bot.sendMessage(CHAT_ID, "Comunidad activo alarma!!!", "");
        bot.sendMessage(CHAT_ID, "HAY UNA  PELEA EN EL SECTOR!!!", "");
      }

      Serial.println("ALARMA PARA TERREMOTO");
      alarmaTerremoto.attach(2, funcionTerremoto);  // activa el tiempo de la funcion terremoto
    }
    esp_task_wdt_reset();
  } else {
    //setDisparo(0);
  }
  delay(1);
}

void handleNewMessages(int numNewMessages) {  //funcion que espera mensajes de telegram
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {              // recibe el numero de mensajes, es necesario, auqnue sea 1
    String chat_id = String(bot.messages[i].chat_id);     //guarda el identificador del chat en formato string
    if (chat_id != CHAT_ID) {                             // comparamos los identificadores
      bot.sendMessage(chat_id, "Unauthorized user", "");  // funcion para mandar un mensaje al chat identificado
      Serial.println("El chat id es:" + chat_id);
      continue;  //Al no ser autorizado, sale del ciclo "for" por ende no ejecuta el codigo restante
    }

    // Se imprime el mensaje recivido
    String text = bot.messages[i].text;  //obtiene el texto del chat iterando la cantidad de mensajes
    Serial.println(text);

    String from_name = bot.messages[i].from_name;  // obtenemos el nombre del usuario de telegram (Nikname)

    if (text == "/comando") {  // nos muestra los comandos a utilizar en el chat de telegram
      String welcome = "La comunidad necesita de ti, " + from_name + ".\n";
      welcome += "Este es un sistema de alarma comunitaria con aviso directo a las autoridades.\n\n";
      welcome += "/Policia_en_camino para detener mensajes y acudir a la emergencia \n";
      welcome += "/Apagar para detener alarma \n";
      welcome += "/Estado para detectar el estado de la alarma \n";
      bot.sendMessage(chat_id, welcome, "");
    }
  }
}


void callback(char *topic, byte *payload, unsigned int length) {  // recibe los mensajes del broker y el topico suscrito
  mensaje = "";
  Serial.print("Mensaje: ");
  for (int i = 0; i < length; i++) {
    mensaje += (char)payload[i];  // variable para recibir mensajes
  }
  mensaje.trim();
  Serial.println(mensaje + "\n");
  String grupoEsp = grupo + "_" + chipID;  //se crea grupo para que no se repita

  String topic1 = "+/";
  String topic2 = grupoEsp;
  fullTopic = topic1 + topic2;

  activacion();                         // esta en escucha de el valor de activacion
  bot.sendMessage(CHAT_ID, topic, "");  // Evnaimos a telegram, el topico de el cual se recibio el mensaje
}

void heartBeatPrint(void) {  //revisa la conexion de el Wifi

  static int num = 1;
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= 400) {
        digitalWrite(2, HIGH);
        previousMillis = 0;
      }
      if (currentMillis - previousMillis >= 800) {
        digitalWrite(2, LOW);
        previousMillis = 0;
      }
      if (currentMillis - previousMillis >= 1200) {
        digitalWrite(2, HIGH);
        previousMillis = 0;
      }
      if (currentMillis - previousMillis >= 1600) {
        digitalWrite(2, LOW);
        previousMillis = 0;
      }
      reconnect();
    }


  } else {
    Serial.println("F");
    digitalWrite(2, LOW);
    if (wifimulti.run() == WL_CONNECTED) {  // si se cae la red actual, intenta conectar con las redes establecidas en el multiWifi
      Serial.println("me conecte a otra red papu");
      Serial.print(WiFi.SSID());
    }
    //digitalWrite(2,HIGH;
  }
  // F means not connected to WiFi
  if (num == 80) {
    Serial.println();
    num = 1;
  } else if (num++ % 10 == 0) {
    Serial.print(" ");
  }
}

void check_status() {  //funcion que va en el loop para verificar la conexion

  byte apmode = digitalRead(boton);      // lee la entrada digital y guarda su valor 1 o 0
  static ulong checkstatus_timeout = 0;  // tiempo donde se compara el valor de millis()

#define HEARTBEAT_INTERVAL 1000L  // El "Delay" para aumentar a la funcion millis()
  // Print hearbeat every HEARTBEAT_INTERVAL (10) seconds.
  if ((millis() > checkstatus_timeout) || (checkstatus_timeout == 0)) {
    heartBeatPrint();                                     //llama a la funcion de comprobacion del wifi
    checkstatus_timeout = millis() + HEARTBEAT_INTERVAL;  // vuelve a entrar cuando millis sea mayor a 1seg
  }
  if (WiFi.status() != WL_CONNECTED && apmode == 1) {  // Si no encuentra ninguna red permite evaluar el estado del boton
    AP_mode = true;                                    // activa el modo AP, al reinicio del esp, entrara en este modo
    if (wifimulti.run() == WL_CONNECTED) {             // Busca una red a la cual conectarse
      Serial.println("me conecte a otra red papu");
      Serial.print(WiFi.SSID());
    }
  }
}

void autoconnectap() {

  ESP_WiFiManager ESP_wifiManager("AutoConnectAP");
  ESP_wifiManager.setDebugOutput(true);
  ESP_wifiManager.setAPStaticIPConfig(IPAddress(192, 168, 0, 120), IPAddress(192, 168, 0, 1), IPAddress(255, 255, 255, 0));  // Crea la configuracion para la red AP
  ESP_wifiManager.setMinimumSignalQuality(-1);                                                                               // la minima n permitida
  Router_SSID = ESP_wifiManager.WiFi_SSID();                                                                                 // Obtiene la ultima red coneectada eeprom
  Router_Pass = ESP_wifiManager.WiFi_Pass();                                                                                 //obtiene la contraseña de la ultima red conectada, Eeprom
  //Router_SSID = "CARLOS";        // Obtiene la ultima red coneectada eeprom
  //Router_Pass = "Cm3148011184";  //obtiene la contraseña de la ultima red conectada, Eeprom
  Serial.println("Stored: SSID = " + Router_SSID + ", Pass = " + Router_Pass);

  if (Router_SSID != "")  //existen credenciales y sigue la conexion
  {
    ESP_wifiManager.setConfigPortalTimeout(0);  //If no access point name has been previously entered disable timeout.
    Serial.println("Got stored Credentials. Timeout 60s");

  } else {
    Serial.println("No stored Credentials. No timeout");
    if (wifimulti.run() == WL_CONNECTED) {  //No encuentra ninguna conexion anterior por lo tanto accede por medio de las guardadas en multiwifi, este es un machete a corregir
      Serial.println("me conecte a otra red papu");
      Serial.print(WiFi.SSID());
    }

    Serial.println("Sali de aqui gf");
  }

  //chipID.toUpperCase();

  String AP_SSID = "Alarma" + chipID;  //El nombre de la red AP
  String AP_PASS = "ESP_" + chipID;    //Pass de la red AP
  if (AP_mode) {                       // No esta en este modo a menos que se especifique dadas las condiciones
    ESP_wifiManager.resetSettings();   // Resetea toda la informacion de las conexiones de la eeprom
    esp_task_wdt_init(240, true);      // establece el watchdog en 4 min para asi tener tiempo de conectarse
  }
  //digitalWrite(led_azul, HIGH);
  Serial.println(AP_SSID);
  Serial.println(AP_PASS);
  //if (ESP_wifiManager.autoConnect(AP_SSID.c_str(), AP_PASS.c_str())) {  // intenta conectarse con las credenciales guardadas en la eprom
  if (ESP_wifiManager.autoConnect(Router_SSID.c_str(), Router_Pass.c_str())) {
    conexion = true;
    Serial.println("4");  // la conexion fue correcta, sigue en el codigo
    Serial.println("WiFi connected la ip es:" + WiFi.localIP().toString());
    delay(1);
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 300) {
      digitalWrite(2, HIGH);
      previousMillis = 0;
    }
    currentMillis = millis();
    if (currentMillis - previousMillis >= 600) {
      digitalWrite(2, LOW);
      previousMillis = 0;
    }
    currentMillis = millis();
    if (currentMillis - previousMillis >= 900) {
      digitalWrite(2, HIGH);
      previousMillis = 0;
    }
    currentMillis = millis();
    if (currentMillis - previousMillis >= 1200) {
      digitalWrite(2, LOW);
      previousMillis = 0;
      /*
    digitalWrite(ONBOARD_LED, HIGH);  // Sale del modo AP
    delay(250);
    digitalWrite(ONBOARD_LED, LOW);  // Sale del modo AP
    delay(250);
    digitalWrite(ONBOARD_LED, HIGH);  // Sale del modo AP
    delay(250);
    digitalWrite(ONBOARD_LED, LOW);  // Sale del modo AP*/
    }
  }
}

void encapsuladas() {  // funcion que trae las funciones principales conexion wifi y broker
  Serial.println("INICIO DE CONEXION");
  autoconnectap();  // llama a la conexion a internet
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  esp_task_wdt_init(20, true);
}
void reconnect() {

  while (!client.connected()) {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= 500) {
      digitalWrite(2, !digitalRead(2));
      previousMillis = currentMillis;
    }
    if (currentMillis - previousMillis >= 1000) {
      digitalWrite(2, !digitalRead(2));
      previousMillis = currentMillis;
    }
    if (currentMillis - previousMillis >= 500) {
      digitalWrite(2, !digitalRead(2));
      previousMillis = currentMillis;
    }
    if (currentMillis - previousMillis >= 500) {
      digitalWrite(2, !digitalRead(2));
      previousMillis = currentMillis;
    }
    Serial.print("Intentando conexión Mqtt...");
    // Creamos un cliente ID
    String clientId = chipID;
    // Intentamos conectar
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("Conectado: " + clientId);
      // Nos suscribimos
      //root_topic_subscribe += chipID;
      //Serial.println(chipID);
      Topico_sub.toCharArray(msg, 50);
      //root_topic_subscribe.toCharArray(Topico_sub, 50);
      if (client.subscribe(msg)) {
        Serial.print("Suscripcion: ");
        Serial.println(Topico_sub);
      } else {
        Serial.println("fallo Suscripciión");
      }
    } else {
      Serial.print("falló :( con error -> ");
      Serial.print(client.state());
      Serial.println(" Intentamos de nuevo en 2 segundos");
      delay(2000);
    }
  }
}




void actualizarbot() {  // Funcion de escucha al bot de telegram
  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      Serial.println("msm recibido");
      handleNewMessages(numNewMessages);  // envia el numero de mensajes a la funcion de telehgram
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
/*
void HandleMqtt()  //revisa la conexion del broker constantemente
{
  if (!client.connected())  // si se descoencta
  {
    if (conexion) {                                            // y hay wifi mqtt_port
      mqttconnect("140.238.178.88", 1883, espclient, "", "");  // intenta nuevamnete la conexion
      client.setCallback(callback);                            // vuelve a escuchar al broker
    }
    numeroEntero = 0;
  }
  client.loop();  // llama al cliente mqtt de manera periodica
  String control = String(numeroEntero);
  static ulong checkstatus_timeout = 0;  // tiempo donde se compara el valor de millis()

#define HEARTBEAT_INTERVAL_CTRL 1000L  // El "Delay" para aumentar a la funcion millis()
  // Print hearbeat every HEARTBEAT_INTERVAL (10) seconds.
  if ((millis() > checkstatus_timeout) || (checkstatus_timeout == 0)) {
    numeroEntero++;
    client.publish("tiempo/prueba", control.c_str());
    if (numeroEntero == 100) {
      numeroEntero = 0;
    }
    checkstatus_timeout = millis() + HEARTBEAT_INTERVAL_CTRL;  // vuelve a entrar cuando millis sea mayor a 1seg
  }
}
*/

// codigo para modular el PWM de la alarma
void setup() {



  time1 = time2 = time3 = millis();

  //ledcAttachPin(alarma, ledChannel);           // configura la salida de la senal de alarma
  //

  wifimulti.addAP("CARLOS", "Cm3148011184");
  wifimulti.addAP("alarma", "123456789");
  espclient_telegram.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  esp_task_wdt_add(NULL);  // necesario para el watchdog
  Serial.begin(115200);
  pinMode(ONBOARD_LED, OUTPUT);  //Pin 2 como salida (Pin 2 es el led azul Interno del ESP32)
  // pinMode(led_verde, OUTPUT);
  // pinMode(led_rojo, OUTPUT);
  // pinMode(led_azul, OUTPUT);
  Serial.println("\nStarting AutoConnectAP_setup");
  esp_task_wdt_init(240, true);  // establece de primer momento el valor de watcdog en 4 min para tener tiempo de la conexion AP
  encapsuladas();                //SETUP                // llama a las funciones principales de conexion
  pinMode(ZVC, INPUT_PULLUP);
  pinMode(triacPulse, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(ZVC), acon, FALLING);  // attach Interrupt at PIN2
  //Serial.println("FINALIZO SETUP");
  esp_task_wdt_init(240, true);  // establece de primer momento el valor de watcdog en 4 min para tener tiempo de la conexion AP
  digitalWrite(triacPulse, LOW);
  dimming = 7200;
}

int last = millis();


void loop() {
  if (!client.connected()) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 500) {
      digitalWrite(2, !digitalRead(2));
      previousMillis = currentMillis;
    }
    if (currentMillis - previousMillis >= 1000) {
      digitalWrite(2, !digitalRead(2));
      previousMillis = currentMillis;
    }
    if (currentMillis - previousMillis >= 500) {
      digitalWrite(2, !digitalRead(2));
      previousMillis = currentMillis;
    }
    if (currentMillis - previousMillis >= 500) {
      digitalWrite(2, !digitalRead(2));
      previousMillis = currentMillis;
    }
    reconnect();
  } else {
    digitalWrite(ONBOARD_LED, HIGH);
  }
  //Serial.println((int)setAlarma, DEC);
  //HandleMqtt();

  check_status();  // chekea el estado del wifi
  if (AP_mode) {   // en caso que se entre en modo AP vuelve a ejecutar las funciones principales
    encapsuladas();
    AP_mode = false;
  }
  // Continia escucha y evaluacion de la conexion al broker
  //actualizarbot();  // Esccuha del bot de telegram

  if (millis() - last >= 2000 && reset_esp == false) {  // reset el watchdogs cada 2 seg, dado el caso no resete la esp se reinciiara
    //Serial.println("Resetting WDT...");
    esp_task_wdt_reset();
    last = millis();
    if (reset_esp == true) {  // con esto se asegura la reiniciada del esp cuabndo se requiera xD
      Serial.println("Stopping WDT reset. CPU should reboot in 3s");
      reset_esp == false;
    }
  }
  esp_task_wdt_reset();
  client.loop();
}
// funcion para detectar el cruce por cero

void acon() {

  delayMicroseconds(dimming);  // read AD0
  digitalWrite(triacPulse, HIGH);

  delayMicroseconds(50);  //delay 50 uSec on output pulse to turn on triac
  digitalWrite(triacPulse, LOW);

  // Serial.println(digitalRead(triacPulse));
}