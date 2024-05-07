// get GPS wave

#include<SoftwareSerial.h>

SoftwareSerial GPS(14,15);  //RX,TX

void setup() {
  GPS.begin(9600);
  Serial.begin(9600);
}

void loop() {
  if (GPS.available()){
    Serial.write(GPS.read());
  }
}
