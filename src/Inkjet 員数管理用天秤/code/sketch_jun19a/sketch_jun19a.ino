#include <SoftwareSerial.h>

SoftwareSerial myScale(0, 1); // RX, TX

void setup() {
  // Open serial communications and wait for the port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for the serial port to connect. Needed for native USB port only
  }


  // set the data rate for the SoftwareSerial port
  myScale.begin(9600);
}

void loop() { // run over and over
  if (myScale.available()) {
    Serial.write(myScale.read()& 0x7F);
  }  
}