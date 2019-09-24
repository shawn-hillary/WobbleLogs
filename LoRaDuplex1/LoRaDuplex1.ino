/*
  LoRa Duplex communication

  Sends a message every half second, and polls continually
  for new incoming messages. Implements a one-byte addressing scheme,
  with 0xFF as the broadcast address.

  Uses readString() from Stream class to read payload. The Stream class'
  timeout may affect other functuons, like the radio's callback. For an

  created 28 April 2017
  by Tom Igoe
*/
#include <SPI.h>              // include libraries
#include <LoRa.h>

const int csPin = 5;          // LoRa radio chip select
const int resetPin = 25;       // LoRa radio reset
const int irqPin = 2;         // change for your board; must be a hardware interrupt pin

String outgoing;              // outgoing message

byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0xFF;     // address of this device
byte destination = 0xBB;      // destination to send to
long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends

void setup() {
  Serial.begin(115200);                   // initialize serial
  while (!Serial);
  Serial.println("LoRa Duplex");
  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(csPin, resetPin, irqPin);// set CS, reset, IRQ pin
  if (!LoRa.begin(433E6)) {             // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }
  LoRa.setSpreadingFactor(7);
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa init succeeded.");
}

void loop() {
  while (!sync){
    sendMessage("syncing");
    syncT = millis();
    byte syncT1 = ((syncT>>24)&0xff);
    byte syncT2 = ((syncT>>16)&0xff);
    byte syncT3 = ((syncT>>8)&0xff);
    byte syncT4 = ((syncT)&0xff); 
    LoRa.write(syncT1);
    LoRa.write(syncT2);
    LoRa.write(syncT3);
    LoRa.write(syncT4);
    delay(10)
    byte syncT1_2 loRa.read();
    byte syncT2_2 loRa.read();
    byte syncT3_2 loRa.read();
    byte syncT4_2 loRa.read();
    long syncT_2 = ((syncT1_2<<24)&0xff000000)+
                   ((syncT2_2<<16)&0xff0000)+
                   ((syncT3_2<<8)&0xff00)+
                   (syncT4_2); 
    long timeDiff = (syncT - syncT_2);
    byte timeDiff1 = ((syncT>>24)&0xff);
    byte timeDiff2 = ((syncT>>16)&0xff);
    byte timeDiff3 = ((syncT>>8)&0xff);
    byte timeDiff4 = ((syncT)&0xff);
    LoRa.write(timeDiff1);
    LoRa.write(timeDiff2);
    LoRa.write(timeDiff3);
    LoRa.write(timeDiff4);
   
  }
  if (millis() - lastSendTime > interval) {
    String message = "HeLoRa World!";   // send a message
    sendMessage(message);
    Serial.println("Sending " + message);
    lastSendTime = millis();            // timestamp the message
    interval = random(2000) + 1000;    // 2-3 seconds
  }

  // parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());
}

void sendMessage(String outgoing) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return
  // read packet header bytes:
  Serial.println("READING...");
  // incoming accelerometer bytes
  byte Xmsb = LoRa.read();
  byte Xlsb = LoRa.read();
  byte Ymsb = LoRa.read();
  byte Ylsb = LoRa.read();
  byte Zmsb = LoRa.read();
  byte Zlsb = LoRa.read();
  byte t1 = LoRa.read();
  byte t2 = LoRa.read();
  byte t3 = LoRa.read();
  byte t4 = LoRa.read();
  
  int16_t X = ((Xmsb&0xFF)<<8)|(Xlsb&0xFF);
  int16_t Y = ((Ymsb&0xFF)<<8)|(Ylsb&0xFF);
  int16_t Z = ((Zmsb&0xFF)<<8)|(Zlsb&0xFF);
  unsigned long T = ((t1<<24)&0xff000000)+((t2<<16)&0xff0000)+((t3<<8)&0xff00)+(t4); 
  
  Serial.print("X: "+(String(X)));
  Serial.print(" Y: "+(String(Y)));
  Serial.print(" Z: "+(String(Z)));
  Serial.print(" T: "+(String(T)));
  Serial.print(" t1: "+(String(t1)));
  Serial.print(" t2: "+(String(t2)));
  Serial.print(" t3: "+(String(t3)));
  Serial.print(" t4: "+(String(t4)));
  Serial.println();
}
