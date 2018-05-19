#include <vector>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Wire.h>
#include <BH1750.h>
#include <Adafruit_BMP280.h>
#include <RtcDS3231.h>
#include "DHT.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#define DHTPIN 12
#define DHTTYPE DHT22 
Adafruit_BMP280 bme;
BH1750 lightMeter;
DHT dht(DHTPIN, DHTTYPE);
RtcDS3231<TwoWire> Rtc(Wire);
OneWire oneWire(2);
DallasTemperature sensors(&oneWire);
 
MDNSResponder mdns;
const char* ssid = "ssid";
const char* password = "pass";
ESP8266WebServer server(80);
String webPage = "";
//int count = 0;
int prevupd=0;
void setup(void){
  Wire.begin(4,5);
  lightMeter.begin();
  bme.begin();
  Rtc.Begin();
  sensors.begin();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
 
  delay(1000);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");
 
  // ждем соединения:
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");  //  "Подключились к "
  Serial.println(ssid);
  Serial.print("IP address: ");  //  "IP-адрес: "
  Serial.println(WiFi.localIP());
 
  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }
 
  server.on("/", [](){
    server.send(200, "text/html", "<pre>" + SendString() + "</pre>");
  });
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();
  if (((getMinutes() % 10 == 0) or (getMinutes() % 10 == 5)) and (getMinutes() != prevupd)){
    prevupd = getMinutes();
    Serial.println(printDateTime(Rtc.GetDateTime()) + "!!!Send information!!!");
    delay(5000);
    SendToServer();
  }
}

void SendToServer(){
  String tosend = SendString();
  WiFiClient client;
  if(client.connect("narodmon.ru", 8283)){
    Serial.println("Connected!");
    //client.print(tosend); // И отправляем данные
  }else{
    Serial.println("error!");
  }
  Serial.println("\r\n\r\n" + tosend);
}


String SendString() {
      delay(1000);
      String buf; // Буфер для отправки на NarodMon
      buf="#" + WiFi.macAddress()+ "#ESP-meteo" + "\r\n"; //формируем заголовок
      buf.replace(":", "-");
      buf += "#T1#" + String(getTempBMP()) + "#BMP280-temp" + "\r\n";
      buf += "#T2#" + String(getTempDHT()) + "#DHT22-temp" + "\r\n";
      buf += "#P1#" + String(getPress()) + "#BMP280-pressure" + "\r\n";
      buf += "#L1#" + String(getLight()) + "#BH1750-light" + "\r\n";
      buf += "#H1#" + String(getHumidity()) + "#DHT22-humanity" + "\r\n";
      buf += "#TH1#" + String(getHIC()) + "#DHT22-HeatIndex" + "\r\n";
      buf += "#TD1#" + String(getDallas()) + "#ds18b20-temp" + "\r\n";
      buf += "##";
      return buf;
}


//-------------------BMP--------------------
float getTempBMP(){
  return bme.readTemperature();
}
//------------------Dallas--------------
float getDallas(){
  sensors.requestTemperatures();
  delay(300);
  return sensors.getTempCByIndex(0);
}

float getPress(){
  return bme.readPressure();
}

float getAlt(){
  return bme.readAltitude(1760);
}
//-----------------LIGHT---------------------

int getLight() {
    return lightMeter.readLightLevel();
}

//-----------------DHT----------------------
float getHumidity() {
    return dht.readHumidity();
}

float getTempDHT() {
    return dht.readTemperature();
}

float getHIC() {
   return dht.computeHeatIndex(getTempDHT(), getHumidity(), false);
}

//------------------RTC-------------------
#define countof(a) (sizeof(a) / sizeof(a[0]))
String printDateTime(const RtcDateTime& dt)
{
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    return datestring;
}
int getMinutes(){
  return Rtc.GetDateTime().Minute();
}


