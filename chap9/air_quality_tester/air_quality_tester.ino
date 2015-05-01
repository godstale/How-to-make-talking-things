#include <EtherCard.h>

#define STATIC 0  // set to 1 to disable DHCP (adjust myip/gwip values below)
#if STATIC
// ethernet interface ip address
static byte myip[] = { 192,168,0,200 };
// gateway ip address
static byte gwip[] = { 192,168,0,1 };
#endif

// ethernet mac address - must be unique on your network
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };

byte Ethernet::buffer[500]; // tcp/ip send and receive buffer
BufferFiller bfill;

// Remember current air quality
int AirQuality = 0;

// Command
#define NUMBER_OF_COMMAND 5
int CmdBuffer[NUMBER_OF_COMMAND] = {0,0,0,0,0};

// Control LED for test
int LedPin = 4;
boolean LedStatus = false;


void setup(){
  pinMode(LedPin, OUTPUT);

  Serial.begin(9600);
  Serial.println("Start arduino air quality server");

  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) 
    Serial.println( "Failed to access Ethernet controller");
#if STATIC
  ether.staticSetup(myip, gwip);
#else
  if (!ether.dhcpSetup())
    Serial.println("DHCP failed");
#endif

  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);  
  ether.printIp("DNS: ", ether.dnsip);  
}


const char http_OK[] PROGMEM =
"HTTP/1.0 200 OK\r\n"
"Content-Type: text/html\r\n"
"Pragma: no-cache\r\n\r\n";

const char http_Found[] PROGMEM =
"HTTP/1.0 302 Found\r\n"
"Location: /\r\n\r\n";

const char http_Unauthorized[] PROGMEM =
"HTTP/1.0 401 Unauthorized\r\n"
"Content-Type: text/html\r\n\r\n"
"<h1>401 Unauthorized</h1>";

void homePage(){
  // Write your home page HTML code here.
  //$F represents p string, $D represents word or byte, $S represents string. 
  bfill.emit_p(PSTR("$F"
    "<meta http-equiv='refresh' content='10'/>"
    "<title>Ethercard LED</title>" 
    "Current Air Quality = $D</a><br/><br/>" 
    "Command $D = <a href=\"?cmd=$D$D\">$S</a><br/>" 
    "Command $D = <a href=\"?cmd=$D$D\">$S</a><br/>" 
    "Command $D = <a href=\"?cmd=$D$D\">$S</a><br/>" 
    "Command $D = <a href=\"?cmd=$D$D\">$S</a><br/>" 
    "Command $D = <a href=\"?cmd=$D$D\">$S</a><br/>"),
  http_OK,     // HTTP header string
  AirQuality,  // Air quality value from sensor
  (word)1,  1,  (CmdBuffer[0]==0)?1:0,  (CmdBuffer[0]==1)?"ON":"OFF",    // Command 1
  (word)2,  2,  (CmdBuffer[1]==0)?1:0,  (CmdBuffer[1]==1)?"ON":"OFF",
  (word)3,  3,  (CmdBuffer[2]==0)?1:0,  (CmdBuffer[2]==1)?"ON":"OFF",
  (word)4,  4,  (CmdBuffer[3]==0)?1:0,  (CmdBuffer[3]==1)?"ON":"OFF",
  (word)5,  5,  (CmdBuffer[4]==0)?1:0,  (CmdBuffer[4]==1)?"ON":"OFF"); 
}

void loop(){
  // Check air quality
  boolean turnOnLed = false;
  int sensorValue = analogRead(A0);  // read data from sensor
  AirQuality = (int)((float)sensorValue/1024*500);
  
  if(AirQuality > 35)
    turnOnLed = true;
    
  // DHCP expiration is a bit brutal, because all other ethernet activity and
  // incoming packets will be ignored until a new lease has been acquired
  //  if (!STATIC /*&& !ether.dhcpLease()*/) {
  //    Serial.println("Acquiring DHCP lease again");
  //    ether.dhcpSetup();
  //  }

  // wait for an incoming TCP packet, but ignore its contents
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len); 
  if (pos) {
    delay(1);   // necessary for older arduino
    bfill = ether.tcpOffset();
    char *data = (char *) Ethernet::buffer + pos;
    
    if (strncmp("GET /", data, 5) != 0) {
      // Unsupported HTTP request
      // 304 or 501 response would be more appropriate
      bfill.emit_p(http_Unauthorized);
    } else {
        data += 5;
        if (data[0] == ' ') {    //Check if the home page, i.e. no URL
          homePage();
        } else if (!strncmp("?cmd=",data,5)){   //Check if a url which changes the command has been recieved
          data += 5;
          char tempCmd[3] = {0};
          strncpy(tempCmd, data, 2);
          tempCmd[2] = 0x00;
          
          if(tempCmd[0] == '1') {
            if(tempCmd[1] == '0') 
              CmdBuffer[0] = 0;
            else
              CmdBuffer[0] = 1;
          } else if(tempCmd[0] == '2') {
            if(tempCmd[1] == '0')
              CmdBuffer[1] = 0;
            else
              CmdBuffer[1] = 1;
          } else if(tempCmd[0] == '3') {
            if(tempCmd[1] == '0')
              CmdBuffer[2] = 0;
            else
              CmdBuffer[2] = 1;
          } else if(tempCmd[0] == '4') {
            if(tempCmd[1] == '0')
              CmdBuffer[3] = 0;
            else
              CmdBuffer[3] = 1;
          } else if(tempCmd[0] == '5') {
            if(tempCmd[1] == '0')
              CmdBuffer[4] = 0;
            else
              CmdBuffer[4] = 1;
          }
          
          Serial.print("Received command = ");
          Serial.println(tempCmd);

          bfill.emit_p(http_Found);
        } else {    // Otherwise, page isn't found
          // Page not found
          bfill.emit_p(http_Unauthorized);
        }
      }
      ether.httpServerReply(bfill.position());    // send http response
  }  // End of if(pos)
  
  // Do what you want
  if( (turnOnLed || CmdBuffer[0] != 0) 
      && LedStatus != true) {
    // turn on LED
    digitalWrite(LedPin, HIGH);
    LedStatus = true;
  } else if(LedStatus != false) {
    // turn on LED
    digitalWrite(LedPin, LOW);
    LedStatus = false;
  }
  
}
