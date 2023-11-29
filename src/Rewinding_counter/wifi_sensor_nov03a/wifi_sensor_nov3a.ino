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
const char topic[] = "assembly/line1/";

const long interval = 8000;
unsigned long previousMillis = 0;

int limitswitch = 13;
int state = LOW;
int value;

void setup() {
  pinMode(limitswitch, INPUT);
  Serial.begin(9600);

  while (!Serial) {
    ; // Wait for serial port to connect. Needed for native USB port only
  }

  connectToWiFiAndMQTT();
}

void loop() {
  mqttClient.poll();

  // Check if the Wi-Fi or MQTT connection is lost
  if (WiFi.status() != WL_CONNECTED || !mqttClient.connected()) {
    // Attempt to reconnect to Wi-Fi and MQTT
    connectToWiFiAndMQTT();
  } else {
    // Read the limit switch status
    value = digitalRead(limitswitch);

    // Check if the state has changed
    if (value != state) {
      state = value;
      if (state) {
        // Try to publish a message
        if (!publishMessage(topic, 1)) {
          // Error occurred while publishing
          // Handle the error or retry
          // You can choose to reconnect to Wi-Fi and MQTT here
          connectToWiFiAndMQTT();  // Reconnect to Wi-Fi and MQTT
        }
      }
    }
  }
  delay(100); // Add a small delay to avoid reading the limit switch too frequently
}


void connectToWiFiAndMQTT() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Check if Wi-Fi is disconnected
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print("Attempting to connect to WPA SSID: ");
      Serial.println(ssid);

      int retryCount = 0;
      while (WiFi.begin(ssid, pass) != WL_CONNECTED && retryCount < 5) {
        Serial.print(".");
        delay(5000);
        retryCount++;
      }

      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected to the network");
      } else {
        // Unable to connect to Wi-Fi, handle the error or retry
        Serial.println("Failed to connect to the network");
        return;
      }
    }

    // Check if MQTT is disconnected
    if (!mqttClient.connected()) {
      Serial.print("Attempting to connect to the MQTT broker: ");
      Serial.println(broker);

      int retryCount = 0;
      while (!mqttClient.connect(broker, port) && retryCount < 5) {
        Serial.print("MQTT connection failed! Error code = ");
        Serial.println(mqttClient.connectError());
        delay(5000);
        retryCount++;
      }

      if (mqttClient.connected()) {
        Serial.println("Connected to the MQTT broker");
      } else {
        // Unable to connect to MQTT, handle the error or retry
        Serial.println("Failed to connect to the MQTT broker");
        return;
      }
    }
  }
}

bool publishMessage(const char* topic, const char* message) {
  Serial.print("Sending message to topic: ");
  Serial.println(topic);
  Serial.println(message);

  if (mqttClient.beginMessage(topic)) {
    mqttClient.print(message);
    if (mqttClient.endMessage()) {
      Serial.println("Message published successfully");
      return true;
    } else {
      Serial.println("Error occurred while publishing the message");
      return false;
    }
  } else {
    Serial.println("Error occurred while beginning the message");
    return false;
  }
}