#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>
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

#define ONE_WIRE_BUS_0 16  // DS18B20 pin
OneWire oneWire(ONE_WIRE_BUS_0);
DallasTemperature DS18B20(&oneWire);
float oldTemp0;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

// Adafruit ADC definitions stuff

Adafruit_ADS1015 ads1015;

WiFiClient espClient;
PubSubClient client(espClient);
char msg[50]; // Message for publishing

// Used for calculating period
unsigned long previousMillis = 0;

// constants won't change :
const long interval = 10000;

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
  ads1015.begin();
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  timeClient.update();

  unsigned long currentMillis = millis();

  int16_t adc0, adc1, adc2;

    if (currentMillis - previousMillis >= interval) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;

      adc0 = ads1015.readADC_SingleEnded(0);
      adc1 = ads1015.readADC_SingleEnded(1);
      adc2 = ads1015.readADC_SingleEnded(2);
      //adc3 = ads1015.readADC_SingleEnded(3);

      // Attempt to get data from ds1b20
      // Keep polling the sensor until a sensible reaf
      // is returned (maybe never if not connected!

      float temp0;

      //do {
      DS18B20.requestTemperatures();

      temp0 = DS18B20.getTempCByIndex(0);
      } while (temp0 == 85.0 || temp0 == (-127.0));

      if (temp0 != oldTemp0)
      {
      oldTemp0 = temp0;
      }

      // Get time from time server
      String formattedTime = timeClient.getFormattedTime();

      snprintf (msg, 75, " %d.%02d", (int)temp0, (int)(temp0*100)%100);
      Serial.print(formattedTime);
      Serial.println(msg);
      client.publish("test/esp/temp0", msg);

      snprintf (msg, 75, " %ld", adc0);
      Serial.print(formattedTime);
      Serial.println(msg);
      client.publish("test/esp/soil1", msg);

      snprintf (msg, 75, " %ld", adc1);
      Serial.print(formattedTime);
      Serial.println(msg);
      client.publish("test/esp/soil2", msg);

      snprintf (msg, 75, " %ld", adc2);
      Serial.print(formattedTime);
      Serial.println(msg);
      client.publish("test/esp/soil3", msg);

  }
}
