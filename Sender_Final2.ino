/*Code for ESP32 based Pole Vibrations Sensor - Device 2
 * Created by Shawn Hillary - shawnhillary03@gmail.com
 * September 2019
 *Adapts LoRa Library Function "LoRa Duplex" created 28 April 2017 by Tom Igoe
*/

#include <SPI.h>              // include libraries
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>
#include <stdio.h>
#include <stdlib.h>

#define n 4096
#define timeOut 80000

Adafruit_MMA8451 mma = Adafruit_MMA8451();
bool sync = 0;
bool gotTime = 0;

unsigned long syncT;
unsigned long syncT_2;
bool logged = 1;
byte key;
long startTime;
const int csPin = 5;          // LoRa radio chip select
const int resetPin = 25;       // LoRa radio reset
const int irqPin = 2;         // change for your board; must be a hardware interrupt pin
int16_t X[n];
int16_t Y[n];
int16_t Z[n];
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

void setup() {
  startTime = millis();
  esp_sleep_enable_timer_wakeup(1020e6);
  Serial.begin(115200);                   // initialize serial
  while (!Serial);
  
  LoRa.setPins(csPin, resetPin, irqPin);// set CS, reset, IRQ pin
  if (!LoRa.begin(868E6)) {             // initialize ratio at 433MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }
  Serial.println("LoRa init succeeded.");
  //Set LoRa Parameters
  LoRa.setSyncWord(0xF3);
  LoRa.setSpreadingFactor(7);
  LoRa.setPreambleLength(8);
  LoRa.setSignalBandwidth(250E3);
  LoRa.enableCrc();
  LoRa.setCodingRate4(8);
  
  //Initialize Accelerometer  
  Serial.println("Adafruit MMA8451 test!");
    while (! mma.begin()) {
    Serial.println("Couldnt start");
    delay(500);
    }
  Serial.println("MMA8451 found!");
  mma.setRange(MMA8451_RANGE_2_G);
  Serial.print("Range = ");
  Serial.print(2 << mma.getRange());  
  Serial.println("G");
  }

void loop() {
  //Timeout function
  if ((millis()+ startTime - ((syncT/1000) - (syncT_2/1000))) > timeOut){
    Serial.println("Timed out");
    esp_deep_sleep_start();
    }
  //Wait for Dev1 to send time to sync
  while (!sync){
    while (!gotTime){
    onReceive(LoRa.parsePacket());
    }
  
  LoRa.beginPacket();
  LoRa.write(0xAA);
  LoRa.endPacket();
  delay(100);
  Serial.println("Synced");
  sync = 1;
  onReceive(LoRa.parsePacket());
  }                 
          takeReadings();
          delay(300);
          
          Serial.println("Done getting yo damn readings boi! Finna write em down ;D");
          if (!logged){
            sendData(t1,0x1);
            sendData(t2,0x2);
            sendData(t3,0x3);
            sendData(t4,0x4);
            sendData(Xmsb,0x5);
            sendData(Xlsb,0x6);
            sendData(Ymsb,0x7);
            sendData(Ylsb,0x8);
            sendData(Zmsb,0x9);
            sendData(Zlsb,0xa);
            LoRa.beginPacket();
            LoRa.write(0xab);       //Send 2 byte packet to notify Dev1 that sending is done.
            LoRa.write(0xcd);
            Serial.println("Sending 2 byte");
            LoRa.endPacket();
            Serial.println("Sleeping zzz");
            esp_deep_sleep_start();
          }
}

void sendData(char set[],byte code){
  long tStartSend = millis(); 
  for(int packs = 0;packs < (n/128);packs++){
    LoRa.beginPacket();
    LoRa.write(code);
    for(int pack = packs*128;pack<((packs*128)+128);pack++){
      LoRa.write(set[pack]);
    }
    LoRa.endPacket();
  }
  long tEndSend = millis();
  Serial.print("Sending time is:   ");
  Serial.println((String)(tEndSend - tStartSend));
  delay(100);
  }
  

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  if (packetSize == 1) {
    byte s = LoRa.read();
    if(s==0xAA){
      Serial.println("Synced!");
      sync = 1;
      return;
      }
    }
    
    if ((packetSize == 4) && !sync && !gotTime) {
      byte syncT1_2 = 0;
      byte syncT2_2 = 0;
      byte syncT3_2 = 0;
      byte syncT4_2 = 0; 
      syncT1_2 = LoRa.read();            
      syncT2_2 = LoRa.read();
      syncT3_2 = LoRa.read();     
      syncT4_2 = LoRa.read();
      syncT_2 = ((syncT1_2<<24)&0xff000000)+
                ((syncT2_2<<16)&0xff0000)+
                ((syncT3_2<<8)&0xff00)+
                 (syncT4_2);
      syncT = micros();
      if(syncT1_2||syncT2_2||syncT3_2||syncT4_2){
        Serial.println("Delaying for: "+String(syncT - syncT_2)); //Delay to sync samples
        delayMicroseconds(syncT - syncT_2 + 100000);
        Serial.println("Got time, sending key");
        gotTime = 1;
        } 
      }
}

void takeReadings(){
  Serial.println("Getting Readings");
  for (int sample=0;sample<n;sample++){
    int del = 10 - (millis()-T[sample -1]);
    if(sample && del>0){
      delay(del);
      }
    T[sample] = millis();
    Serial.println("Time at sample no " +(String)sample +" is " +(String)T[sample]);
    mma.read();
    sensors_event_t event; 
    mma.getEvent(&event);
    X[sample] = mma.x;
    Y[sample] = mma.y;
    Z[sample] = mma.z;
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
    Serial.println("Done Getting Readings!");
  }
  
