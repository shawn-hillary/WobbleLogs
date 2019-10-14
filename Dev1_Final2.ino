/*Code for ESP32 based Pole Vibrations Sensor - Device 1
   Created by Shawn Hillary - shawnhillary03@gmail.com
   September 2019
  Adapts LoRa Library Function "LoRa Duplex" created 28 April 2017 by Tom Igoe
*/

#include <SPI.h>              // include libraries
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>
#include <stdio.h>
#include <stdlib.h>
#include <arduinoFFT.h>
#include "FS.h"
#include "SD.h"

#define n 4096 //samples
#define timeOut 900000

const int csPin = 5;            // LoRa radio chip select
const int resetPin = 25;        // LoRa radio reset
const int irqPin = 2;           // change for your board; must be a hardware interrupt pin
Adafruit_MMA8451 mma = Adafruit_MMA8451();
arduinoFFT FFT1 = arduinoFFT();
arduinoFFT FFT2 = arduinoFFT();
bool sync =  0;
long unsigned int count = 0;

int16_t X[n], X2[n];            // Create arrays for recieved and recorded bytes
int16_t Y[n], Y2[n];
int16_t Z[n], Z2[n];
int16_t vane[n], anemo[n];
uint8_t cardType = SD.cardType();
unsigned long int T[n], T_2[n];
char X2msb[n];
char X2lsb[n];
char Y2msb[n];
char Y2lsb[n];
char Z2msb[n];
char Z2lsb[n];
char t21[n];
char t22[n];
char t23[n];
char t24[n];
double vibe1[n];
double vibe2[n];
double imag1[n];
double imag2[n];
bool logged = 1;
long startTime;
bool doneSending = 0;
int storeVal = 0;
int p1, p2, p3, p4, p5, p6, p7, p8, p9, p10;
RTC_DATA_ATTR int bootCount = 0;



void takeReadings() {
  for (int sample = 0; sample < n; sample++) {
    int del = 10 - (millis() - T[sample - 1]);
    if (sample && del > 0) {
      Serial.println("Delaying:  " + (String)del );
      delay(del);
    }
    T[sample] = millis();
    mma.read();
    sensors_event_t event;
    mma.getEvent(&event);
    X[sample] = mma.x;
    Y[sample] = mma.y;
    Z[sample] = mma.z;
    vane[sample] =
      anemo[sample] =
        Serial.println("Sample: " + (String)sample);
    if (sample == n - 1) {
      logged = 0;
    }
  }
}

void onReceive(int packetSize) {
  if (packetSize) {
    Serial.println("Got pack!");
  }
  if (packetSize == 0) return;          // if there's no packet, return
  Serial.println(packetSize);

  if (packetSize == 1) {
    Serial.println("Checking key");
    byte s = LoRa.read();
    if (s == 0xAA) {
      Serial.println("Synced");
      LoRa.beginPacket();
      LoRa.write(s);
      LoRa.endPacket();
      sync = 1;
      takeReadings();
      return;
    }
  }

  if (packetSize == 2) {
    doneSending = 1;
    p1, p2, p3, p4, p5, p6, p7, p8, p9, p10 = 0;
    storeData();
  }

  //  esp_deep_sleep_start();
  if (packetSize == 129) {

    char code = LoRa.read();
    int s = 0;

    if (code == 1) {
      Serial.println("Got here, code is 1");
      int s = 0;
      while (LoRa.available()) {
        t21[p1 + s] = LoRa.read();
        s ++;
      }
      p1 = p1 + 128;
    }

    if (code == 2) {
      Serial.println("Got here, code is 2");
      int s = 0;
      while (LoRa.available()) {
        t22[p2 + s] = LoRa.read();
        s ++;
      }
      p2 = p2 + 128;
    }

    if (code == 3) {
      Serial.println("Got here, code is 3");
      int s = 0;
      while (LoRa.available()) {
        t23[p3 + s] = LoRa.read();
        s ++;
      }
      p3 = p3 + 128;
    }

    if (code == 4) {
      Serial.println("Got here, code is 4");
      int s = 0;
      while (LoRa.available()) {
        t24[p4 + s] = LoRa.read();
        s ++;
      }
      p4 = p4 + 128;
    }

    if (code == 5) {
      Serial.println("Got here, code is 5");
      int s = 0;
      while (LoRa.available()) {
        X2msb[p5 + s] = LoRa.read();
        s ++;
      }
      p5 = p5 + 128;
    }

    if (code == 6) {
      Serial.println("Got here, code is 6");
      int s = 0;
      while (LoRa.available()) {
        X2lsb[p6 + s] = LoRa.read();
        s ++;
      }
      p6 = p6 + 128;
    }

    if (code == 7) {
      Serial.println("Got here, code is 7");
      int s = 0;
      while (LoRa.available()) {
        Y2msb[p7 + s] = LoRa.read();
        s ++;
      }
      p7 = p7 + 128;
    }

    if (code == 8) {
      Serial.println("Got here, code is 8");
      int s = 0;
      while (LoRa.available()) {
        Y2lsb[p8 + s] = LoRa.read();
        s ++;
      }
      p8 = p8 + 128;
    }

    if (code == 9) {
      Serial.println("Got here, code is 9");
      int s = 0;
      while (LoRa.available()) {
        Z2msb[p9 + s] = LoRa.read();
        s ++;
      }
      p9 = p9 + 128;
    }

    if (code == 10) {
      Serial.println("Got here, code is 10");
      int s = 0;
      while (LoRa.available()) {
        Z2lsb[p10 + s] = LoRa.read();
        s ++;
      }
      p10 = p10 + 128;
    }
    Serial.println((String)LoRa.packetRssi());
  }

}

void setup() {
  Serial.begin(115200);                   // initialize serial
  while (!Serial);
  Serial.println("LoRa Duplex");
  LoRa.setPins(csPin, resetPin, irqPin);
  if (!LoRa.begin(868E6)) {             // initialize radio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    return;                             // if failed, do nothing
  }
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

  LoRa.setSpreadingFactor(7);
  LoRa.setSyncWord(0xF3);
  LoRa.setPreambleLength(8);
  LoRa.setSignalBandwidth(250E3);
  //  LoRa.explicitHeaderMode()
  LoRa.enableCrc();
  LoRa.setCodingRate4(8);
  Serial.println("LoRa init succeeded.");

  startTime = millis();
  esp_sleep_enable_timer_wakeup(600e6);
  unsigned long syncT = micros();           //Send this time to DEV2
  byte syncT1 = ((syncT >> 24) & 0xff);
  byte syncT2 = ((syncT >> 16) & 0xff);
  byte syncT3 = ((syncT >> 8) & 0xff);
  byte syncT4 = ((syncT) & 0xff);
  LoRa.beginPacket();
  LoRa.write(syncT1);
  LoRa.write(syncT2);
  LoRa.write(syncT3);
  LoRa.write(syncT4);
  LoRa.endPacket();
  Serial.println("Sending..");

  if (sync) {
    takeReadings();

  }
}

void loop() {
  if ((millis() + startTime) > timeOut) {
    esp_sleep_enable_timer_wakeup(10);
    Serial.println("Timed out");
    esp_deep_sleep_start();
  }
  if (!sync) {
    onReceive(LoRa.parsePacket());
  }
  if (!doneSending) {
    onReceive(LoRa.parsePacket());
  }
  if (sync && doneSending) {
    storeData();
    writeData();
    bootCount ++;
    esp_deep_sleep_start();
  }
}

void storeData() {
  for (storeVal = 0; storeVal < n; storeVal++) {
    X2[storeVal] = (((X2msb[storeVal] & 0xFF) << 8) | (X2lsb[storeVal] & 0xFF));
    Y2[storeVal] = ((Y2msb[storeVal] & 0xFF) << 8) | (Y2lsb[storeVal] & 0xFF);
    Z2[storeVal] = ((Z2msb[storeVal] & 0xFF) << 8) | (Z2lsb[storeVal] & 0xFF);
    T_2[storeVal] = ((t21[storeVal] << 24) & 0xff000000) + ((t22[storeVal] << 16) & 0xff0000) + ((t23[storeVal] << 8) & 0xff00) + (t24[storeVal]);
  }
}
void writeData() {
  LoRa.end();
  while (!SD.begin(15)) {
    Serial.println("Card Mount Failed");
    delay(200);
  }
  cardType = SD.cardType();
  while (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    cardType = SD.cardType();
    delay(200);
  }
  String tempString;
  String setName = ("/set_" + (String)bootCount + ".csv");
  writeFile(SD, setName.c_str(), "Here we test! \n" );
  for (int w = 0; w < n; w++) {
    tempString = String(T[w], DEC);
    appendFile(SD, setName.c_str(), tempString.c_str());
    appendFile(SD, setName.c_str(), ";");
    tempString = String(X[w], DEC);
    appendFile(SD, setName.c_str(), tempString.c_str());
    appendFile(SD, setName.c_str(), ";");
    tempString = String(Y[w], DEC);
    appendFile(SD, setName.c_str(), tempString.c_str());
    appendFile(SD, setName.c_str(), ";");
    tempString = String(Z[w], DEC);
    appendFile(SD, setName.c_str(), tempString.c_str());
    appendFile(SD, setName.c_str(), ";");
    tempString = String(T_2[w], DEC);
    appendFile(SD, setName.c_str(), tempString.c_str());
    appendFile(SD, setName.c_str(), ";");
    tempString = String(X2[w], DEC);
    appendFile(SD, setName.c_str(), tempString.c_str());
    appendFile(SD, setName.c_str(), ";");
    tempString = String(Y2[w], DEC);
    appendFile(SD, setName.c_str(), tempString.c_str());
    appendFile(SD, setName.c_str(), ";");
    tempString = String(Z2[w], DEC);
    appendFile(SD, setName.c_str(), tempString.c_str());
    appendFile(SD, setName.c_str(), ";");
    tempString = String(vane[w], DEC);
    appendFile(SD, setName.c_str(), tempString.c_str());
    appendFile(SD, setName.c_str(), ";");
    tempString = String(anemo[w], DEC);
    appendFile(SD, setName.c_str(), tempString.c_str());
    appendFile(SD, setName.c_str(), "\n");
    Serial.println((String)w);
  }
  Serial.println("Donzo!");

}

void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
  // Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
  } else {
    Serial.println("Append failed");
  }
  file.close();
}
