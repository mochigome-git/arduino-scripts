#include "WiFiS3.h"
#include <Wire.h>
#include <WiFiClient.h>
#include <ArduinoHttpClient.h>
#include "arduino_secrets.h"

// Define a constant for the pin number
const int limitSwitchPin = 13;
int state = LOW;

// Wi-Fi configuration
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
int status = WL_IDLE_STATUS;
const unsigned long interval = 3000;
unsigned long previousMillis = 0;
WiFiClient wifi;

// http configuration
char serverAddress[] = "192.100.0.25"; // server address
int port = 8000;
// Specify the complete URL path including the table
char *urlPath = "/rest/v1/nk3_log_data_realtime?id=eq.1";
const String HTTP_METHOD = "PATCH";
const String SERVICE_ROLE_KEY = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.ewogICAgInJvbGUiOiAic2VydmljZV9yb2xlIiwKICAgICJpc3MiOiAic3VwYWJhc2UiLAogICAgImlhdCI6IDE2NjYwMjI0MDAsCiAgICAiZXhwIjogMTgyMzc4ODgwMAp9.sbuBA2BnmzMP1CIMIyPWPEnAkGSnBUhFsOwcXEng5qg";
HttpClient client = HttpClient(wifi, serverAddress, port);

void setup()
{
    Serial.begin(9600);
    pinMode(limitSwitchPin, INPUT);
    // check for the WiFi module:
    if (WiFi.status() == WL_NO_MODULE)
    {
        Serial.println("Communication with WiFi module failed!");
        // don't continue
        while (true)
            ;
    }
    // attempt to connect to WiFi network:
    while (status != WL_CONNECTED)
    {
        Serial.print("Attempting to connect to WPA SSID: ");
        Serial.println(ssid);
        // Connect to WPA/WPA2 network:
        status = WiFi.begin(ssid, pass);
    }

    // you're connected now, so print out the data:
    Serial.print("You're connected to the network");
    printCurrentNet();
    printWifiData();
}

void loop()
{
    // Read the limit switch state
    int val = digitalRead(limitSwitchPin);
    // Only print if the state changes to HIGH (1)
    if (val == state)
    {
        makeHttpPostRequest("{\"dlot\": 1}");
        delay(1000);
        makeHttpPostRequest("{\"dlot\": 0}");
    }
}

void printWiFiStatus()
{
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
}

void makeHttpPostRequest(char *postData)
{
    // Additional parameters for the request
    const char *contentType = "application/json";

    Serial.println("Making POST request");

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

    // Record the start time
    unsigned long startTime = millis();
    // Set the timeout duration (in milliseconds)
    const unsigned long timeout = 5000;

    // Read the status code and body of the response
    while (!client.available() && (millis() - startTime) < timeout)
    {
        delay(10); // Wait for data or timeout
    }

    // Check if the request timed out
    if ((millis() - startTime) >= timeout)
    {
        Serial.println("HTTP POST request timed out");
        return;
    }

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

void printWifiData()
{
    // print your board's IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");

    Serial.println(ip);

    // print your MAC address:
    byte mac[6];
    WiFi.macAddress(mac);
    Serial.print("MAC address: ");
    printMacAddress(mac);
}

void printCurrentNet()
{
    // print the SSID of the network you're attached to:
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print the MAC address of the router you're attached to:
    byte bssid[6];
    WiFi.BSSID(bssid);
    Serial.print("BSSID: ");
    printMacAddress(bssid);

    // print the received signal strength:
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.println(rssi);

    // print the encryption type:
    byte encryption = WiFi.encryptionType();
    Serial.print("Encryption Type:");
    Serial.println(encryption, HEX);
    Serial.println();
}

void printMacAddress(byte mac[])
{
    for (int i = 5; i >= 0; i--)
    {
        if (mac[i] < 16)
        {
            Serial.print("0");
        }
        Serial.print(mac[i], HEX);
        if (i > 0)
        {
            Serial.print(":");
        }
    }
    Serial.println();
}
