#include <U8glib.h>
#include <SoftwareSerial.h>

//----- OLED instance
// IMPORTANT NOTE: The complete list of supported devices 
// with all constructor calls is here: http://code.google.com/p/u8glib/wiki/device

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);	// I2C / TWI 

//----- HC-11 Control
SoftwareSerial hcSerial(2, 3); // HC-11's TX, RX

//----- Parsing
const char CHECK_0 = 0x55;
const char CHECK_1 = 0x01;
const char CHECK_2_1 = 0x01;
const char CHECK_2_2 = 0x02;
const char CHECK_END_1 = 0xFE;
const char CHECK_END_2 = 0xFF;

#define BUFFER_SIZE 8
static char Buffer[BUFFER_SIZE] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char SendingBuffer[BUFFER_SIZE+1] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

//----- Global variables
int Temperature = 0;
int Humidity = 0;
int AirQuality = 0;


void setup() {
  Serial.begin(9600);
  hcSerial.begin(9600);
  showInfo();
}


void loop()
{
  boolean is_state_changed = false;
  
  if (hcSerial.available()>0)  {
    // Get incoming byte
    char in_byte = 0;
    in_byte = hcSerial.read();
    
    // Add received byte to buffer
    for(int i=0; i<BUFFER_SIZE-1; i++) {
      Buffer[i] = Buffer[i+1];
    }
    Buffer[BUFFER_SIZE-1] = in_byte;

    // Check condition
    if(Buffer[0] == CHECK_0 && Buffer[1] == CHECK_1) {
      if(Buffer[2] == CHECK_2_1) {
        // Found temperature command
        Serial.println("Parse temp&humi command : ");
        
        if(Buffer[7] == CHECK_END_1) {   // Check validity
          Temperature = (Buffer[3] << 8) | Buffer[4];
          Humidity = (Buffer[5] << 8) | Buffer[6];
          Serial.print("Temp=");
          Serial.print(Temperature);
          Serial.print(", Humi=");
          Serial.println(Humidity);
          is_state_changed = true;
        }
        
        Serial.println("Parsing completed.");
      } else if(Buffer[2] == CHECK_2_2) {
        // Found air quality command
        Serial.println("Parse AirQ command : ");
        
        if(Buffer[7] == CHECK_END_2) {   // Check validity
          AirQuality = (Buffer[3] << 8) | Buffer[4];
          Serial.print("AirQ=");
          Serial.println(AirQuality);
          is_state_changed = true;
        }
        
        Serial.println("Parsing completed.");
      }

    }  // End of Check condition
  }  // End of if (hcSerial.available()>0)
  
  // Change LED
  if(is_state_changed) {
    // Show result on screen
    showInfo();
  }
  
}





//////////////////////////////////////////////////////
// Display

// Draw screen
const int DISP_CHAR_LEN = 11;
char str_line1[] = "Temp:      ";
char str_line2[] = "Humi:      ";
char str_line3[] = "AirQ:      ";

void showInfo() {
  // Init string buffer
  for(int i=0; i<DISP_CHAR_LEN; i++) {
    str_line1[i] = 0x00;
    str_line2[i] = 0x00;
    str_line3[i] = 0x00;
  }
  String dataString1 = "Temp: ";
  dataString1 += Temperature;
  dataString1 += "'C";
  dataString1.toCharArray(str_line1, DISP_CHAR_LEN);
  
  String dataString2 = "Humi: ";
  dataString2 += Humidity;
  dataString2 += "%";
  dataString2.toCharArray(str_line2, DISP_CHAR_LEN);
  
  String dataString3 = "AirQ: ";
  dataString3 += AirQuality;
  dataString3 += "";
  dataString3.toCharArray(str_line3, DISP_CHAR_LEN);
  
  // picture loop  
  u8g.firstPage();  
  do {
    // draw text
    u8g.setFont(u8g_font_fixed_v0);
    u8g.setFontRefHeightExtendedText();
    u8g.setDefaultForegroundColor();
    u8g.setFontPosTop();
    u8g.drawStr(20, 10, str_line1);
    u8g.drawStr(20, 26, str_line2);
    u8g.drawStr(20, 47, str_line3);
  } while( u8g.nextPage() );
}

