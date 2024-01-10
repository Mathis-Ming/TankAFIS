#include <MeMegaPi.h>
#include <SoftwareSerial.h>
#include <stdio.h>
#include <stdlib.h>

MeMegaPiDCMotor motor(PORT1B);
uint8_t speed = 100;


void setup()
{
  Serial.begin(115200);
  Serial3.begin(115200);    //The factory default baud rate is 115200
  Serial.println("Bluetooth Start!");
}

void loop()
{ 

  if(Serial3.available())
  {
    if(Serial3.readString() == "on")
    {
      Serial.println(Serial3.readString());
      motor.run(speed);
      Serial3.flush();
    }
    else
    {
      //Serial.println("off");
      Serial.println(Serial3.readString());
      motor.stop();
      Serial3.flush();
    }
    
  }
  Serial3.write("Hello from MegaPi!\n");
  
  //delay(1000);
}