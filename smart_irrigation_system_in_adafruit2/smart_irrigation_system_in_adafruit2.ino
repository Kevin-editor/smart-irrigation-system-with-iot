#include <WiFi.h>
#include <WiFiClient.h>

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <DHT11.h>

DHT11 dht11(32);
int sensor =34;
int pump=27;
int buzzer=12;
int green=13;
int threshold=40;
int threshod=1000;

#define WLAN_SSID       "*********"   
#define WLAN_PASS       "**********"  


#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883 
#define AIO_USERNAME  "***************"
#define AIO_KEY       "*********************"

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);


Adafruit_MQTT_Publish temp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");
Adafruit_MQTT_Publish hum  = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");
Adafruit_MQTT_Publish moisturesensor  = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/soil_moisture");
Adafruit_MQTT_Subscribe onoffpump = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/pump");
void MQTT_connect();

void setup() {
  Serial.begin(115200);
  delay(10);
pinMode(green,OUTPUT);
pinMode(buzzer,OUTPUT);
pinMode(pump,OUTPUT);
pinMode(sensor,INPUT);
digitalWrite(green,HIGH);
digitalWrite(buzzer,LOW);
digitalWrite(pump,LOW);
Serial.print("SYSTEM IRRIGATION SYSTEM ");

 
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  mqtt.subscribe(&onoffpump);
}


void loop() {
  MQTT_connect();

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(500))) {
    if (subscription == &onoffpump) {
      Serial.print(F("Got1: "));
      Serial.println((char *)onoffpump.lastread);

      char *value = (char *)onoffpump.lastread;
      String message = String(value);
      message.trim();
      if (message == "1") {
        digitalWrite(pump, HIGH);
        
      }
      if (message == "0") {
        digitalWrite(pump, LOW);
        
      }
    }
  }
  
  int temperature = dht11.readTemperature();
  int humidity = dht11.readHumidity();
  int moisturevalue=analogRead(sensor);
  int moisturePercent = map(moisturevalue, 1023, 0, 0, 100);

  int result = dht11.readTemperatureHumidity(temperature, humidity);


  Serial.print(F("\nSending Humidity val "));
  Serial.print(humidity);
  if (!hum.publish((float)humidity)) {
    Serial.println(F(" -> Failed"));
  } else {
    Serial.println(F(" -> OK!"));
  }

  
  Serial.print(F("Sending Temperature val "));
  Serial.print(temperature);
  if (!temp.publish((float)temperature)) {
    Serial.println(F(" -> Failed"));
  } else {
    Serial.println(F(" -> OK!"));
  }

  Serial.print(moisturePercent);
  if (!moisturesensor.publish((float)moisturePercent)){
    Serial.println(F(" -> Failed"));
    Serial.println("WATERING");
    }
    else {
     Serial.println(F(" -> OK!"));
      }
      
  Serial.print(F("open pump "));
    if(moisturePercent < threshold){
    digitalWrite(pump,HIGH);
    digitalWrite(buzzer,HIGH);
    digitalWrite(green,LOW);
    Serial.println("WATERING");
    }
   else{
      Serial.print(F("CLose pump "));
      digitalWrite(buzzer,LOW);
      digitalWrite(pump,LOW);
      digitalWrite(green,HIGH);
    } 

  delay(5000); 
}

void MQTT_connect() {
  int8_t ret;

  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 3;

  while ((ret = mqtt.connect()) != 0) { 
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
    retries--;
    if (retries == 0) {
      while (1); 
    }
  }
  Serial.println("MQTT Connected!");
}
