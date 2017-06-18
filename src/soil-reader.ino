#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// WiFi AP and MQTT definitions

const char* ssid = "";
const char* password = "";
const char* mqtt_server = "";
const char* mqtt_user = "";
const char* mqtt_password = "";

// timeserver deifintions

#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "europe.pool.ntp.org"

// ds18b20 definitions

#define ONE_WIRE_BUS_0 2  // DS18B20 pin
OneWire oneWire(ONE_WIRE_BUS_0);
DallasTemperature DS18B20(&oneWire);
float oldTemp0;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

WiFiClient espClient;
PubSubClient client(espClient);
char msg[50]; // Message for publishing

int sensorPin = A0; // select the input pin for soil sensor
unsigned int adc0 = 0; // variable to store the value coming from the sensor

// Used for calculating period
//unsigned long previousMillis = 0;

// constants won't change:
// 30e6 is 30 seconds
const long interval = 30e7;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  timeClient.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);

  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  timeClient.update();

  int16_t adc0, adc1, adc2;

      // Get reading from ADC pin

      adc0 = analogRead(sensorPin); // read the value from the sensor

      // Attempt to get data from ds1b20

      float temp0;

      //do {
      Serial.println("Trying to get temperature reading...");
      DS18B20.requestTemperatures();
      temp0 = DS18B20.getTempCByIndex(0);

      // Get time from time server
      String formattedTime = timeClient.getFormattedTime();

      snprintf (msg, 75, " %d.%02d", (int)temp0, (int)(temp0*100)%100);
      Serial.print(formattedTime);
      Serial.println(msg);
      client.publish("tele/soil/pot0/temp0", msg);

      snprintf (msg, 75, " %ld", adc0);
      Serial.print(formattedTime);
      Serial.println(msg);
      client.publish("tele/soil/pot0/moisture0", msg);

      Serial.println("Going into deep sleep");
      ESP.deepSleep(interval);
}

void loop() {}
