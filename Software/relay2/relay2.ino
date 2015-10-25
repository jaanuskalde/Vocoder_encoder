#include <SPI.h>
#include <Streaming.h>
//#include "nRF24L01.h"
#include "RF24.h"

RF24 radio(6,7);
byte address[7] = "1Relay";

uint8_t data [2][12] = {{ 5,6,7,  8, 9, 10,11,0, 1, 2, 3,4},{ 0,11,10,9,8,7,6,5,4,3,2,1}};

uint8_t data_out [18] = {0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0};


#define MODE_SWITCH 5
#define MODE_RELAY 4
#define Y15 2
#define Y16 A4
#define Y17 A5
#define LED_EN 10

void setup()
{
  Serial.begin(115200);
  //motor controller init
  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  //pins for 4-to-16 driver
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(Y15, OUTPUT);
  digitalWrite(Y15, HIGH);
  pinMode(Y16, OUTPUT);
  digitalWrite(Y16, HIGH);
  pinMode(Y17, OUTPUT);
  digitalWrite(Y17, HIGH);
  
  
  //radio init
  radio.begin();                          // Start up the radio
  radio.setAutoAck(1);                    // Ensure autoACK is enabled
  radio.setRetries(15,15);                // Max delay between retries & number of retries
  radio.setPayloadSize(24);        //payload
  radio.setDataRate(RF24_1MBPS);  //data rate to 1Mbps
  radio.openReadingPipe(1,address);
  radio.startListening();                 // Start listening

  //force clock
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  pinMode(LED_EN, OUTPUT);
  digitalWrite(LED_EN, LOW);

  pinMode(MODE_SWITCH, INPUT);
  digitalWrite(MODE_SWITCH, HIGH); //pullup
  pinMode(MODE_RELAY, OUTPUT);

}


void loop()
{
  convert(digitalRead(MODE_SWITCH));
  if (digitalRead(MODE_SWITCH))
  {
    digitalWrite(MODE_RELAY, LOW);
    
    for (char dev = 0; dev < 18; dev ++)
    {
      select(dev);
      SPI.transfer(data_out[dev]);
      delayMicroseconds(1);
    }
  }
  else
  {
    digitalWrite(MODE_RELAY, HIGH);  

    for (char dev = 0; dev < 18; dev ++)
    {
      select(dev);
      SPI.transfer(data_out[dev]);
      delayMicroseconds(1);
    }
  }
  select(18);
  
  if( radio.available())
  {
    uint8_t tmp [24];
    boolean success = false;
    radio.read(tmp, 24);             // Get the payload
    
    for (char i = 0; i < 24; i++)
    {
      if (tmp[i] != 0) success = true;
    }
    
    if (success)
    {
      memcpy(data,tmp,24);
      Serial << "Data in: ";
      for (char y = 0; y < 12; y ++) Serial << data[0][y] << " ";
      Serial << endl << "Data out: ";
      for (char dev = 0; dev < 18; dev ++) Serial << data_out[dev] << " ";
      Serial << endl;
    }
  }
/*
  for (char i = 0; i < 12; i ++)
  {
    data[0][i] ++;
    if (data[0][i] > 11) data[0][i] = 0;
  }*/
  
  delay(20);


}

void convert(char brd)
{
  //zero the data
  memset(data_out,0,18);

  //populate the data
  for (char y = 0; y < 12; y ++)
  for (char x = 0; x < 12; x ++) 
  {
    if (data[brd][y] == x)
    {
      unsigned char num_ch = x+y*12;
      data_out[num_ch / 8] |= 1 << (7 & num_ch);
    }
  }
}

//select relay driver, input from 0 to 17
void select(char dev)
{
  if (dev < 15)
  {
    digitalWrite(Y15, HIGH);
    digitalWrite(Y16, HIGH);
    digitalWrite(Y17, HIGH);
    if (dev & 0x1) digitalWrite(A0, HIGH); else digitalWrite(A0, LOW);
    if (dev & 0x2) digitalWrite(A1, HIGH); else digitalWrite(A1, LOW);
    if (dev & 0x4) digitalWrite(A2, HIGH); else digitalWrite(A2, LOW);
    if (dev & 0x8) digitalWrite(A3, HIGH); else digitalWrite(A3, LOW);
  }
  else
  {
    digitalWrite(A0, HIGH);
    digitalWrite(A1, HIGH);
    digitalWrite(A2, HIGH);
    digitalWrite(A3, HIGH);
    
    if (dev == 15) digitalWrite(Y15, LOW); else digitalWrite(Y15, HIGH);
    if (dev == 16) digitalWrite(Y16, LOW); else digitalWrite(Y16, HIGH);
    if (dev == 17) digitalWrite(Y17, LOW); else digitalWrite(Y17, HIGH);
  }
}
