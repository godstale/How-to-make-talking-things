#include <SoftwareSerial.h>

SoftwareSerial WiFiSerial(2, 3); // WiFi232's TX, RX
const int PIN_LEFT = 4;
const int PIN_RIGHT = 5;
const int PIN_FORWARD = 6;
const int PIN_REVERSE = 7;

const char CHECK_0 = 0x55;
const char CHECK_1 = 0x01;
const char CHECK_2 = 0x01;
const char CHECK_2_0 = 0x00;
const char CHECK_LAST_MIN = 0x58;
const char CHECK_LAST_MAX = 0x5F;

#define BUFFER_SIZE 8
static char Buffer[BUFFER_SIZE] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char SendingBuffer[BUFFER_SIZE+1] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

int StateLeft = LOW;
int StateRight = LOW;
int StateForward = LOW;
int StateReverse = LOW;


void setup()
{
  pinMode(PIN_LEFT, OUTPUT);
  pinMode(PIN_RIGHT, OUTPUT);
  pinMode(PIN_FORWARD, OUTPUT);
  pinMode(PIN_REVERSE, OUTPUT);

  digitalWrite(PIN_LEFT, LOW);
  digitalWrite(PIN_RIGHT, LOW);
  digitalWrite(PIN_FORWARD, LOW);
  digitalWrite(PIN_REVERSE, LOW);
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  Serial.println("WiFi232 control example");
  WiFiSerial.begin(9600);
}


void loop()
{
  boolean is_state_changed = false;
  
  if (WiFiSerial.available()>0)  {
    // Get incoming byte
    char in_byte = 0;
    in_byte = WiFiSerial.read();
    
    // Add received byte to buffer
    for(int i=0; i<BUFFER_SIZE-1; i++) {
      Buffer[i] = Buffer[i+1];
    }
    Buffer[BUFFER_SIZE-1] = in_byte;

    // Check condition
    if(Buffer[0] == CHECK_0 && Buffer[1] == CHECK_1 && Buffer[2] == CHECK_2) {
      Serial.println("Parse command : ");
      
      if((Buffer[3] & 0x02) > 0) {  // Check left on
        Serial.println("Cmd = Left on");
        StateLeft = HIGH;
        is_state_changed = true;
      } else if((Buffer[3] & 0x01) > 0) {  // Check left off
        Serial.println("Cmd = Left off");
        StateLeft = LOW;
        is_state_changed = true;
      } 
      if((Buffer[4] & 0x02) > 0) {  // Check right on
        Serial.println("Cmd = Right on");
        StateRight = HIGH;
        is_state_changed = true;
      } else if((Buffer[4] & 0x01) > 0) {  // Check right off
        Serial.println("Cmd = Right off");
        StateRight = LOW;
        is_state_changed = true;
      }
      if((Buffer[5] & 0x02) > 0) {  // Check forward on
        Serial.println("Cmd = Forward on");
        StateForward = HIGH;
        is_state_changed = true;
      } else if((Buffer[5] & 0x01) > 0) {  // Check forward off
        Serial.println("Cmd = Forward off");
        StateForward = LOW;
        is_state_changed = true;
      }
      if((Buffer[6] & 0x02) > 0) {  // Check reverse on
        Serial.println("Cmd = Reverse on");
        StateReverse = HIGH;
        is_state_changed = true;
      } else if((Buffer[6] & 0x01) > 0) {  // Check reverse off
        Serial.println("Cmd = Reverse off");
        StateReverse = LOW;
        is_state_changed = true;
      }
      
      Serial.println("Parsing completed.");
    }  // End of Check condition

  }  // End of if (WiFiSerial.available()>0)
  
  // Change LED
  if(is_state_changed) {
    digitalWrite(PIN_LEFT, StateLeft);
    digitalWrite(PIN_RIGHT, StateRight);
    digitalWrite(PIN_FORWARD, StateForward);
    digitalWrite(PIN_REVERSE, StateReverse);
    
    // Send result to remote
    SendingBuffer[0] = 0x22;    // start byte
    SendingBuffer[1] = 0x01;
    SendingBuffer[2] = 0x01;
    SendingBuffer[BUFFER_SIZE-1] = 0x24;  // end byte : checksum byte = start byte + (buffer[1] ~ buffer[6])
    
    if(StateLeft == HIGH) {
      SendingBuffer[3] = 0x02;
      SendingBuffer[BUFFER_SIZE-1] += 0x02;
    } else {
      SendingBuffer[3] = 0x01;
      SendingBuffer[BUFFER_SIZE-1] += 0x01;
    }
    if(StateRight == HIGH) {
      SendingBuffer[4] = 0x02;
      SendingBuffer[BUFFER_SIZE-1] += 0x02;
    } else {
      SendingBuffer[4] = 0x01;
      SendingBuffer[BUFFER_SIZE-1] += 0x01;
    }
    if(StateForward == HIGH) {
      SendingBuffer[5] = 0x02;
      SendingBuffer[BUFFER_SIZE-1] += 0x02;
    } else {
      SendingBuffer[5] = 0x01;
      SendingBuffer[BUFFER_SIZE-1] += 0x01;
    }
    if(StateReverse == HIGH) {
      SendingBuffer[6] = 0x02;
      SendingBuffer[BUFFER_SIZE-1] += 0x02;
    } else {
      SendingBuffer[6] = 0x01;
      SendingBuffer[BUFFER_SIZE-1] += 0x01;
    }

    Serial.print("Checksum = ");
    Serial.println(SendingBuffer[BUFFER_SIZE-1], HEX);
    Serial.println();
    WiFiSerial.print(SendingBuffer);
  }
  
}
