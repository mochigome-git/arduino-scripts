
#include "WiFiS3.h"
#include <Wire.h>
#include <WiFiClient.h>
#include "arduino_secrets.h"

// Wi-Fi and MQTT credentials
char ssid[] = SECRET_SSID; // Network SSID (name)
char pass[] = SECRET_PASS; // Network password
int keyIndex = 0;          // Network key index number (needed only for WEP)

int limitSwitch = 13;
int state = LOW;

String HOST_NAME = "192.168.0.126";
int HTTP_PORT = 8000;
String HTTP_METHOD = "POST";
String SERVICE_ROLE_KEY = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.ewogICAgInJvbGUiOiAic2VydmljZV9yb2xlIiwKICAgICJpc3MiOiAic3VwYWJhc2UiLAogICAgImlhdCI6IDE2NjYwMjI0MDAsCiAgICAiZXhwIjogMTgyMzc4ODgwMAp9.sbuBA2BnmzMP1CIMIyPWPEnAkGSnBUhFsOwcXEng5qg";
String PATH_NAME = "/rest/v1/";
String queryString = "?test=true";

// Time interval for connection attempts
const unsigned long interval = 8000;
unsigned long previousMillis = 0;

WiFiClient client;

void setup()
{
    Serial.begin(9600);
    pinMode(limitSwitch, INPUT);

    // Connect to Wi-Fi
    connectToWiFi();
}

void loop()
{
    unsigned long currentMillis = millis();
    // Resolve the host name to an IP address
    IPAddress serverIP;

    // Check if the Wi-Fi connection is lost
    if (WiFi.status() != WL_CONNECTED)
    {
        // Attempt to reconnect to Wi-Fi
        connectToWiFi();
    }
    else
    {
        int val = digitalRead(limitSwitch);

        if (val != state)
        {
            state = val;
            Serial.print("Sensor value = ");

            // make a POST request when the limit switch state changes
            if (state)
            {
                if (WiFi.hostByName(HOST_NAME.c_str(), serverIP))
                {
                    // Connect to the server
                    if (client.connect(serverIP, HTTP_PORT))
                    {
                        Serial.println("Connected to server");

                        // send HTTP header
                        client.print(HTTP_METHOD + " " + PATH_NAME + queryString + " HTTP/1.1\r\n");
                        client.print("Host: " + HOST_NAME + "\r\n");
                        client.print("Connection: close\r\n");
                        client.print("apikey: " + SERVICE_ROLE_KEY + "\r\n");
                        client.print("Authorization: Bearer " + SERVICE_ROLE_KEY + "\r\n");
                        client.print("Content-Type: application/json\r\n");
                        client.print("Content-Length: 0\r\n");
                        client.print("\r\n"); // end HTTP header

                        Serial.println("POST request sent");

                        // Wait for the server to process the request
                        delay(1000);

                        // the server's disconnected, stop the client:
                        client.stop();
                        Serial.println("Disconnected from server");
                    }
                    else
                    {
                        Serial.println("Connection to server failed");
                    }
                }
                else
                {
                    Serial.println("Failed to resolve host");
                }
            }
        }
    }
}

void connectToWiFi()
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
                Serial.println("Connected to the network");
            }
            else
            {
                Serial.println("Failed to connect to the network");
            }
        }
    }
}