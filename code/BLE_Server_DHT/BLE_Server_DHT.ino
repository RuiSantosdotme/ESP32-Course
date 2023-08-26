/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "DHT.h"

//BLE server name
#define bleServerName "ESP32_DHT"

// Uncomment one of the lines below for whatever DHT sensor type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// Default UUID for Environmental Sensing Service
// https://www.bluetooth.com/specifications/assigned-numbers/
#define SERVICE_UUID (BLEUUID((uint16_t)0x181A))

// Temperature Characteristic and Descriptor (default UUID)
// Check the default UUIDs here: https://www.bluetooth.com/specifications/assigned-numbers/
BLECharacteristic dhtTemperatureCharacteristic(BLEUUID((uint16_t)0x2A6E), BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor dhtTemperatureDescriptor(BLEUUID((uint16_t)0x2902));

// Humidity Characteristic and Descriptor (default UUID)
BLECharacteristic dhtHumidityCharacteristic(BLEUUID((uint16_t)0x2A6F), BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor dhtHumidityDescriptor(BLEUUID((uint16_t)0x2902));

// DHT Sensor
const int DHTPin = 14;

// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

bool deviceConnected = false;

//Setup callbacks onConnect and onDisconnect
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Device Connected");
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Device Disconnected");
  }
};

void setup() {
  // Start DHT sensor
  dht.begin();

  // Start serial communication 
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init(bleServerName);

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *dhtService = pServer->createService(SERVICE_UUID);

  // Create BLE Characteristics and corresponding Descriptors
  dhtService->addCharacteristic(&dhtTemperatureCharacteristic);
  dhtTemperatureCharacteristic.addDescriptor(&dhtTemperatureDescriptor);
  
  dhtService->addCharacteristic(&dhtHumidityCharacteristic);
  dhtHumidityCharacteristic.addDescriptor(&dhtHumidityDescriptor);
  
  // Start the service
  dhtService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  if (deviceConnected) {
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht.readTemperature(true);
    // Read humidity
    float h = dht.readHumidity();

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    
    //Notify temperature reading from DHT sensor
    uint16_t temperatureCTemp = (uint16_t)t;
    //Set temperature Characteristic value and notify connected client
    dhtTemperatureCharacteristic.setValue(temperatureCTemp);
    dhtTemperatureCharacteristic.notify();
    Serial.print("Temperature Celsius: ");
    Serial.print(t);
    Serial.print(" *C");
   
    
    //Notify humidity reading from DHT
    uint16_t humidityTemp = (uint16_t)h;
    //Set humidity Characteristic value and notify connected client
    dhtHumidityCharacteristic.setValue(humidityTemp);
    dhtHumidityCharacteristic.notify();   
    Serial.print(" - Humidity: ");
    Serial.print(h);
    Serial.println(" %");
    
    delay(10000);
  }
}
