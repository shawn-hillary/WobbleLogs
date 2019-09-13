/*********
  Modified from the examples of the Arduino LoRa library
  More resources: https://randomnerdtutorials.com
*********/

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>
Adafruit_MMA8451 mma = Adafruit_MMA8451();

//define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 2
int counter = 0;
unsigned long t;

void setup() {
  
  //initialize Accelerometer
  if (! mma.begin()) {
    Serial.println("Couldnt start Accelerometer");
    while (1);
  }
  Serial.println("MMA8451 found!");
  mma.setRange(MMA8451_RANGE_2_G);
  Serial.print("Range = "); Serial.print(2 << mma.getRange());  
  Serial.println("G");
  mma.setDataRate(MMA8451_DATARATE_800_HZ);
  
  //initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Sender");

  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  
  //replace the LoRa.begin(---E-) argument with your location's frequency 
  //866E6 for Europe
  while (!LoRa.begin(866E6)) {
    Serial.println(".");
    delay(500);
  }
   // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
}

void loop() {
  //Read Accelerometer
  t = millis();
  mma.read();
  sensors_event_t event; 
  mma.getEvent(&event);
  double x = event.acceleration.x;
  double y = event.acceleration.y;
  double z = event.acceleration.z;  
  double wigglepower = (sqrt(x*x + y*y + z*z));
  //Send Accelerometer Data  
  Serial.println("Sending packet: ");
  Serial.println(wigglepower);
  //Send LoRa packet to receiver
  LoRa.beginPacket();
  LoRa.print(x);
  LoRa.print("  ");
  LoRa.print(y);
  LoRa.print("  ");
  LoRa.print(z);
  LoRa.print("  time = ");
  LoRa.print(t);
  LoRa.print("  count = ");
  LoRa.print(counter);
  LoRa.endPacket();
  counter++;
}
