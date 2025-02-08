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
#include <LoRa.h>




// Define pins for Sensor
#define TFT_CS     5
#define TFT_RST    16
#define TFT_DC     17
#define DHT_PIN   15        // DHT11 sensor pin
#define PIRpin      2 
#define moisture   34
int PIRvalue =0;
const int csPin = 4;
const int resetPin = 14;
const int irqPin = 12;
// Sensor Types
#define DHT_TYPE DHT11   // DHT sensor type (DHT11)
Adafruit_BMP3XX bmp;

//I2C Address
#define SEALEVELPRESSURE_HPA (1013.25)

// Create an instance of the Component
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
  Serial.begin(115200);
//Pin Mode Select
pinMode(PIRpin, INPUT);

LoRa.setPins(csPin, resetPin, irqPin);
if (!LoRa.begin(868E6)) {
Serial.println("Starting LoRa failed!");
while (1);
}
Serial.println("LoRa Initializing OK!");
  
// Initialize the BMP388 sensor
  bmp.begin_I2C();
  dht.begin();

  // Initialize the TFT display
  tft.initR(INITR_144GREENTAB); // Initialize with green tab
  tft.setRotation(3);          // Rotate for landscape view
  tft.fillScreen(ST77XX_BLACK); // Clear the screen

  // Set up oversampling and filter initialization
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);

   
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
  //motion detect code
PIRvalue = digitalRead(PIRpin);  
  
  
  //Moisture detect code
  int read=analogRead(moisture);
  int value_moisture=constrain(read,0,100);

  // Clear the screen before updating
  tft.fillScreen(ST77XX_BLACK);

  // Display header
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.setCursor(5, 5);
  tft.println("Irrigation");

  // Display temperature
  tft.setTextColor(ST77XX_RED);
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
  tft.print("Soil Moistue: ");
  tft.print(value_moisture);
  tft.println(" %");

  // Display pump status
  tft.setCursor(10, 90);
  tft.print("Pump: ");
  tft.println("ON");

  // Print mock values to the serial monitor (optional)
  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print(" C, Humidity: ");
  Serial.print(humidity);
  Serial.print(" %, Soil Moisture: ");
  Serial.print(value_moisture);
  Serial.print(" %, Pump: ");
  Serial.println("");

  String datasend="T:"+String(temperature,1)+"H:"+String(humidity)+"P:"+String(value)+"M:"+String(value_moisture);
LoRa.beginPacket();
LoRa.print(datasend);
LoRa.endPacket();
Serial.println("Data send.");
  delay(2000);
}

