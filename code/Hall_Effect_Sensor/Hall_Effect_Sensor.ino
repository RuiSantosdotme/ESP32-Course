// Simple sketch to access the internal hall effect detector on the esp32.
// values can be quite low. 
// Brian Degger / @sctv  

int val = 0;

void setup() {
  Serial.begin(9600);
}

// put your main code here, to run repeatedly
void loop() {
  // read hall effect sensor value
  val = hallRead();
  // print the results to the serial monitor
  Serial.println(val); 
  delay(1000);
}
