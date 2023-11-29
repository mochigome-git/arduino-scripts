#include <Wire.h>
#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"
#include <SoftwareSerial.h>

// Pin assignments
const SoftwareSerial Scale(0, 1); // RX, TX

// Wi-Fi and MQTT credentials
char ssid[] = SECRET_SSID;      // Network SSID (name)
char pass[] = SECRET_PASS;      // Network password

// MQTT settings
const char broker[] = "192.168.0.6";
int port = 1883;
const char topic[] = "Inkjet/weight/L1";
int wifiRetryCount = 0; // Add wifiRetryCount variable
int mqttRetryCount = 0; // Add mqttRetryCount variable
bool publishMessage(const char* topic, const char* message);

// Time interval for connection attempts
const unsigned long interval = 8000;
unsigned long previousMillis = 0;

// Reset function
void(* resetFunc) (void) = 0;

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

void connectToWiFiAndMQTT();

void setup() {
  Serial.begin(9600);

  while (!Serial) {
    ; // Wait for serial port to connect. Needed for native USB port only
  }

  Scale.begin(9600);
  connectToWiFiAndMQTT();
}

void loop() {
  mqttClient.poll();

  // Check if the Wi-Fi or MQTT connection is lost
  if (WiFi.status() != WL_CONNECTED || !mqttClient.connected()) {
    // Attempt to reconnect to Wi-Fi and MQTT
    connectToWiFiAndMQTT();
  } else {
    char weight[20]; // Array to store the weight value

    // Collect ASCII readings from weighing scale
    if (Scale.available()) {
      static int weightIndex = 0; // Index for weight array
      char c = Scale.read() & 0x7F; // Read a character from the scale
      if (c == '\n') {
        weight[weightIndex] = '\0'; // Null-terminate the weight array
        //Serial.println(weight); // Output the weight to Serial monitor
        publishMessage(topic, weight); // Publish the weight to MQTT
        weightIndex = 0; // Reset the weight array index
      } else {
        weight[weightIndex] = c; // Store the character in the weight array
        weightIndex++;
        if (weightIndex >= sizeof(weight) - 1) {
          weightIndex = 0; // Reset the weight array index if it reaches the end
        }
      }
    }
  }

  // Wait for a short delay
  delay(100);
}

void connectToWiFiAndMQTT() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Check if Wi-Fi is disconnected
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print("Attempting to connect to WPA SSID: ");
      Serial.println(ssid);

      if (wifiRetryCount < 3) {
        int retryCount = 0;
        while (WiFi.begin(ssid, pass) != WL_CONNECTED && retryCount < 3) {
          Serial.print(".");
          delay(500);
          retryCount++;
        }

        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("Connected to the network");
          wifiRetryCount = 0; // Reset the retry count
        } else {
          // Unable to connect to Wi-Fi, handle the error or retry
          Serial.println("Failed to connect to the network");
          wifiRetryCount++;
          if (wifiRetryCount >= 3) {
            Serial.println("Exceeded Wi-Fi connection retry limit. Restarting the program...");
            resetFunc(); // Restart the program
          }
          return;
        }
      } else {
        Serial.println("Exceeded Wi-Fi connection retry limit. Restarting the program...");
        resetFunc(); // Restart the program
      }
    }

    // Check if MQTT is disconnected
    if (!mqttClient.connected()) {
      Serial.print("Attempting to connect to the MQTT broker: ");
      Serial.println(broker);

      if (mqttRetryCount < 3) {
        int retryCount = 0;
        while (!mqttClient.connect(broker, port) && retryCount < 1) {
          Serial.print("MQTT connection failed! Error code = ");
          Serial.println(mqttClient.connectError());
          delay(500);
          retryCount++;
        }

        if (mqttClient.connected()) {
          Serial.println("Connected to the MQTT broker");
          Serial.println("Start collecting weighing scale data...");
          mqttRetryCount = 0; // Reset the retry count
        } else {
          // Unable to connect to MQTT, handle the error or retry
          Serial.println("Failed to connect to the MQTT broker");
          mqttRetryCount++;
          if (mqttRetryCount >= 3) {
            Serial.println("Exceeded MQTT connection retry limit. Restarting the program...");
            resetFunc(); // Restart the program
          }
          return;
        }
      } else {
        Serial.println("Exceeded MQTT connection retry limit. Restarting the program...");
        resetFunc(); // Restart the program
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
