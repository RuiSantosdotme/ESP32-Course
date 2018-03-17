/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "DHT.h"

//Default Temperature is in Celsius
//Comment the next line for Temperature in Fahrenheit
#define temperatureCelsius

//BLE server name
#define bleServerName "dhtESP32"

// Uncomment one of the lines below for whatever DHT sensor type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID "91bad492-b950-4226-aa2b-4ede9fa42f59"

#ifdef temperatureCelsius
  BLECharacteristic dhtTemperatureCelsiusCharacteristics("cba1d466-344c-4be3-ab3f-189f80dd7518", BLECharacteristic::PROPERTY_NOTIFY);
  BLEDescriptor dhtTemperatureCelsiusDescriptor(BLEUUID((uint16_t)0x2902));
#else
  BLECharacteristic dhtTemperatureFahrenheitCharacteristics("f78ebbff-c8b7-4107-93de-889a6a06d408", BLECharacteristic::PROPERTY_NOTIFY);
  BLEDescriptor dhtTemperatureFahrenheitDescriptor(BLEUUID((uint16_t)0x2901));
#endif

BLECharacteristic dhtHumidityCharacteristics("ca73b3ba-39f6-4ab3-91ae-186dc9577d99", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor dhtHumidityDescriptor(BLEUUID((uint16_t)0x2903));

// DHT Sensor
const int DHTPin = 14;

// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

bool deviceConnected = false;

//Setup callbacks onConnect and onDisconnect
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
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

  // Create BLE Characteristics and Create a BLE Descriptor
  // Descriptor bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml       

  #ifdef temperatureCelsius
    dhtService->addCharacteristic(&dhtTemperatureCelsiusCharacteristics);
    dhtTemperatureCelsiusDescriptor.setValue("DHT temperature Celsius");
    dhtTemperatureCelsiusCharacteristics.addDescriptor(new BLE2902());
  #else
    dhtService->addCharacteristic(&dhtTemperatureFahrenheitCharacteristics);
    dhtTemperatureFahrenheitDescriptor.setValue("DHT temperature Fahrenheit");
    dhtTemperatureFahrenheitCharacteristics.addDescriptor(new BLE2902());
  #endif  
  
  dhtService->addCharacteristic(&dhtHumidityCharacteristics);
  dhtHumidityDescriptor.setValue("DHT humidity");
  dhtHumidityCharacteristics.addDescriptor(new BLE2902());
  
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
    #ifdef temperatureCelsius
      static char temperatureCTemp[7];
      dtostrf(t, 6, 2, temperatureCTemp);
      //Set temperature Characteristic value and notify connected client
      dhtTemperatureCelsiusCharacteristics.setValue(temperatureCTemp);
      dhtTemperatureCelsiusCharacteristics.notify();
      Serial.print("Temperature Celsius: ");
      Serial.print(t);
      Serial.print(" *C");
    #else
      static char temperatureFTemp[7];
      dtostrf(f, 6, 2, temperatureFTemp);
      //Set temperature Characteristic value and notify connected client
      dhtTemperatureFahrenheitCharacteristics.setValue(temperatureFTemp);
      dhtTemperatureFahrenheitCharacteristics.notify();
      Serial.print("Temperature Fahrenheit: ");
      Serial.print(f);
      Serial.print(" *F");
    #endif
    
    //Notify humidity reading from DHT
    static char humidityTemp[7];
    dtostrf(h, 6, 2, humidityTemp);
    //Set humidity Characteristic value and notify connected client
    dhtHumidityCharacteristics.setValue(humidityTemp);
    dhtHumidityCharacteristics.notify();   
    Serial.print(" - Humidity: ");
    Serial.print(h);
    Serial.println(" %");
    
    delay(10000);
  }
}
