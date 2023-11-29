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
const char topic[] = "NK2/Air_Pressure/test";

const long interval = 8000;
unsigned long previousMillis = 0;

const int numReadings = 25;    // Number of voltage readings to collect
float voltageReadings[numReadings];
int currentReading = 0;
bool readingsComplete = false;

const float alfa = 0.1;        // Exponential moving average factor
float ema = 0.0;               // Exponential moving average

void setup() {
  Serial.begin(9600);
  analogRead(A2);

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
    // Read the sensor value
    int sensorValue = analogRead(A2);
    float voltage = convertAnalogToVoltage(sensorValue);

    // Store the voltage reading
    voltageReadings[currentReading] = voltage;
    currentReading++;

    // Check if all readings have been collected
    if (currentReading >= numReadings) {
      currentReading = 0;
      readingsComplete = true;
    }

    // Calculate the exponential moving average of the collected voltage readings
    if (readingsComplete) {
      ema = voltageReadings[0];
      for (int i = 1; i < numReadings; i++) {
        ema = (alfa * voltageReadings[i]) + ((1 - alfa) * ema);
      }

      // Print the analog value and filtered voltage
      Serial.print("Analog Value: ");
      Serial.print(sensorValue);
      Serial.print("\tVoltage: ");
      Serial.println(ema, 3);  // Print filtered voltage with 3 decimal points

      // Convert voltage mean to MPa using the function y = 0.262x - 0.265
      float mpa = 0.262 * ema - 0.265;

      // Publish the mean value in MPa
      Serial.print("Pressure: ");
      Serial.print(mpa, 3);  // Print pressure with 3 decimal points
      Serial.println(" MPa");
      publishMessage(topic, String(mpa, 3));

      // Reset the readingsComplete flag
      readingsComplete = false;
    }

    // Wait for a short delay
    delay(100);
  }
}

float convertAnalogToVoltage(float analogValue) {
  // Apply the quadratic curve conversion
  float voltage = analogValue * (5.0 / 1023.0);

  return voltage;
}

void connectToWiFiAndMQTT() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Check if Wi-Fi is disconnected
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print("Attempting to connect to WPA SSID: ");
      Serial.println(ssid);

      while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
        Serial.print(".");
        delay(5000);
      }

      Serial.println("Connected to the network");
    }

    // Check if MQTT is disconnected
    if (!mqttClient.connected()) {
      Serial.print("Attempting to connect to the MQTT broker: ");
      Serial.println(broker);

      while (!mqttClient.connect(broker, port)) {
        Serial.print("MQTT connection failed! Error code = ");
        Serial.println(mqttClient.connectError());
        delay(5000);
      }

      Serial.println("Connected to the MQTT broker");
    }
  }
}

void publishMessage(const char* topic, String message) {
  Serial.print("Sending message to topic: ");
  Serial.println(topic);
  Serial.println(message);

  mqttClient.beginMessage(topic);
  mqttClient.print(message);
  mqttClient.endMessage();

  Serial.println();
}
