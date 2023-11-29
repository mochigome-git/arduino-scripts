#include <Wire.h>
#include <ArduinoMqttClient.h>
#include "WiFiS3.h"
#include "arduino_secrets.h"
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix matrix;

// Pin assignments
const int sensorPin = A0;

// Wi-Fi and MQTT credentials
char ssid[] = SECRET_SSID; // Network SSID (name)
char pass[] = SECRET_PASS; // Network password
//int keyIndex = 0;          // Network key index number (needed only for WEP)

int led = LED_BUILTIN;
int status = WL_IDLE_STATUS;
WiFiServer server(80);

// MQTT settings
const char broker[] = "192.168.0.6";
int port = 1883;
const char topic[] = "NK2/Fibre/2U";

// Time interval for connection attempts
const unsigned long interval = 8000;
unsigned long previousMillis = 0;

// Constants for readings
const int numReadings = 25; // Number of voltage readings to collect
const float alfa = 0.1;     // Exponential moving average factor

// Arrays for readings
float voltageReadings[numReadings];
int currentReading = 0;
bool readingsComplete = false;

// Exponential moving average
float ema = 0.0; // Exponential moving average

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

void setup()
{
    Serial.begin(9600);
    analogRead(sensorPin);
    matrix.begin();

    while (!Serial)
    {
        ; // Wait for serial port to connect. Needed for native USB port only
    }

    connectToWiFiAndMQTT();
}

uint32_t frame[] = {
    0, 0, 0, 0xFFFF};

void loop()
{
    mqttClient.poll();

    // Check if the Wi-Fi or MQTT connection is lost
    if (WiFi.status() != WL_CONNECTED || !mqttClient.connected())
    {
        // Attempt to reconnect to Wi-Fi and MQTT
        connectToWiFiAndMQTT();
    }
    else
    {
        // Read the sensor value
        int sensorValue = analogRead(sensorPin);
        float voltage = convertAnalogToVoltage(sensorValue);

        // Store the voltage reading
        voltageReadings[currentReading] = voltage;
        currentReading++;

        // Check if all readings have been collected
        if (currentReading >= numReadings)
        {
            currentReading = 0;
            readingsComplete = true;
        }

        // Calculate the exponential moving average of the collected voltage readings
        if (readingsComplete)
        {
            ema = voltageReadings[0];
            for (int i = 1; i < numReadings; i++)
            {
                ema = (alfa * voltageReadings[i]) + ((1 - alfa) * ema);
            }

            // Print the analog value and filtered voltage
            Serial.print("Analog Value: ");
            Serial.print(sensorValue);
            Serial.print("\tVoltage: ");
            Serial.println(ema, 3); // Print filtered voltage with 3 decimal points

            // Convert voltage mean to MPa using the function y = 0.262x - 0.265
            float mpa = 0.262 * ema;

            // Publish the mean value in MPa
            Serial.print("Pressure: ");
            Serial.print(mpa, 3); // Print pressure with 3 decimal points
            Serial.println(" MPa");

            if (publishMessage(topic, String(mpa, 3).c_str()))
            {
                // Message published successfully
                readingsComplete = false;
                matrix.beginDraw();

                matrix.stroke(0xFFFFFFFF);
                matrix.textScrollSpeed(50);

                // add the text
                const char text[] = "    On Air!    ";
                matrix.textFont(Font_5x7);
                matrix.beginText(0, 1, 0xFFFFFF);
                matrix.println(text);
                matrix.endText(SCROLL_LEFT);

                matrix.endDraw();
            }
            else
            {
                // Error occurred while publishing
                // Handle the error or retry
                connectToWiFiAndMQTT(); // Reconnect to Wi-Fi and MQTT
            }
        }
        // Wait for a short delay
        delay(100);
    }
}

float convertAnalogToVoltage(float analogValue)
{
    // Apply the quadratic curve conversion
    float voltage = analogValue * (5 / 1023.0);

    return voltage;
}

void connectToWiFiAndMQTT()
{
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis;

        // Check if Wi-Fi is disconnected
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.print("Attempting to connect to WPA SSID: ");
            Serial.println(ssid);

            int retryCount = 0;
            while (WiFi.begin(ssid, pass) != WL_CONNECTED && retryCount < 5)
            {
                Serial.print(".");
                delay(5000);
                retryCount++;
            }

            if (WiFi.status() == WL_CONNECTED)
            {
                pinMode(led, OUTPUT);
                Serial.println("Connected to the network");
                printWiFiStatus();
            }
            else
            {
                // Unable to connect to Wi-Fi, handle the error or retry
                Serial.println("Failed to connect to the network");
                return;
            }
        }

        // Check if MQTT is disconnected
        if (!mqttClient.connected())
        {
            Serial.print("Attempting to connect to the MQTT broker: ");
            Serial.println(broker);

            int retryCount = 0;
            while (!mqttClient.connect(broker, port) && retryCount < 5)
            {
                Serial.print("MQTT connection failed! Error code = ");
                Serial.println(mqttClient.connectError());
                delay(5000);
                retryCount++;
            }

            if (mqttClient.connected())
            {
                Serial.println("Connected to the MQTT broker");
            }
            else
            {
                // Unable to connect to MQTT, handle the error or retry
                Serial.println("Failed to connect to the MQTT broker");
                return;
            }
        }
    }
}

bool publishMessage(const char *topic, const char *message)
{
    Serial.print("Sending message to topic: ");
    Serial.println(topic);
    Serial.println(message);

    if (mqttClient.beginMessage(topic))
    {
        mqttClient.print(message);
        if (mqttClient.endMessage())
        {
            Serial.println("Message published successfully");
            return true;
        }
        else
        {
            Serial.println("Error occurred while publishing the message");
            return false;
        }
    }
    else
    {
        Serial.println("Error occurred while beginning the message");
        return false;
    }
}

void printWiFiStatus()
{
    // print the SSID of the network you're attached to:
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print your WiFi shield's IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    // print where to go in a browser:
    Serial.print("To see this page in action, open a browser to http://");
    Serial.println(ip);
}
