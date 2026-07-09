#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
// #include <Adafruit_NeoPixel.h>

EthernetUDP udp;

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

const int ARTNET_PORT    = 6454;
const int MAX_BUFFER     = 530;

const int LED_PIN        = 14;
const int NUM_LEDS       = 300;
const int START_UNIVERSE = 0;
const int MAX_INDEX = 1024;

uint8_t XFERED = 10; //how many leds are combined into one (how many pixels per channel)
uint8_t ADDR_SPACE = 3; //how many address each channel takes up (for RGB LEDS ALWAYS 3 unless we want one color leds)
const int START_VAL = 0; //first address we want to read

uint8_t led_buffer [NUM_LEDS] = {};
bool ledbuff_writeprotect = false;

//static address backups
#define MYIPADDR 192,168,1,28
#define MYIPMASK 255,255,0,0
#define MYDNS 192,168,1,1
#define MYGW 192,168,1,1

#define ARTNET_OPDMX 0x5000

uint8_t packetBuffer[MAX_BUFFER];

// Adafruit_NeoPixel leds(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);


bool universesReceived[2] = {false};  // bool(ean) status array to track - initialized at false.
unsigned long lastFrameTime = 0;       //track when the previous frame was recieved - initialized at 0.
const int FRAME_TIMEOUT_MS = 1000; 

void readpackets();
void updateleds();

// TaskHandle_t Task_Handle1;
// TaskHandle_t Task_Handle2;




/////////////////////////////DEALING WITH ARTNET/////////////////////////////


/////////////////////////////ON BOOT TESTING CODE/////////////////////////////

// void initTest() {
//   uint32_t colors[] = {
//     // leds.Color(255, 0, 0),
//     // leds.Color(0, 255, 0),
//     // leds.Color(0, 0, 255),
//     // leds.Color(0, 0, 0),
//     leds.Color(255, 255, 255)
    
//   };
//   for (uint32_t c : colors) {
//     for (int i = 0; i < NUM_LEDS; i++) leds.setPixelColor(i, c);
//     leds.show();
//     delay(400);
//   }
// }

bool connectethernet (){
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }else{
      IPAddress ip(MYIPADDR);
      IPAddress dns(MYDNS);
      IPAddress gw(MYGW);
      IPAddress sn(MYIPMASK);
      Ethernet.begin(mac, ip, dns, gw, sn);
      Serial.println("failed to use DHCP using Static address");
      return false;
    }
    delay(100);
    return true;
    }else{
      Serial.print("ETHERNET CONNECTED @");
      Serial.print(Ethernet.localIP().toString().c_str());
      return false;
    }

}


/////////////////////////////SETUP//////////////////////////////////

void setup() {
  memset(led_buffer, 0, sizeof(led_buffer));

  Serial.begin(115200);
  Ethernet.init(17); 

  //inital led testing
  // leds.begin();
  // leds.clear();
  // leds.show();
  // initTest();

  Serial.println("attempting to connect via ethernet");
  
  while(true)if (!connectethernet()) return;

  udp.begin(ARTNET_PORT);



  //rtos tasks

  // xTaskCreate(readpackets,"read artnet packets and add data to buffer",10000,NULL,1,&Task_Handle1); 
  // xTaskCreate(updateleds,"update leds",15000,NULL,2,&Task_Handle2);
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    Serial.println("gotpacket");
    udp.begin(ARTNET_PORT);
  } 
}
