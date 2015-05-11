#include <SoftwareSerial.h>

int pin=A0;    // MQ135 analog out pin
SoftwareSerial hcSerial(2, 3); //Connect HC-11's TX, RX

// Remember current air quality
int AirQuality = 0;

#define SENDING_INTERVAL 3000
unsigned long prevReadTime = 0;


void setup()
{
   Serial.begin(9600);
   hcSerial.begin(9600);
}

void loop()
{
  int sensorValue = analogRead(A0);  // read data from sensor
  AirQuality = (int)((float)sensorValue/1024*500);
  
  if(millis() - prevReadTime > SENDING_INTERVAL) {
    hcSerial.write(0x55);
    hcSerial.write(0x01);
    hcSerial.write(0x02);
    hcSerial.write((byte)(AirQuality >> 8));
    hcSerial.write((byte)AirQuality);
    hcSerial.write((byte)0x00);
    hcSerial.write((byte)0x00);
    hcSerial.write(0xFF);

    // for debug    
//    Serial.print(0x55, HEX);
//    Serial.print(" ");
//    Serial.print(0x01, HEX);
//    Serial.print(" ");
//    Serial.print(0x01, HEX);
//    Serial.print(" ");
//    Serial.print((byte)(i_temp >> 8), HEX);
//    Serial.print(" ");
//    Serial.print((byte)i_temp, HEX);
//    Serial.print(" ");
//    Serial.print((byte)(i_humi >> 8), HEX);
//    Serial.print(" ");
//    Serial.print((byte)i_humi, HEX);
//    Serial.print(" ");
//    Serial.print(0xFE, HEX);
//    Serial.println();

    prevReadTime = millis();
  }
  
  
  delay(500); //delay for reread
}
