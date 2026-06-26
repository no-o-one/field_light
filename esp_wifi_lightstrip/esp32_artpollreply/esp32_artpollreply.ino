#include <Arduino.h>
#include "WiFi.h"
#include "AsyncUDP.h"
#include "Network.h"

//const uint256_t fakeartpol = 0x000000000000000000000000000000000000000000000000000008008B01A8C00AD089DF5F6E0000000000000000030001000002000080008000080008C0C0C0C00400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000414C4F205D325B203130303023000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000065646F6E2074654E747241202D20414C4F;
const char* ssid = "jinx wtf";
const char* password = "jinx wtf";

const uint64_t artnet_id = 0x4172742d4e657400; //hex for artnet
const uint16_t apr_opcode = 0x0021; //
const uint16_t artnet_port = 0x3619;
const uint16_t firmware_version = 0;

const uint16_t apr_switch = 0; //temp
const uint8_t input_universes [4] = {0,1,2,3}; //list the univeres for each input port
const uint8_t output_universes [4] = {0,1,2,3}; //same as input change if your crazy and want to output on some crazy universes

const uint16_t apr_oemcode = 0x7FF0; //code reserved for prototyping/experimental use
const uint8_t apr_ubeaversion = 0; //not supported change if UBEA is supported
const uint8_t apr_status1 = 0xd2;
const uint16_t apr_estaman = 0x2866; //code reserved starlight xnet4 (a 4 in/out dmx node)
uint8_t apr_portname [18] = {0x4a,0x49,0x4e,0x58,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint8_t apr_longname [64] = {0X4A,0X49,0X4E,0X58,0X20,0X41,0X4E,0X44,0X20,0X41,0X4C,0X49,0X53,0X41,0X27,0X53,0X20,0X53,0X55,0X50,0X45,0X52,0X20,0X43,0X4F,0X4F,0X4C,0X20,0X41,0X4E,0X44,0X20,0X4E,0X4F,0X54,0X20,0X41,0X54,0X20,0X41,0X4C,0X4C,0X20,0X4A,0X41,0X4E,0X4B,0X20,0X44,0X4D,0X58,0X20,0X4E,0X4F,0X44,0X45,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00};
const uint16_t apr_numports = 4; //high byte first
const uint8_t apr_porttypes = 0xc0; //all ports are for artnet only & can only take input
const uint8_t apr_goodinput = 0b01110000; //hardcoded to say no issues
const uint8_t apr_goodoutput = 0b11110000; //hardcoded to say no issues
uint8_t apr_swin [4] = {0,0,0,0}; //bits 3-0 for each input port into the low nibble
const uint8_t apr_swout [4] = {0,0,0,0}; //bits 3-0 for each output port into the low nibble
const uint8_t apr_acnpriority = 0; //we don't support sACN
const uint8_t apr_swmacro = 0b00000000; //macroinputs not accepted at the moment
const uint8_t apr_swremote = 0b0000000; //remoteinputs not accepted at the moment
const uint8_t apr_style = 0x00; //we are a dmx node not a console
const uint8_t apr_bindindex = 0; //we are a root device make higher for further up the chain of commmand
const uint8_t apr_status2 = 0x08; //hardcoded to say no issues
const uint8_t apr_status3 = 0b00000000; //hardcoded to say no issues
const uint64_t apr_deafultresponduid = 0;
const uint16_t apr_userdata = 0;
const uint16_t apr_refreshrate = 0x000f;

uint32_t nodestatus = 0x30303031;
uint16_t apr_counter = 0;

uint8_t mac_addr[6];




NetworkUDP udp;
IPAddress ip;








void artpollreply(){
  udp.beginPacket(IPAddress(192,168,1,255),0x3619);
  
  //id hardcoded 8 bytes
  for(int i=7;i>=0;i--){
    udp.write(((uint8_t*)(&artnet_id))[i]);
  }
  
  udp.write(((uint8_t*)(&apr_opcode))[1]);//udp.write(*(((uint8_t*)apr_opcode)+1)); //opcode lo
  udp.write(((uint8_t*)(&apr_opcode))[0]); //opcode hi

  
  //our ip address 4 bytes
  udp.write(192);
  udp.write(168);
  udp.write(1);
  udp.write(140);


  udp.write(((uint8_t*)(&artnet_port))[1]);//artnet port lo
  udp.write(((uint8_t*)(&artnet_port))[0]);//artnet port hi
  
  udp.write(((uint8_t*)(&firmware_version))[0]);//versinfoH high byte of firmware revision number
  udp.write(((uint8_t*)(&firmware_version))[1]);//versiofoL
  
  //netswitch bits 14-8 of the 15 bit port addess into the bottom 7 bits
  //subswitch bits 7-4 of the 15 bit port addess into the bottom 4 bits



  
  
  //TEMP
  for(int i=1;i<=2;i++){
    udp.write(0);
  }



  udp.write(((uint8_t*)(&apr_oemcode))[1]);//eom lo
  udp.write(((uint8_t*)(&apr_oemcode))[0]);//eom hi
  
  udp.write(apr_ubeaversion);//ubea version hard code to 0
  
  udp.write(apr_status1);//status 1: 11110000. (00110000 if dead)


  udp.write(((uint8_t*)(&apr_estaman))[0]);//estaman hi
  udp.write(((uint8_t*)(&apr_estaman))[1]);//estaman lo
  
  for(int i=0;i<=17;i++){
    udp.write(apr_portname[i]);
  }

  //longname
  for(int i=0;i<=63;i++){
    udp.write(apr_longname[i]);
  }


  //node report #xxxx [yyyy] zzzzzz... x is hex code y is counter incrementing and z plain text
  //xxxx
  udp.write(0x23);
  for(int i=3;i>=0;i--){
    udp.write(((uint8_t*)(&nodestatus))[i]);//i hate this the xxxx needs to be in the form of hex that will be parsed as asci then read as hex :(
  }
  //yyyy
  udp.write(0x20);
  udp.write(0x5b);
  udp.write(0x31);
  udp.write(0x5d);
  udp.write(0x20);
  //zzzz for now we only print 0's otherwise 60 char plain tex status message
  for(int i=1;i<=54;i++){
    udp.write(0x41);
  }
  
  udp.write(((uint8_t*)(&apr_numports))[1]);//numports hi
  udp.write(((uint8_t*)(&apr_numports))[0]);//numports lo
  
  //
  //port types
    for(int i=1;i<=4;i++){
      udp.write(apr_porttypes);
    }
  //goodinput
    for(int i=1;i<=4;i++){
      udp.write(apr_goodinput);
    }
  //goodoutputA
    for(int i=1;i<=4;i++){
      udp.write(apr_goodoutput);
    }
  
  
  
  
  //
  
  
  //swin (4) bits 3-0 of 15 bit port address for each of the 4 possible input ports
  //swout (4) bits 3-0 of 15 bit port address for each of the 4 possible output ports
  
  
  //swin (4) bits 3-0 of 15 bit port address for each of the 4 possible input ports (its just the universe number of each port)
  for(int i=0;i<=3;i++){
    udp.write(input_universes[i]);
  }

  //swout (4) bits 3-0 of 15 bit port address for each of the 4 possible output ports (its just the universe number of each port)
  for(int i=0;i<=3;i++){
    udp.write(output_universes[i]);
  }



  udp.write(apr_acnpriority);//acnpriorty hard code to 0 cuz we hate sACN
  
  udp.write(apr_swmacro);//swmacro do we support macros (LOL no) all 0
  
  udp.write(apr_swremote);//swremote do we support remote trigger inputs (LOL no) all 0
  
  //3 spare unused bytes hardcode to 0
  for(int i=1;i<=3;i++){
    udp.write(0);
  }

  udp.write(apr_style); //style style code of device
  
  //print out mac address
  for(int i=0;i<=5;i++){
    udp.write(mac_addr[i]);
  }
  
  //no bind ip so just 0
  for(int i=0;i<=3;i++){
    udp.write(0x00);
  }
  
  udp.write(apr_bindindex); //bind index 0 cuz we are root device

  udp.write(apr_status2); //status 2 00101110
  
  //goodoutputB 11110000
  for(int i=0;i<=3;i++){
    udp.write(apr_goodoutput); 
  }
  
  
  udp.write(apr_status3); //status 3 00000000
  
  //defaultrespUID
  for(int i=7;i>=2;i--){
    udp.write(((uint8_t*)(&apr_deafultresponduid))[i]);
  }
    
  udp.write(((uint8_t*)(&apr_userdata))[0]);//userdata hi
  udp.write(((uint8_t*)(&apr_userdata))[1]);//userdata lo


  udp.write(((uint8_t*)(&apr_refreshrate))[0]);//refreshrate hi
  udp.write(((uint8_t*)(&apr_refreshrate))[1]);//refreshrate lo

  //empty byte filler at the end
  for(int i=1;i<=10;i++){
    udp.write(0);
  }
  udp.endPacket();
  //increment counter for debuging
  apr_counter += 1;
  if(apr_counter >= 10000){
    apr_counter = 0;
  }

}
bool connectWifi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i++ > 20){
        Serial.println("\n failed to connect wifi");
        return false;
    }
  }
  Serial.printf("\n wifi conected @ %s\n", WiFi.localIP().toString().c_str());
  return true;
}
void setup() {
  Network.begin();
  Serial.begin(115200);
  if (!connectWifi()) return;
  Network.macAddress(mac_addr);
}
void loop() {
  artpollreply(); 
  delay(1000);
  Serial.println(WiFi.localIP());
}
