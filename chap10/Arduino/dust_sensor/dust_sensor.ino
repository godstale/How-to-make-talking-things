#include <U8glib.h>
#include "bitmap.h"
#include <EtherCard.h>

///////////////////////////////////////////////////////////////////
//----- Ethernet instance

// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
byte Ethernet::buffer[400];
static uint32_t timer;

// Destination domain - do not use IP
char website[] PROGMEM = "api.thingspeak.com";    // use your website domain
Stash stash;

//timing variable
int res = 0;

///////////////////////////////////////////////////////////////////
//----- OLED instance
// IMPORTANT NOTE: The complete list of supported devices 
// with all constructor calls is here: http://code.google.com/p/u8glib/wiki/device

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);	// I2C / TWI 


///////////////////////////////////////////////////////////////////
// Dust sensor 
/*
 Interface to Sharp GP2Y1010AU0F Particle Sensor
 Program by Christopher Nafis 
 Written April 2012
 
 http://www.sparkfun.com/datasheets/Sensors/gp2y1010au_e.pdf
 http://sensorapp.net/?p=479
 
 Sharp pin 1 (V-LED)   => 5V (connected to 150ohm resister)
 220uF capacitor between pin1 and pin2
 Sharp pin 2 (LED-GND) => Arduino GND pin
 Sharp pin 3 (LED)     => Arduino pin 2
 Sharp pin 4 (S-GND)   => Arduino GND pin
 Sharp pin 5 (Vo)      => Arduino A0 pin
 Sharp pin 6 (Vcc)     => 5V
 */
 
int DustPin = 0;
int DustVal=0;  // analog read
int count=0;
float ppm=0;    // sum of analog read
char s[32];
float voltage = 0;  // average voltage
float DustDensity = 0;  // average dust density
float PpmPercf = 0;  // 
unsigned long LastSensingTime = 0;
const unsigned long SENSING_INTERVAL = 3000;

int LedPower=2;
int DelayTime=280;
int DelayTime2=40;
float OffTime=9680;

unsigned long LastConnectionTime = 0;        // last time you connected to the server, in milliseconds
const unsigned long POSTING_INTERVAL = 30*1000;  //delay between updates to Pachube.com


void setup() {
  pinMode(LedPower,OUTPUT);    // Init dust sensor's Infrared
  
  //Initialize Ethernet
  initialize_ethernet();
}
 
void loop(){
  // Monitoring incoming packets
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len); 
  
  // Read dust sensor 
  if(millis() - LastSensingTime > SENSING_INTERVAL) {
    readDustSensor();
    LastSensingTime = millis();
  }

  if(millis() - LastConnectionTime > POSTING_INTERVAL) {
    // calculate average voltage and dust density
    voltage = ppm/count*0.0049;
    DustDensity = 0.17*voltage-0.1;
    PpmPercf = (voltage-0.0256)*120000;
    if (PpmPercf < 0)
      PpmPercf = 0;
    if (DustDensity < 0 )
      DustDensity = 0;
    if (DustDensity > 0.5)
      DustDensity = 0.5;
    
    // display result
    String dataString = "";
    dataString += dtostrf(DustDensity, 5, 4, s);
    dataString += " mg/m3";
    showInfo(dataString, 2);
    
    // send to remote server
    sendData();

    // initialize parameters
    count=0;
    ppm=0;
    LastConnectionTime = millis();
  }

}


//////////////////////////////////////////////////////
// Read analog voltage value from dust sensor

void readDustSensor() {
  count = count + 1;
  // LedPower is any digital pin on the arduino connected to Pin 3 on the sensor
  digitalWrite(LedPower,LOW); // power on the LED
  delayMicroseconds(DelayTime);
  
  DustVal=analogRead(DustPin); // read the dust value via pin 5 on the sensor
  ppm = ppm+DustVal;
  delayMicroseconds(DelayTime2);

  digitalWrite(LedPower,HIGH); // turn the LED off
  delayMicroseconds(OffTime);
  
  String strVoltage = "Voltage=";
  strVoltage += dtostrf(DustVal, 5, 2, s);
  showInfo(strVoltage, 3);
}

//////////////////////////////////////////////////////
// Ethernet functions

#define MAX_URL_LENGTH 51
char url_buf[MAX_URL_LENGTH];

// Send data to remote server
void sendData() {
  // initialize buffer
  for(int i=0; i<MAX_URL_LENGTH; i++)
    url_buf[i] = 0x00;
  
  // make request parameters
  String strUrl = "update?key=WRXUSJ4S7G0LXWGS&field1=";
  strUrl += dtostrf(DustDensity, 5, 4, s);
  strUrl.toCharArray(url_buf, MAX_URL_LENGTH - 1);
  url_buf[MAX_URL_LENGTH - 1] = 0x00;

  // send request
  ether.browseUrl(PSTR("/"), url_buf, website, my_callback);
}

void sendDataUsingPost() {
  // generate two fake values as payload - by using a separate stash,
  // we can determine the size of the generated message ahead of time
  byte sd = stash.create();
  stash.print("field1=");
  stash.print(String(DustDensity, DEC));
  stash.save();
  
  // generate the header with payload - note that the stash size is used,
  // and that a "stash descriptor" is passed in as argument using "$H"
  Stash::prepare(PSTR("POST /update HTTP/1.1" "\r\n" 
                      "Host: $F" "\r\n" 
                      "Connection: close" "\r\n" 
                      "X-THINGSPEAKAPIKEY: $F" "\r\n" 
                      "Content-Type: application/x-www-form-urlencoded" "\r\n" 
                      "Content-Length: $D" "\r\n" 
                      "\r\n" 
                      "$H"),
          website, PSTR("WRXUSJ4S7G0LXWGS"), stash.size(), sd);

  // send the packet - this also releases all stash buffers once done
  ether.tcpSend();
}

// called when the client request is complete
static void my_callback (byte status, word off, word len) {
  Ethernet::buffer[off+300] = 0;
  showInfo(">>> Received", 3);
  //showInfo((char*) Ethernet::buffer + off);
  //delay(5000);
}

// Initialize ethernet module
void initialize_ethernet(void){  
  for(;;){ // keep trying until you succeed 
    if (ether.begin(sizeof Ethernet::buffer, mymac) == 0){ 
      showInfo("Failed to access Ethernet controller", 3);
      delay(3000);
      continue;
    }
    showInfo("Ethernet begin", 3);
    
    if (!ether.dhcpSetup()){
      showInfo("DHCP failed", 3);
      delay(3000);
      continue;
    }

    ether.printIp("IP:  ", ether.myip);
    ether.printIp("GW:  ", ether.gwip);  
    ether.printIp("DNS: ", ether.dnsip);  

    if (!ether.dnsLookup(website)) {
      showInfo("DNS failed", 3);
      
      // Force DNS to OpenDNS 208.67.222.222, 208.67.220.220
      // Let's choose the other one : 208.67.220.220
      ether.dnsip[0] = 168;
      ether.dnsip[1] = 126;
      ether.dnsip[2] = 63;
      ether.dnsip[3] = 1;
      
      //ether.printIp("IP:  ", ether.dnsip);
      if (!ether.dnsLookup(website)) {
        showInfo("DNS retry failed", 3);
        delay(5000);
        continue;
      } else {
        showInfo("DNS success", 3);
      }
      delay(5000);
    }

    showInfo("Ethernet initialized!!", 3);
    delay(3000);

    //reset init value
    res = 0;
    break;
  }
}


//////////////////////////////////////////////////////
// Display

// Draw screen
const int DISP_CHAR_LEN = 13;
const int DISP_CHAR_LEN2 = 24;
char str_line1[] = "Dust density ";
char str_line2[] = "00.0000 mg/m3";
char str_line3[] = "Initializing............";

void showInfo(String strDisp, int line_index) {
  if(strDisp == NULL) {
    return;
  }
  
  int posY = 45;
  if(line_index == 1) {
    for(int i=0; i<DISP_CHAR_LEN; i++)
      str_line1[i] = 0x00;
    strDisp.toCharArray(str_line1, DISP_CHAR_LEN - 1);
  } else if(line_index == 2) {
    for(int i=0; i<DISP_CHAR_LEN; i++)
      str_line2[i] = 0x00;
    strDisp.toCharArray(str_line2, DISP_CHAR_LEN - 1);
  } else {
    for(int i=0; i<DISP_CHAR_LEN2; i++)
      str_line3[i] = 0x00;
    strDisp.toCharArray(str_line3, DISP_CHAR_LEN2 - 1);
  }
  
  // picture loop  
  u8g.firstPage();  
  do {
    // draw icon
    u8g.drawBitmapP( 13, 11, 3, 24, IMG_logo_24x24);
    
    // draw text
    u8g.setFont(u8g_font_fixed_v0);
    u8g.setFontRefHeightExtendedText();
    u8g.setDefaultForegroundColor();
    u8g.setFontPosTop();
    u8g.drawStr(50, 10, str_line1);
    u8g.drawStr(50, 26, str_line2);
    u8g.drawStr(8, 47, str_line3);
  } while( u8g.nextPage() );
}

