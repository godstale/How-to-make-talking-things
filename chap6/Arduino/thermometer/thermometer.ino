#include <DHT11.h>
#include <SoftwareSerial.h>

int pin=5;    // 연결한 아두이노 디지털 핀 번호
DHT11 dht11(pin); 
SoftwareSerial BTSerial(2, 3); //Connect BT's TX, RX

float temp_sum = 0;
float humi_sum = 0;
int count = 0;

#define SENDING_INTERVAL 40000
unsigned long prevReadTime = 0;


void setup()
{
   Serial.begin(9600);
   BTSerial.begin(9600);
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
    BTSerial.print("Error No :");
    BTSerial.print(err);
    BTSerial.println();    
  }
  
  if(millis() - prevReadTime > SENDING_INTERVAL) {
    BTSerial.print("thingspeak:key=xxx&field1=");
    BTSerial.print((int)(temp_sum / count));
    BTSerial.print("&field2=");
    BTSerial.print((int)(humi_sum / count));
    BTSerial.print("[*]");
    
    count = 0;
    temp_sum = 0;
    humi_sum = 0;
    prevReadTime = millis();
  }
  
  
  delay(DHT11_RETRY_DELAY); //delay for reread
}
