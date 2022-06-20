#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SimpleDHT.h>

const char *ssid = "POCO X3 NFC";            // sesuaikan dengan username wifi
const char *password = "lumajang";        // sesuaikan dengan password wifi
const char *mqtt_server = "broker.hivemq.com"; // isikan server broker

WiFiClient espClient;
PubSubClient client(espClient);

SimpleDHT11 dht11(D6);
#define triggerPin  D8
#define echoPin     D7

long now = millis();
long lastMeasure = 0;
String macAddr = "";

void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
  macAddr = WiFi.macAddress();
  Serial.println(macAddr);
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(macAddr.c_str()))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup()
{
  // s
  Serial.begin(115200);
  Serial.println("Mqtt Node-RED");
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  if (!client.loop())
  {
    client.connect(macAddr.c_str());
  }
  now = millis();
  if (now - lastMeasure > 5000)
  {
    lastMeasure = now;
    int err = SimpleDHTErrSuccess;

    byte temperature = 0;
    byte humidity = 0;

    String statusSuhu;
    if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess)
    {
      Serial.print("Pembacaan DHT11 gagal, err=");
      Serial.println(err);
      delay(1000);
      return;
    }
    if(temperature >= 26 && temperature <= 30){
      statusSuhu = "Normal";
    }else{
      statusSuhu = "Dingin"; 
    }

    static char temperatureTemp[7];
    static char kelembapanTemp[7];
    static char ketinggianTemp[7];

    dtostrf(temperature, 4, 2, temperatureTemp);
    dtostrf(humidity, 4, 2, kelembapanTemp);

    long duration, jarak;
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2); 
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10); 
    digitalWrite(triggerPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    jarak = (duration/2) / 29.1;
    Serial.println("jarak :");
    delay(100);
    
    Serial.println(temperatureTemp);
    Serial.println(kelembapanTemp);
    
    dtostrf(jarak, 4, 2, ketinggianTemp);
    Serial.print(ketinggianTemp);
    Serial.println("cm");

    client.publish("room/suhuu", temperatureTemp); 
    client.publish("room/ketinggian", ketinggianTemp); 
    client.publish("room/kelembapan", kelembapanTemp); 

  }
}
