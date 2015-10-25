#include <TimerOne.h>
#include <SPI.h>
#include "RF24.h"

#define A 7
#define B 8
#define C 9
#define D A0
#define G 6
#define DI 5
#define CLK 4
#define LAT 4

RF24 radio(3,2);
byte addresses[][7] = {"1Relay","2Relay"};
char data_out[24] = {11, 5, 2, 7, 4, 3, 9, 6, 10, 8, 1, 0, 5, 11, 9, 2, 8, 6, 10, 1, 0, 7, 4, 3};

char data[24] = {255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255};
//char data[24] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};


unsigned char state = 0;
unsigned char which = 0;
unsigned char y,x = 0;

volatile unsigned char led_row = 0;

void updateDisplay()
{
  digitalWrite(G, HIGH);
  digitalWrite(CLK, HIGH);
  digitalWrite(CLK, LOW);
  shiftOut(DI, CLK, LSBFIRST, data[led_row*2]);
  shiftOut(DI, CLK, LSBFIRST, data[(led_row*2)+1]);
  digitalWrite(CLK, LOW);
  digitalWrite(CLK, HIGH);
  digitalWrite(CLK, LOW);
  
  if (led_row & 1) digitalWrite(A, LOW); else digitalWrite(A, HIGH); 
  if (led_row & 2) digitalWrite(B, LOW); else digitalWrite(B, HIGH); 
  if (led_row & 4) digitalWrite(C, LOW); else digitalWrite(C, HIGH); 
  if (led_row & 8) digitalWrite(D, LOW); else digitalWrite(D, HIGH); 
  digitalWrite(G, LOW);

  led_row++;
  
  if (led_row == 12) led_row = 0;
}

void setup()
{
  pinMode(LAT, INPUT);
  pinMode(CLK, OUTPUT);
  pinMode(DI, OUTPUT);
  
  pinMode(A, OUTPUT);
  digitalWrite(A, LOW);
  pinMode(B, OUTPUT);
  digitalWrite(B, LOW);
  pinMode(C, OUTPUT);
  digitalWrite(C, LOW);
  pinMode(D, OUTPUT);
  digitalWrite(D, LOW);
  pinMode(G, OUTPUT);
  digitalWrite(G, LOW);

  //timer for display
  Timer1.initialize(1000);
  Timer1.attachInterrupt(updateDisplay);


  Serial.begin(9600);


  radio.begin();                          // Start up the radio
  radio.setAutoAck(1);                    // Ensure autoACK is enabled
  radio.setRetries(15,15);                // Max delay between retries & number of retries
  radio.setPayloadSize(24);        //payload
  radio.setDataRate(RF24_1MBPS);  //data rat  e to 1Mbps
  radio.openWritingPipe(addresses[0]);
  interrupts();
}

void loop()
{
  
  if (Serial.available() > 0)
  {
    if (state == 0)
    {
      char tmp = Serial.read();
      if (tmp == 'e')
      {
        which = 0;
        state++;
      }
      else if (tmp == 'd')
      {
        which = 1;
        state++;
      }
      else state = 0;
      Serial.println(state);
    }
    else if (state == 1)
    {
      if (Serial.read() == 12) state ++; else state = 0;
      Serial.println(state);
    }
    else if (state == 2)
    {
      if (Serial.read() == 12) state ++; else state = 0;
      Serial.println(state);
      Serial.println("Receiving matrix.");
      y=11;
      x=0;
    }
    else if (state == 3)
    {
      int tmp = Serial.read();
      if (tmp == 0)
      {
        if (which == 0) data[y*2+((x&8)>>3)] |= 1<<(x-(x&8)); 
      }
      else if (tmp == 1)
      {
        if (which == 0)
        {
          data[y*2+((x&8)>>3)] &= ~(1<<(x-(x&8)));
          data_out[y] = x;
        }
        else
        {
          data_out[y+12] = x;
        }
      }
      else
      {
         Serial.println("Error, cancelling");
         state = 0;
      }

      x++;
      
      if (x == 12)
      {
        x = 0;
        y --;

        if (y == 255)
        {
          Serial.println("Matrix received.");
          state = 0;
          //new matrixes to slaves
          radio.openWritingPipe(addresses[0]);
          radio.write(data_out, 24);
          radio.openWritingPipe(addresses[1]); 
          radio.write(data_out, 24);

        }
      }
    }
  }
  
/*
    for (; y < 12; y++)
    {
      for (; x < 12; x++)
      {
        while((Serial.available() < 2));
         
        
        Serial.print(x-(x&8));
        Serial.print(' ');
        
        if (Serial.read() == '0')
        {
          data[y*2+((x&8)>>3)] |= 1<<(x-(x&8)); 
        }
        else
        {
          data[y*2+((x&8)>>3)] &= ~(1<<(x-(x&8)));
        }
      }
    }
    Serial.println(x);
    Serial.println(y);
    
    if ((y == 12) && (x == 12)) Serial.println("Matrix received.");
  }
*/ 
  
  /*
  if (nrf24.available())
  {
    Serial.println("got data");
    char data[10];
    uint8_t len = 0;
    if (!nrf24.recv((uint8_t*)data, &len))
      Serial.println("read failed");
    else
      Serial.println(data);
  }*/  
//  delay(1);
}
