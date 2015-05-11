#include <DHT11.h>
#include <SoftwareSerial.h>

int pin=5;    // DHT11 data pin
DHT11 dht11(pin); 
SoftwareSerial hcSerial(2, 3); //Connect HC-11's TX, RX

float temp_sum = 0;
float humi_sum = 0;
int count = 0;

#define SENDING_INTERVAL 5000
unsigned long prevReadTime = 0;


void setup()
{
   Serial.begin(9600);
   hcSerial.begin(9600);
}

void loop()
{
  int err;
  float temp, humi;
  if((err=dht11.read(humi, temp))==0)
  {
    temp_sum = temp_sum + temp;
    humi_sum = humi_sum + humi;
    count = count + 1;
  }
  else
  {
    Serial.print("Error No :");
    Serial.print(err);
    Serial.println();
  }
  
  if(millis() - prevReadTime > SENDING_INTERVAL) {
    int i_temp = (int)(temp_sum / count);
    int i_humi = (int)(humi_sum / count);
    hcSerial.write(0x55);
    hcSerial.write(0x01);
    hcSerial.write(0x01);
    hcSerial.write((byte)(i_temp >> 8));
    hcSerial.write((byte)i_temp);
    hcSerial.write((byte)(i_humi >> 8));
    hcSerial.write((byte)i_humi);
    hcSerial.write(0xFE);

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
    
    count = 0;
    temp_sum = 0;
    humi_sum = 0;
    prevReadTime = millis();
  }
  
  
  delay(1000); //delay for reread
}
