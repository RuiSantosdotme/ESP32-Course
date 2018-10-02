/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/

#include "FS.h"
#include "SPIFFS.h"

// Create a File object to manipulate your file
File myFile;

// File path
const char* myFilePath = "/new_file.txt";

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  
  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("Error while mounting SPIFFS");
    return;
  }
  
  // Open file and write data to it
  myFile = SPIFFS.open(myFilePath, FILE_WRITE);
  if (myFile.print("Example message in write mode")){
    Serial.println("Message successfully written");  
  }
  else{
    Serial.print("Writting message failled!!");
  }
  myFile.close();

  // Append data to file
  myFile = SPIFFS.open(myFilePath, FILE_APPEND);
  if(myFile.print(" - Example message appended to file")){
    Serial.println("Message successfully appended");  
  }
  else{
    Serial.print("Appending failled!");  
  }
  myFile.close();

  // Read file content
  myFile = SPIFFS.open(myFilePath, FILE_READ);
  Serial.print("File content: \"");
  while(myFile.available()) {
    Serial.write(myFile.read());
  }
  Serial.println("\"");   

  // Check file size
  Serial.print("File size: ");
  Serial.println(myFile.size());

  myFile.close();
  
  // Delete file
  if(SPIFFS.remove(myFilePath)){
    Serial.println("File successfully deleted");
  }
  else{
    Serial.print("Deleting file failed!");
  }
}
void loop(){
  
}
