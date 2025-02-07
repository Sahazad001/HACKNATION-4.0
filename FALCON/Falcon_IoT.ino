#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Arduino.h>
#include <SD.h>
#include <WiFi.h>
#include <DHT.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP3XX.h>


#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"


// Define pins for Sensor
#define TFT_CS     5
#define TFT_RST    16
#define TFT_DC     17
#define DHT_PIN     4        // DHT11 sensor pin
#define PIRpin      2 
#define PIRvalue    0
// Sensor Types
#define DHT_TYPE DHT11   // DHT sensor type (DHT11)
Adafruit_BMP3XX bmp;

//I2C Address
#define SEALEVELPRESSURE_HPA (1013.25)


// Wi-Fi Credentials
#define WIFI_SSID "Anil"
#define WIFI_PASSWORD "12344321"

// Firebase Credentials
#define API_KEY "AIzaSyCFRUM92_4FSwO_FwWEMP2Pikj9FP6QKS4"
#define DATABASE_URL "https://test-2144f-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

// Create an instance of the Component
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
  Serial.begin(115200);
//Pin Mode Select
pinMode(PIRpin, INPUT);

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

// Initialize the BMP388 sensor
  bmp.begin_I2C();

  // Initialize the TFT display
  tft.initR(INITR_144GREENTAB); // Initialize with green tab
  tft.setRotation(3);          // Rotate for landscape view
  tft.fillScreen(ST77XX_BLACK); // Clear the screen

  // Set up oversampling and filter initialization
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);

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

   if (! bmp.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }
  float value=(bmp.pressure/100.00 )*-1;
  Serial.print("Pressure = ");
  Serial.print(value);
  Serial.println(" hPa");


float temperature = dht.readTemperature(); // Temperature in Celsius
float humidity = dht.readHumidity();       // Humidity in percentage


  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
 int PIRvalue = digitalRead(PIRpin);  
  
  
  // Clear the screen before updating
  tft.fillScreen(ST77XX_BLACK);

  // Display header
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(2);
  tft.setCursor(5, 5);
  tft.println("Irrigation");

  // Display temperature
  tft.setTextSize(1);
  tft.setCursor(10, 30);
  tft.print("Temp: ");
  tft.print(temperature, 1); // One decimal place
  tft.println(" C");

  // Display humidity
  tft.setCursor(10, 50);
  tft.print("Humidity: ");
  tft.print(humidity, 1); // One decimal place
  tft.println(" %");

  // Display soil moisture
  tft.setCursor(10, 70);
  tft.print("Soil: ");
  tft.print(10);
  tft.println(" %");

  // Display pump status
  tft.setCursor(10, 90);
  tft.print("Pump: ");
  tft.println("ON");

  // Print mock values to the serial monitor (optional)
  Serial.print("Temp: ");
  Serial.print("");
  Serial.print(" C, Humidity: ");
  Serial.print(25.7);
  Serial.print(" %, Soil: ");
  Serial.print("");
  Serial.print(" %, Pump: ");
  Serial.println("");

 if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    if (Firebase.RTDB.setFloat(&fbdo, "Weather/sensor/Temperature", temperature)) {
      Serial.println("Temperature data sent to Firebase");
    }

    if (Firebase.RTDB.setFloat(&fbdo, "Weather/sensor/Humidity", humidity)) {
      Serial.println("Humidity data sent to Firebase");
    }
    if (Firebase.RTDB.setFloat(&fbdo, "Weather/sensor/Humidity", humidity)) {
      Serial.println("Humidity data sent to Firebase");
    }

    else {
      Serial.println("Firebase upload failed: " + fbdo.errorReason());
    }
    count++;
 }
  // Delay before updating the values again
  delay(2000);
}

