#include <Wire.h>
#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"

char ssid[] = SECRET_SSID;      // Network SSID (name)
char pass[] = SECRET_PASS;      // Network password

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "192.168.0.6";
int port = 1883;
const char topic[] = "assembly_line1_test";

const long interval = 8000;
unsigned long previousMillis = 0;

int value;
int sensorValue = A0;
int previousValue = 0;

void setup() {
  Serial.begin(9600);

  while (!Serial) {
    ; // Wait for serial port to connect. Needed for native USB port only
  }

  connectToWiFi();
  connectToMQTTBroker();
}

void loop() {
  mqttClient.poll();

  // Check if the Wi-Fi connection is lost
  if (WiFi.status() != WL_CONNECTED) {
    // Disconnect from the MQTT broker
    mqttClient.stop();

    // Attempt to reconnect to the Wi-Fi network
    connectToWiFi();

    // Reconnect to the MQTT broker
    connectToMQTTBroker();
  } else {
    // Check if the MQTT client is connected
    if (!mqttClient.connected()) {
      // Disconnect from the MQTT broker
      mqttClient.stop();

      // Attempt to reconnect to the MQTT broker
      connectToMQTTBroker();
    } else {

      // Check if the previous value has changed
      // Read the sensor value status
      value = analogRead(sensorValue);
      float voltage = value * (5.0 / 1023.0);

      // Check if the value has changed
      if (value != previousValue) {
        previousValue = value;
        Serial.print("value: ");
        Serial.println(voltage);
        publishMessage(topic, voltage);
      }      
    }
  }

  delay(100); // Add a small delay to avoid reading the limit switch too frequently
}

void connectToWiFi() {
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);

  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.print(".");
    delay(5000);
  }

  Serial.println("Connected to the network");
  Serial.println();
}

void connectToMQTTBroker() {
  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  while (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    delay(5000);
  }

  Serial.println("Connected to the MQTT broker");
  Serial.println();
}

void publishMessage(const char* topic, float value) {
  Serial.print("Sending message to topic: ");
  Serial.println(topic);
  Serial.println(value, 2); // Print value with 2 decimal points

  mqttClient.beginMessage(topic);
  mqttClient.print(value, 2); // Publish value with 2 decimal points
  mqttClient.endMessage();

  Serial.println();
}