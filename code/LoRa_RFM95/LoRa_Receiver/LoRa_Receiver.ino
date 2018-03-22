/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

//define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 2

#define OLED_RESET 16
Adafruit_SSD1306 display(OLED_RESET);

void setup() { 
  //OLED display setup
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("LoRa Receiver");
  display.display();
  
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Receiver");
  
  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);

  //replace the LoRa.begin(---E-) argument with your location's frequency 
  //note: the frequency should match the sender's frequency
  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
  if (!LoRa.begin(866E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initializing OK!");
  display.setCursor(0,10);
  display.println("LoRa Initializing OK!");
  display.display();
}

void loop() {
  //try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    //received a packet
    Serial.print("Received packet ");
    display.clearDisplay();
    display.setCursor(0,0);
    display.print("Received packet ");
    display.display();

    //read packet
    while (LoRa.available()) {
      String LoRaData = LoRa.readString();
      Serial.print(LoRaData);
      display.setCursor(0,10);
      display.print(LoRaData);      
      display.display();
    }

    //print RSSI of packet
    int rrsi = LoRa.packetRssi();
    Serial.print(" with RSSI ");    
    Serial.println(rrsi);
    display.setCursor(0,20);
    display.print("RSSI: ");
    display.setCursor(30,20);    
    display.print(rrsi);
    display.display();
  }
}
