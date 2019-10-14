#include <SPI.h>              // include libraries
#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>
#include <stdio.h>
#include <stdlib.h>
#include <arduinoFFT.h>
#include "FS.h"
#include "SD.h"

#define n 4096 //samples
#define timeOut 150000000
unsigned long startTime;
Adafruit_MMA8451 mma = Adafruit_MMA8451();
long unsigned int count = 0;

int16_t X[n];
int16_t Y[n];
int16_t Z[n];          // Create arrays for recieved and recorded bytes
int16_t vane[n], anemo[n];
int32_t avgWind;
int32_t avgDir;
unsigned long int T[n];
uint8_t cardType = SD.cardType();
RTC_DATA_ATTR int bootCount = 0;



void setup() {
  // initialize serial
  pinMode(2, OUTPUT);
  startTime = millis();
  Serial.begin(115200);
  while (!Serial);
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
  esp_sleep_enable_timer_wakeup(10e6);
}

void loop() {
  takeReadings();
  writeData();
  bootCount ++;
  int64_t tSleep = (300e6+600  e6*(float)(1-(((float)(avgWind-304))/1000)));  //Sleep time dependant on the wind speed
  esp_deep_sleep(tSleep);

  // put your main code here, to run repeatedly:

}

void takeReadings() {
  digitalWrite(2, HIGH);
  for (int sample = 0; sample < n; sample++) {
    int del = 10 - (millis() - T[sample - 1]);
    if (sample && del > 0) {
      Serial.println("Delaying:  " + (String)del );
      delay(del);
    }
    T[sample] = millis();
    Serial.print("T: " + (String)T[sample]);
    mma.read();
    sensors_event_t event;
    mma.getEvent(&event);
    X[sample] = mma.x;
    Serial.print(" X: " + (String)X[sample]);
    Y[sample] = mma.y;
    Serial.print(" Y: " + (String)Y[sample]);
    Z[sample] = mma.z;
    Serial.print(" Z: " + (String)Z[sample]);
    vane[sample] = analogRead(32);
    Serial.print(" Vane: " + (String)vane[sample]);
    anemo[sample] = analogRead(14);
    Serial.println(" Anemo: " + (String)anemo[sample]);
  }

  for (int wind = 0; wind < n; wind++) {
    avgWind += anemo[wind];
    avgDir += vane[wind];
  }
  avgWind = avgWind / n;
  avgDir = avgDir / n;
  digitalWrite(2, LOW);
}

void writeData() {
  while (!SD.begin(15)) {
    Serial.println("Card Mount Failed");
    delay(1000);
    digitalWrite(2, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(1000);                       // wait for a second
    digitalWrite(2, LOW);    // turn the LED off by making the voltage LOW
  }
  cardType = SD.cardType();
  while (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    cardType = SD.cardType();
    delay(200);
  }
  String tempString = "";
  String setName = ("/"+(String)bootCount+".csv");
  writeFile(SD, setName.c_str(), "readings:\n" );
  for (int w = 0; w < n; w++) {
    digitalWrite(2, !digitalRead(2));
    tempString = "";
    tempString += String(T[w], DEC) + ";";
    tempString += String(X[w], DEC) + ";";
    tempString += String(Y[w], DEC) + ";";
    tempString += String(Z[w], DEC) + ";";
    tempString += String(vane[w], DEC) + ";";
    tempString += String(anemo[w], DEC) + "\n";
    appendFile(SD, setName.c_str(), tempString.c_str());
  }
  Serial.println("Donzo!");
  digitalWrite(2, LOW);

}


//SD Functions

void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    for (int bnk = 0; bnk < 30; bnk++) {
      digitalWrite(2, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(50);                       // wait for a second
      digitalWrite(2, LOW);    // turn the LED off by making the voltage LOW
      delay(100);
    }
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
    for (int bnk = 0; bnk < 2; bnk++) {
      digitalWrite(2, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(500);                       // wait for a second
      digitalWrite(2, LOW);    // turn the LED off by making the voltage LOW
      delay(1000);
    }
  } else {
    Serial.println("Write failed");
    for (int bnk = 0; bnk < 30; bnk++) {
      digitalWrite(2, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(50);                       // wait for a second
      digitalWrite(2, LOW);    // turn the LED off by making the voltage LOW
      delay(1000);
    }
    
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
