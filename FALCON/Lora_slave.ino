#include <Arduino.h>
#include <SD.h>
#include <WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <LoRa.h>



#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"


// Define pins for Sensor
const int csPin = 5;
const int resetPin = 14;
const int irqPin = 2;
//I2C Address



// Wi-Fi Credentials
#define WIFI_SSID "Anil"
#define WIFI_PASSWORD "12344321"

// Firebase Credentials
#define API_KEY "AIzaSyA0NfagNvPBerGE_H-bfXt7UXcEI4yNUKw"
#define DATABASE_URL "https://falcon-9bb9e-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

// Create an instance of the Component


void setup() {
  Serial.begin(115200);
//Pin Mode Select


  // Initialize Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Initialize Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

// Initialize the lora module
  LoRa.setPins(csPin, resetPin, irqPin);
if (!LoRa.begin(868E6)) {
Serial.println("Starting LoRa failed!");
while (1);
}
Serial.println("LoRa Initializing OK!");
  

  // Initialize the TFT display
  
  // Set up oversampling and filter initialization
  
   if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase Auth OK");
    signupOK = true;
  } else {
    Serial.printf("Firebase Auth Error: %s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.println("Packet received!");

    String receivedData = "";
    while (LoRa.available()) {
      receivedData += (char)LoRa.read();
    }
    Serial.println("Received: " + receivedData);

    // Parse temperature and humidity (assuming format: "T:25.5,H:60.2")
    float temperature, humidity,moisture,pressure;
    sscanf(receivedData.c_str(), "T:%f,H:%f,P:%f,M:%f", &temperature, &humidity,&moisture,&pressure);


 
    if (Firebase.RTDB.setFloat(&fbdo, "Weather/Temperature", temperature)) {
      Serial.println("Temperature data sent to Firebase");
    }

    if (Firebase.RTDB.setFloat(&fbdo, "Weather/Humidity", humidity)) {
      Serial.println("Humidity data sent to Firebase");
    }
    
  if (Firebase.RTDB.setFloat(&fbdo, "Weather/pressure",pressure)) {
      Serial.println("pressure data sent to Firebase");
    }
    if (Firebase.RTDB.setFloat(&fbdo, "Weather/Moisture",moisture)) {
      Serial.println("pressure data sent to Firebase");
    }
if (Firebase.RTDB.setString(&fbdo, "Motion", "true")) {
      Serial.println("Temperature data sent to Firebase");
    }

    else {
      Serial.println("Firebase upload failed: " + fbdo.errorReason());
    }
    count++;
 
  // Delay before updating the values again
  delay(2000);
 }
}
