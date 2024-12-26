#include <ESP8266WiFi.h>
#include <Wire.h>
#include <DHT11.h>

#define DHT_PIN 16
#define PUMP_PIN 5
#define HUMI_GROUND_PIN A0

#define ssid       "iot ptit"
#define password   "1234567890"

WiFiClient client;
DHT11 dht11(DHT_PIN);

#define writeAPIKey "9D0TN74ARR0CFYL4"
#define server		  "api.thingspeak.com"

unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

//FunctionDeclare
void    wifiSetup();
void    thingConnect();

void setup() {
  Serial.begin(9600);
  wifiSetup();
  pinMode(PUMP_PIN,OUTPUT);//relay
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    lastTime = millis();
    if(WiFi.status() != WL_CONNECTED){
      ConnectWiFI();
    }
    int temp = 0;
    int humi = 0;
    int pump = 0;
    int result  = dht11.readTemperatureHumidity(temp, humi);
    if (result == 0) {
    }else{
      Serial.println("Cant read data on DHT11");
    };
    int humi_ground_raw = 0;
    for(int i=0;i<=9;i++){
    humi_ground_raw+=analogRead(HUMI_GROUND_PIN);
    }
    humi_ground_raw=humi_ground_raw/10;
    int humi_ground = 100 - map(humi_ground_raw, 350, 1023, 0, 100);
    if(humi_ground < 20){
      pump = 1;
    }else{
      pump = 0;
    }
    digitalWrite(PUMP_PIN,pump);
    thingConnect(temp,humi,humi_ground,pump);
  }
}

void wifiSetup() {
  Serial.print("Connecting");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.print("\r\nWiFi connected");
}

void thingConnect(int temp, int humi,int humi_ground,int pump) {
  if (client.connect(server, 80)) {
    String body = "field1=" + String(temp) + "&field2=" + String(humi) + "&field3=" + String(humi_ground) + "&field4=" + String(pump);
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + String(writeAPIKey) + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(body.length());
    client.print("\n\n");
    client.print(body);
    client.print("\n\n");
    Serial.println(body);
  }
}

void ConnectWiFI() {
  Serial.print("Connecting");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\r\nWiFi connected");
}