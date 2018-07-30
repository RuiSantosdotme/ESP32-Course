/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/

#include <WiFi.h>
extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}
#include <AsyncMqttClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Change the credentials below, so your ESP32 connects to your router
#define WIFI_SSID "REPLACE_WITH_YOUR_SSID"
#define WIFI_PASSWORD "REPLACE_WITH_YOUR_PASSWORD"

// Change the MQTT_HOST variable to your Raspberry Pi IP address, 
// so it connects to your Mosquitto MQTT broker
#define MQTT_HOST IPAddress(192, 168, 1, X)
#define MQTT_PORT 1883

// Create objects to handle MQTT client
AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

unsigned long previousMillis = 0;    // Stores last time temperature was published
const long interval = 10000;         // interval at which to publish sensor readings

const int ledPin = 25;              // GPIO where the LED is connected to
int ledState = LOW;                 // the current state of the output pin

// GPIO where the DS18B20 is connected to
const int oneWireBus = 27;          
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

const int buttonPin = 32;            // Define GPIO where the pushbutton is connected
int buttonState;                     // current reading from the input pin (pushbutton)
int lastButtonState = LOW;           // previous reading from the input pin (pushbutton)
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flicker

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      connectToMqtt();
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
      xTimerStart(wifiReconnectTimer, 0);
      break;
  }
}

// Add more topics that want your ESP32 to be subscribed to
void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  // ESP32 subscribed to esp32/led topic
  uint16_t packetIdSub = mqttClient.subscribe("esp32/led", 0);
  Serial.print("Subscribing at QoS 0, packetId: ");
  Serial.println(packetIdSub);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

// You can modify this function to handle what happens when you receive a certain message in a specific topic
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  String messageTemp;
  for (int i = 0; i < len; i++) {
    //Serial.print((char)payload[i]);
    messageTemp += (char)payload[i];
  }
  // Check if the MQTT message was received on topic esp32/led
  if (strcmp(topic, "esp32/led") == 0) {
    // If the LED is off turn it on (and vice-versa)
    if (messageTemp == "on") {
      digitalWrite(ledPin, HIGH);
    } 
    else if (messageTemp == "off") {
      digitalWrite(ledPin, LOW);
    }
  }
 
  Serial.println("Publish received.");
  Serial.print("  message: ");
  Serial.println(messageTemp);
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
}

void setup() {
  // Start the DS18B20 sensor
  sensors.begin();
  
  // Define LED as an OUTPUT and set it LOW
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
  // Define buttonPin as an INPUT
  pinMode(buttonPin, INPUT);
  
  Serial.begin(115200);

  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  WiFi.onEvent(WiFiEvent);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  connectToWifi();
}

void loop() {
  unsigned long currentMillis = millis();
  // Every X number of seconds (interval = 5 seconds) 
  // it publishes a new MQTT message on topic esp32/temperature
  if (currentMillis - previousMillis >= interval) {
    // Save the last time a new reading was published
    previousMillis = currentMillis;
    // New temperature readings
    sensors.requestTemperatures(); 

    // Publish an MQTT message on topic esp32/temperature with Celsius degrees
    uint16_t packetIdPub2 = mqttClient.publish("esp32/temperature", 2, true, 
                            String(sensors.getTempCByIndex(0)).c_str());
                            
    // Publish an MQTT message on topic esp32/temperature with Fahrenheit degrees
    //uint16_t packetIdPub2 = mqttClient.publish("esp32/temperature", 2, true, 
    //                        String(sensors.getTempFByIndex(0)).c_str());
                            
    Serial.print("Publishing on topic esp32/temperature at QoS 2, packetId: ");
    Serial.println(packetIdPub2);
  }

  // Read the state of the pushbutton and save it in a local variable
  int reading = digitalRead(buttonPin);

  // If the pushbutton state changed (due to noise or pressing it), reset the timer
  if (reading != lastButtonState) {
    // Reset the debouncing timer
    lastDebounceTime = millis();
  }
  
  // If the button state has changed, after the debounce time
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // And if the current reading is different than the current buttonState    
    if (reading != buttonState) {
      buttonState = reading;
      // Publish an MQTT message on topic esp32/led/toggle to toggle the LED (turn the LED on or off)
      if ((buttonState == HIGH)) {
        if (!digitalRead(ledPin)) {
          mqttClient.publish("esp32/led/toggle", 0, true, "on");
          Serial.println("Publishing on topic esp32/led/toggle topic at QoS 0");
        }
        else if (digitalRead(ledPin)) {
          mqttClient.publish("esp32/led/toggle", 0, true, "off");
          Serial.println("Publishing on topic esp32/led/toggle topic at QoS 0");
        }
      }
    }
  }
  // Save the reading. Next time through the loop, it'll be the lastButtonState
  lastButtonState = reading;
}
