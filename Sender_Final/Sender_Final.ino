#include <SPI.h>              // include libraries
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>
#include <stdio.h>
#include <stdlib.h>

#define n 10

const int csPin = 5;          // LoRa radio chip select
const int resetPin = 25;       // LoRa radio reset
const int irqPin = 2;         // change for your board; must be a hardware interrupt pin
String outgoing;              // outgoing message
byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0xBB;     // address of this device
byte destination = 0xFF;      // destination to send to
long lastSendTime = 0;        // last send time
Adafruit_MMA8451 mma = Adafruit_MMA8451();
RTC_DATA_ATTR int timeDiff = 0;
RTC_DATA_ATTR int timeDiff2 = 0;
int16_t X[n];
int16_t Y[n];
int16_t Z[n];
uint16_t set = 0;
long T[n];
char Xmsb[n];
char Xlsb[n];
char Ymsb[n];
char Ylsb[n];
char Zmsb[n];
char Zlsb[n];
char t1[n];
char t2[n];
char t3[n];
char t4[n];
bool logged = 1;

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return
  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length
  String incoming = "";
  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }
  if (incomingLength != incoming.length()) {   // check length for error
    Serial.println("error: message length does not match length");
    return;                             // skip rest of function
  }
  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return;                             // skip rest of function
  }
  // if message is for this device, or broadcast, print details:
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Message ID: " + String(incomingMsgId));
  Serial.println("Message length: " + String(incomingLength));
  Serial.println("Message: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
}
void setup() {
  esp_sleep_enable_timer_wakeup(1e6);
  Serial.begin(115200);                   // initialize serial
  while (!Serial);
  Serial.println("LoRa Duplex");
  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(csPin, resetPin, irqPin);// set CS, reset, IRQ pin
  if (!LoRa.begin(433E6)) {             // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }
  Serial.println("LoRa init succeeded.");
  LoRa.setSyncWord(0xF3);
  LoRa.setSpreadingFactor(7);
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
    while (!sync){
      if(LoRa.parsePacket()){
        byte syncT1_2 loRa.read();
        byte syncT2_2 loRa.read();
        byte syncT3_2 loRa.read();
        byte syncT4_2 loRa.read();
        long syncT = millis();
        byte syncT1 = ((syncT>>24)&0xff);
        byte syncT2 = ((syncT>>16)&0xff);
        byte syncT3 = ((syncT>>8)&0xff);
        byte syncT4 = ((syncT)&0xff);
        LoRa.write(syncT1);
        LoRa.write(syncT2);
        LoRa.write(syncT3);
        LoRa.write(syncT4);
        delay(10)
        byte timeDiff1_2 loRa.read();
        byte timeDiff2_2 loRa.read();
        byte timeDiff3_2 loRa.read();
        byte timeDiff4_ loRa.read();
        long timeDiff_2 = ((timeDiff1_2<<24)&0xff000000)+
                   ((timeDiff2_2<<16)&0xff0000)+
                   ((timeDiff3_2<<8)&0xff00)+
                   (timeDiff4_); 
        }
  
    for (int sample=0;sample<n;sample++){
      int del = 10 - (millis()-T[sample -1]);
        if(sample && del>0){
          Serial.println("Delaying:  "+(String)del );
          delay(del);
          }
        T[sample] = millis();
        mma.read();
        sensors_event_t event; 
        mma.getEvent(&event);
        X[sample] = mma.x;
        Y[sample] = mma.y;
        Z[sample] = mma.z;
        Serial.println("Sample: "+(String)sample);
        Xlsb[sample] = (X[sample] <<8)>>8;
        Xmsb[sample] = (X[sample]>>8)&0xff;
        Ylsb[sample] = (Y[sample] <<8)>>8;
        Ymsb[sample] = (Y[sample]>>8)&0xff;
        Zlsb[sample] = (Z[sample] <<8)>>8;
        Zmsb[sample] = (Z[sample]>>8)&0xff;
        t1[sample] = (T[sample] >> 24)&0xff;
        t2[sample] = (T[sample] >> 16)&0xff;
        t3[sample] = (T[sample] >> 8)&0xff;
        t4[sample] =  T[sample]&0xff;
        if (sample == n-1){
          logged = 0;
          }
        }
    Serial.println("Done getting yo damn readings boi! Finna write em down ;D");
    if (!logged){
        for (int val = 0; val < n;val++){
          LoRa.beginPacket();                   // start packet
          LoRa.write(Xmsb[val]);
          LoRa.write(Xlsb[val]);
          LoRa.write(Ymsb[val]);
          LoRa.write(Ylsb[val]);
          LoRa.write(Zmsb[val]);
          LoRa.write(Zlsb[val]);
          LoRa.write(t1[val]);
          LoRa.write(t2[val]);
          LoRa.write(t3[val]);
          LoRa.write(t4[val]);
          LoRa.endPacket();
          }
        }
        
    Serial.println("Going to sleep now");
    Serial.println("t1 ="+(String)timeDiff);
    Serial.println("  t2="+(String)timeDiff2);
    Serial.flush();
    timeDiff += -1000;
    timeDiff2 += 1000;
    esp_deep_sleep_start();
}
  
