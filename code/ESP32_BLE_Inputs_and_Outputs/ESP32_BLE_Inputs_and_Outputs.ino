/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/

// Include necessary libraries
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// DO NOT CHANGE THE NEXT UUIDs
// Otherwise, you also need to modify the Android application used in this project
#define SERVICE_UUID            "C6FBDD3C-7123-4C9E-86AB-005F1A7EDA01"
#define CHARACTERISTIC_UUID_RX  "B88E098B-E464-4B54-B827-79EB2B150A9F"
#define CHARACTERISTIC_UUID_TX  "D769FACF-A4DA-47BA-9253-65359EE480FB"

// Data wire is plugged into ESP32 GPIO
#define ONE_WIRE_BUS 27
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature
DallasTemperature sensors(&oneWire);

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// Temperature Sensor variable
float temperature = 0;
const int ledPin = 26;

// Setup callbacks onConnect and onDisconnect
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

// Setup callback when new value is received (from the Android application)
class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();
    if(rxValue.length() > 0) {
      Serial.print("Received value: ");
      for(int i = 0; i < rxValue.length(); i++) {
        Serial.print(rxValue[i]);
      }
      // Turn the LED ON or OFF according to the command received
      if(rxValue.find("ON") != -1) { 
        Serial.println(" - LED ON");
        digitalWrite(ledPin, HIGH);
      }
      else if(rxValue.find("OFF") != -1) {
        Serial.println(" - LED OFF");
        digitalWrite(ledPin, LOW);
      }
    }
  }
};

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  sensors.begin();
  
  // Create the BLE Device
  BLEDevice::init("ESP32_Board");

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY);
                      
  pCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE);

  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting to connect...");
}

void loop() {
  // When the device is connected
  if(deviceConnected) {
    // Measure temperature
    sensors.requestTemperatures();
    
    // Temperature in Celsius
    temperature = sensors.getTempCByIndex(0);
    // Uncomment the next line to set temperature in Fahrenheit 
    // (and comment the previous temperature line)
    //temperature = sensors.getTempFByIndex(0); // Temperature in Fahrenheit
    
    // Convert the value to a char array
    char txString[8];
    dtostrf(temperature, 1, 2, txString);
    
    // Set new characteristic value
    pCharacteristic->setValue(txString);
    // Send the value to the Android application
    pCharacteristic->notify(); 
    Serial.print("Sent value: ");
    Serial.println(txString);
  }
  delay(5000);
}
