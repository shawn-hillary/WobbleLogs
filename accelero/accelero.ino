#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>
Adafruit_MMA8451 mma = Adafruit_MMA8451();
void setup(void) {
  Serial.begin(9600); 
  Serial.println("Adafruit MMA8451 test!");
  if (! mma.begin()) {
    Serial.println("Couldnt start");
    while (1);
  }
  Serial.println("MMA8451 found!");
  mma.setRange(MMA8451_RANGE_2_G);
  Serial.print("Range = "); Serial.print(2 << mma.getRange());  
  Serial.println("G");
}
void loop() {
  // Read the 'raw' data in 14-bit counts
  mma.read(); 
  sensors_event_t event; 
  mma.getEvent(&event);
  double x = event.acceleration.x;
  double y = event.acceleration.y;
  double z = event.acceleration.z;  
  double wigglepower = (sqrt(x*x + y*y + z*z)-9.81);
  Serial.print(wigglepower);
  Serial.print("\t");
  uint8_t o = mma.getOrientation();
  Serial.println();
  delay(5); 
}
