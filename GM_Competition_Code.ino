#include <DHTesp.h>
/*BEWARE, DO NOT CHANGE ANYTHING IN THIS FILE, 
 * ONLY THE INDICATED LINES.
*/

// Including libraries for DHT and ESP8266 microcontroller
#include <ESP8266WiFi.h>
#include "DHT.h"
#include <stdio.h>
#define DHTTYPE DHT11   // DHT 11


//Water Sensor Libraries
#include <RBD_WaterSensor.h>
#include <RBD_Threshold.h>
#include <RBD_Capacitance.h>
#include <dummy.h>
#include <EEPROM.h>
#define SensorPin A0 

//CHANGE THE SSID WITH YOUR WIFI NAME(" ")
const char* ssid = "NETGEAR46"; 
//CHANGE THE PASSWORD WITH YOUR PASSWORD(" ")
const char* password = "unevenchair682";

// web server activates at port 80
WiFiServer server(80);

//Defining DHT sensor pin
const int DHTPin = 5;  //Comunicación de datos en el pin 5 (GPIO 5 -- D1)
// Inicianting sensor
DHT dht(DHTPin, DHTTYPE);

// Temporal variables
static char celsiusTemp[7];
static char fahrenheitTemp[7];
static char humidityTemp[7];
static char kelvinTemp[7];
static char waterLevel[7];

//This just executes at the beginning
void setup() {
  // Initializing and configuring serial communication speed
  Serial.begin(115200);
  delay(10);

  dht.begin();
  
  pinMode(4, OUTPUT);
  pinMode(0, INPUT);



  // Connecting to Wifi
  Serial.println();
  Serial.print("Connecting to:  ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Wi-Fi connected");

  // Starting the web server
  server.begin();
  Serial.println("Web server executing. Waiting for the ESP IP...");
  delay(10000);

  // Shows ESP IP in the Serial Monitor
  Serial.println(WiFi.localIP());
}

// This will repeat continuously
void loop() {
  //Waiting new connections(clients)
  WiFiClient client = server.available();

  if (client) {
    Serial.println("New Client");
    // Boolean to localize the end of HTTP request
    boolean blank_line = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
  
        if (c == '\n' && blank_line) {
          float h = dht.readHumidity();
          // Read temperature in Celsius (defaut measure unit in this sensor)
          float t = dht.readTemperature();
          // Reading temperature in Fahrenheit (If is "true" it will read)
          float f = dht.readTemperature(true);
          // Checking for failures in the reading
          if (isnan(h) || isnan(t) || isnan(f)) {
            Serial.println("Failing to read from the DHT sensor");
            strcpy(celsiusTemp, "Failed");
            strcpy(fahrenheitTemp, "Failed");
            strcpy(humidityTemp, "Failed");
          }
          else {
            // Calculating temeprature, humidity and water level
            float hic = dht.computeHeatIndex(t, h, false);
            dtostrf(hic, 6, 2, celsiusTemp);
            float hif = dht.computeHeatIndex(f, h);
            dtostrf(hif, 6, 2, fahrenheitTemp);
            dtostrf(h, 6, 2, humidityTemp);
            float K = 273.15+hic;
            dtostrf(K, 6, 2, kelvinTemp);

            digitalWrite(4, HIGH);
            delay(1000);
            float sensorValue = analogRead(SensorPin);
            dtostrf(sensorValue, 6, 2, waterLevel);
//            Serial.println(sensorValue);
            delay(1000);
            digitalWrite(4, LOW);
            delay(1000);
            // You can eliminate those lines with Serial.print() is just for debugging
//            Serial.print("Humidity: ");
//            Serial.print(h);
//            Serial.print(" %\t Temperature: ");
//            Serial.print(t);
//            Serial.print(" *C ");
//            Serial.print(f);
//            Serial.print(" *F\t Heat index: ");
//            Serial.print(hic);
//            Serial.print(" *C ");
//            Serial.print(hif);
//            Serial.print(" *F");
//            Serial.print("Humidity: ");
//            Serial.print(h);
//            Serial.print(" %\t Temperature: ");
//            Serial.print(t);
//            Serial.print(" *C ");
//            Serial.print(f);
//            Serial.print(" *F\t Heat index: ");
//            Serial.print(hic);
//            Serial.print(" *C ");
//            Serial.print(hif);
//            Serial.println(" *F");
          }
          //HTML Page Layout
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();
          // Showing temperature and water sensor values
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<head></head><body><h1>MicroWeather</h1><h3>Temperature in Celsius: ");
          client.println(celsiusTemp);
          client.println("*C</h3><h3>Temperature in Fahrenheit: ");
          client.println(fahrenheitTemp);
          client.println("*F</h3><h3>Temperature in Kelvin: ");
          client.println(kelvinTemp);
          client.println("*K</h3><h3>Humidity: ");
          client.println(humidityTemp);
          client.println("</h3><h3>Water Level: ");
          client.println(waterLevel);
          client.println("</body>");
          client.print("<meta http-equiv=\"refresh\" content=\"1\">");  //Actualización de la página cada segundo
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // When starts to read new line
          blank_line = true;
        }
        else if (c != '\r') {
          // When finds a character in actual line
          blank_line = false;
        }
      }
    }
    // Closing connection with the client
    delay(1);
    client.stop();
    Serial.println("Client disconnected.");
  }
}
