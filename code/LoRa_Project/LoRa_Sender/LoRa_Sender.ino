/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/

// Libraries for LoRa Module
#include <SPI.h>            
#include <LoRa.h>

//DS18B20 libraries
#include <OneWire.h>
#include <DallasTemperature.h>

// LoRa Module pin definition
// define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 2

// LoRa message variable
String message;

// Save reading number on RTC memory
RTC_DATA_ATTR int readingID = 0;

// Define deep sleep options
uint64_t uS_TO_S_FACTOR = 1000000;  // Conversion factor for micro seconds to seconds
// Sleep for 30 minutes = 0.5 hours  = 1800 seconds
uint64_t TIME_TO_SLEEP = 1800;

// Data wire is connected to ESP32 GPIO15
#define ONE_WIRE_BUS 15
// Setup a oneWire instance to communicate with a OneWire device
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

// Moisture Sensor variables
const int moisturePin = 26;
const int moisturePowerPin = 12;
int soilMoisture;

// Temperature Sensor variables
float tempC;
float tempF;

//Variable to hold battery level;
float batteryLevel;
const int batteryPin = 27;

void setup() {
  pinMode(moisturePowerPin, OUTPUT);
  
  // Start serial communication for debugging purposes
  Serial.begin(115200);

  // Enable Timer wake_up
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  // Start the DallasTemperature library
  sensors.begin();
  
  // Initialize LoRa
  //replace the LoRa.begin(---E-) argument with your location's frequency 
  //note: the frequency should match the sender's frequency
  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
  LoRa.setPins(ss, rst, dio0);
  Serial.println("initializing LoRa");
  
  int counter = 0;  
  while (!LoRa.begin(866E6) && counter < 10) {
    Serial.print(".");
    counter++;
    delay(500);
  }
  if (counter == 10) {
    // Increment readingID on every new reading
    readingID++; 
    // Start deep sleep
    Serial.println("Failed to initialize LoRa. Going to sleep now");
    esp_deep_sleep_start();
  }
  // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa initializing OK!");

  getReadings();
  Serial.print("Battery level = "); 
  Serial.println(batteryLevel, 2);
  Serial.print("Soil moisture = ");
  Serial.println(soilMoisture);
  Serial.print("Temperature Celsius = ");
  Serial.println(tempC);
  Serial.print("Temperature Fahrenheit = ");
  Serial.println(tempF);
  Serial.print("Reading ID = ");
  Serial.println(readingID);
  
  sendReadings();
  Serial.print("Message sent = ");
  Serial.println(message);
  
  // Increment readingID on every new reading
  readingID++;
  
  // Start deep sleep
  Serial.println("DONE! Going to sleep now.");
  esp_deep_sleep_start(); 
} 

void loop() {
  // The ESP32 will be in deep sleep
  // it never reaches the loop()
}

void getReadings() {
  digitalWrite(moisturePowerPin, HIGH);
   
  // Measure temperature
  sensors.requestTemperatures(); 
  tempC = sensors.getTempCByIndex(0); // Temperature in Celsius
  tempF = sensors.getTempFByIndex(0); // Temperature in Fahrenheit
   
  // Measure moisture
  soilMoisture = analogRead(moisturePin);
  digitalWrite(moisturePowerPin, LOW);

  //Measure battery level
  batteryLevel = map(analogRead(batteryPin), 0.0f, 4095.0f, 0, 100);
}

void sendReadings() {
  // Send packet data
  // Send temperature in Celsius
  message = String(readingID) + "/" + String(tempC) + "&" + 
            String(soilMoisture) + "#" + String(batteryLevel);
  // Uncomment to send temperature in Fahrenheit
  //message = String(readingID) + "/" + String(tempF) + "&" + 
  //          String(soilMoisture) + "#" + String(batteryLevel);
  delay(1000);
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();
}
