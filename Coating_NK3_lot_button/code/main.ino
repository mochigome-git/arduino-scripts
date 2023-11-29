#include "WiFiS3.h"
#include <Wire.h>
#include <WiFiClient.h>
#include <ArduinoHttpClient.h>
#include "arduino_secrets.h"

// Wi-Fi and MQTT credentials
char ssid[] = SECRET_SSID; // Network SSID (name)
char pass[] = SECRET_PASS; // Network password
int keyIndex = 0;          // Network key index number (needed only for WEP)

int limitSwitch = 13;
int state = LOW;

char serverAddress[] = "192.168.0.126"; // server address
int port = 8000;
// Specify the complete URL path including the table
char *urlPath = "/rest/v1/test";
String HTTP_METHOD = "POST";
String SERVICE_ROLE_KEY = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.ewogICAgInJvbGUiOiAic2VydmljZV9yb2xlIiwKICAgICJpc3MiOiAic3VwYWJhc2UiLAogICAgImlhdCI6IDE2NjYwMjI0MDAsCiAgICAiZXhwIjogMTgyMzc4ODgwMAp9.sbuBA2BnmzMP1CIMIyPWPEnAkGSnBUhFsOwcXEng5qg";

char *postData = "{\"test\": true}";

// Time interval for connection attempts
const unsigned long interval = 8000;
unsigned long previousMillis = 0;

// Create an HTTP client object
WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);
int status = WL_IDLE_STATUS;

void setup()
{
    Serial.begin(9600);
    pinMode(limitSwitch, INPUT);

    // Connect to Wi-Fi
    connectToWiFi();
}

void loop()
{
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

            // Make an HTTP POST request when the limit switch state changes
            if (state == 1)
            {
                makeHttpPostRequest();
                delay(100);
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

void makeHttpPostRequest()
{
    // Additional parameters for the request
    const char *contentType = "application/json";

    Serial.println("making POST request");

    // Make the HTTP POST request
    client.beginRequest();
    client.post("/");
    // Start the request with the full URL
    client.startRequest(urlPath, HTTP_METHOD.c_str(), contentType);
    client.sendHeader("Content-Type", contentType);
    client.sendHeader("Content-Length", strlen(postData));
    client.sendHeader("apikey", SERVICE_ROLE_KEY);
    client.sendHeader("Authorization", "Bearer " + SERVICE_ROLE_KEY);

    // Begin the request body
    client.beginBody();
    // Send the POST data using print
    client.print(postData);
    // End the request
    client.endRequest();

    // Read the status code and body of the response
    int statusCode = client.responseStatusCode();
    String response = client.responseBody();

    // Handle the response
    Serial.print("Status code: ");
    Serial.println(statusCode);
    Serial.print("Response: ");
    Serial.println(response);

    if (statusCode == 201)
    {
        Serial.println("HTTP Post successfully");
    }
    else if (statusCode > 0)
    {
        Serial.print("HTTP Response code: ");
        Serial.println(statusCode);
    }
    else
    {
        Serial.println("HTTP POST request failed");
    }
}
