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

const int numReadings = 10;    // Number of voltage readings to collect
float voltageReadings[numReadings];
int currentReading = 0;
bool readingsComplete = false;

const float alfa = 0.1;        // Exponential moving average factor
float ema = 0.0;               // Exponential moving average

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
      // Check if all readings have been collected
      if (!readingsComplete) {
        // Read the sensor value
        int sensorValue = analogRead(A0);
        float voltage = sensorValue * (5.0 / 1023.0);

        // Store the voltage reading
        voltageReadings[currentReading] = voltage;
        currentReading++;

        // Check if all readings have been collected
        if (currentReading >= numReadings) {
          currentReading = 0;
          readingsComplete = true;
        }

        // Print the voltage reading
        Serial.print("Voltage: ");
        Serial.println(voltage);
      } else {
        // Calculate the exponential moving average of the collected voltage readings
        ema = (alfa * voltageReadings[numReadings - 1]) + ((1 - alfa) * ema);

        // Convert voltage mean to MPa using the function y = 0.262x +- 0.255
        float mpa = 0.262 * ema - 0.191;

        // Publish the mean value in MPa
        publishMessage(topic, mpa);

        // Reset the readingsComplete flag
        readingsComplete = false;
      }
    }
  }

  delay(100); // Add a small delay to avoid reading the sensor too frequently
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
  Serial.println(value, 3); // Print value with 2 decimal points

  mqttClient.beginMessage(topic);
  mqttClient.print(value, 3); // Publish value with 2 decimal points
  mqttClient.endMessage();

  Serial.println();
}