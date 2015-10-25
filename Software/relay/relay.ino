#include "Tlc5940.h"
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

RF24 radio(6,7);
byte addresses[][7] = {"1Relay","2Relay"};

uint8_t data [2][12] = {{ 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,0},{ 0,11,10,9,8,7,6,5,4,3,2,1}};

#define MODE_SWITCH 5
#define MODE_RELAY 4

//can be 0 or 1
#define BOARD_ID 0

void setup()
{
  Tlc.init(0);
  
  radio.begin();                          // Start up the radio
  radio.setAutoAck(1);                    // Ensure autoACK is enabled
  radio.setRetries(15,15);                // Max delay between retries & number of retries
  radio.setPayloadSize(24);        //payload
  radio.setDataRate(RF24_1MBPS);  //data rate to 1Mbps
  radio.openReadingPipe(1,addresses[0]);
  radio.startListening();                 // Start listening

  pinMode(MODE_SWITCH, INPUT);
  digitalWrite(MODE_SWITCH, HIGH); //pullup
  pinMode(MODE_RELAY, OUTPUT);
}

void loop()
{
  Tlc.clear();

  if (digitalRead(MODE_SWITCH))
  {
    digitalWrite(MODE_RELAY, LOW);  
    for (char y = 0; y < 12; y ++) for (char x = 0; x < 12; x ++) if (data[BOARD_ID][y] == x) Tlc.set(x+y*12, 4095);
  }
  else
  {
    digitalWrite(MODE_RELAY, HIGH);  
    for (char y = 0; y < 12; y ++) for (char x = 0; x < 12; x ++) if (data[1-BOARD_ID][y] == x) Tlc.set(x+y*12, 4095);
  }
    
  Tlc.update();
  delay(100);
  /*
  for (unsigned char channel = 0; channel < 144; channel ++)
  {
    Tlc.clear();
    Tlc.set(channel, 4095);

    Tlc.update();

    delay(250);  
 }*/
  
  if( radio.available())
  {
    radio.read(data, 24);             // Get the payload
  }
}

