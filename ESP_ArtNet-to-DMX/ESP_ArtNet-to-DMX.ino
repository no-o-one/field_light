#include <WiFi.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>
#include <password_secrets.h>

#define ARTNET_OPDMX 0x5000

const char* ssid      = secret_ssid;
const char* password  = secret_password;


const int ARTNET_PORT     = 6454;
const int MAX_BUFFER      = 530;

const int DATA_POS        = 14;
const int DATA_NEG        = ;


const int UNIVERSE        = 1;
const int NET             = 0;
const int SUBNET          = 0;

uint8_t DMX_BUFFER [511] = {};
bool DMXBUF_writeprotect = false;

WiFiUDP udp;
uint8_t packetBuffer[MAX_BUFFER];



bool universeReceived = false;  // bool(ean) status array to track - initialized at false.
unsigned long lastFrameTime = 0;       //track when the previous frame was recieved - initialized at 0.
const int FRAME_TIMEOUT_MS = 1000;

void readpackets();
void updateleds();

TaskHandle_t Task_Handle1;
TaskHandle_t Task_Handle2;



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


// bool alluniversereceived() {
//   for (int i = 0; i < NUM_UNIVERSES; i++) {
//     if (!universereceived[i])
//         return false;
//   }
//   return true;
// }

// void handleArtDmx(uint8_t* buf, int len) {
//   if (len < 18) return;

//   uint16_t universe = buf[14] | (buf[15] << 8);
//   uint16_t dmxLen   = (buf[16] << 8) | buf[17];
//   uint8_t* dmx      = &buf[18];

//   //Serial.printf("  Got universe %d (%d/%d)\n", universe, relativeUniverse + 1, NUM_UNIVERSES);

//   int startLed = relativeUniverse; //which led does this univesrse start with
//   // Serial.print("startled");
//   // Serial.println(startLed);
//   for (int i = 0; i < ((int)(dmxLen)); i += 1) {
//     int ledIndex = startLed + (i / ADDR_SPACE);// always an int becuase incremented in fractions of 3
//     if (ledIndex >= 511) break;
//   }
    
//   int buffindex = 0;
//   for (int i = START_VAL;  i < (int)((NUM_LEDS * ADDR_SPACE)/XFERED);i++){
//     DMX_BUFFER[buffindex] = dmx[i];
//     buffindex++;
//   }
//   ledbuff_writeprotect = false;
//   // Serial.println("reached end of loop");
//   universereceived[relativeUniverse] = true;
//   lastFrameTime = millis();

//   //ADD DATA FLAG TO TELL IF OK TO WRITE TO NEOPIXELS OR NA

//   // make sure both universes recieved or else its fractured data
//   // if (alluniversereceived()) {
//   //leds.show();
//   //  memset(universereceived, 0, sizeof(universereceived));  // reset for next frame
//   //  }
// }

// void parseArtNet(uint8_t* buf, int len) {
//   if (len < 10) return;
//   if (memcmp(buf, "Art-Net\0", 8) != 0) return;

//   uint16_t opcode = buf[8] | (buf[9] << 8);
//   if (opcode == ARTNET_OPDMX) {
//     ledbuff_writeprotect = true;
//     handleArtDmx(buf, len);
//   }
// }

void setup() {
  Serial.begin(115200);
  delay(500);

  //initialize led buffer to all 0s
  for(int i=1;i<511;i++){
    DMX_BUFFER[i] = 0;
  }

  if (!connectWifi()) return;

  udp.begin(ARTNET_PORT);
  Serial.printf("Listening on port %d \n",ARTNET_PORT);
  pinMode(DATA_POS,OUTPUT);
  pinMode(DATA_NEG,OUTPUT);

  //xTaskCreate(readpackets,"read artnet packets and add data to buffer",10000,NULL,1,&Task_Handle1);
  //xTaskCreate(updateleds,"update leds",15000,NULL,2,&Task_Handle2);
}

void write_dmx(){
}
void begin_dmx(){

}
void end_dmx(){}







void loop() {

}

// void readpackets(void *pvParameters){
//   for(;;){
//     udp.begin(ARTNET_PORT);
//     int packetSize = udp.parsePacket();
//     if (packetSize) {
//       int len = udp.read(packetBuffer, MAX_BUFFER);
//       parseArtNet(packetBuffer, len);
//       lastFrameTime = millis(); // upd last packet recieve status
//       udp.begin(ARTNET_PORT);
//     }
//     delay(25); //give the cpu some time to not kill us
    
    
//     // timeout for dropped universes
//     bool anyReceived = false; // reset
//     for (int i = 0; i < NUM_UNIVERSES; i++) {
//       if (universereceived) { anyReceived = true; break; }
//     }

//     if (anyReceived && (millis() - lastFrameTime > FRAME_TIMEOUT_MS)) {
//       // Serial.println("timeout this is a fractured frame");
//       memset(universereceived, 0, sizeof(universereceived));
//     }
//     delay(25);
//   //   for(int i=0;i<NUM_LEDS;i++){
//   //     DMX_BUFFER[i] = 10;
//   // }
//   }
// }


// void updatedmx(void *pvParameters){
//   for(;;){
//     // for(int i=0;i<NUM_LEDS;i++){
//     //   DMX_BUFFER[i] = 10;
//     // }
//     if(ledbuff_writeprotect = false){
//       int k = 0;
//       for(int i=0;i<(int)(NUM_LEDS/XFERED);i++){
//         int index = i * 3;
//         for(int j=0;j<XFERED;j++){
//           leds.setPixelColor(((k*XFERED)+j),DMX_BUFFER[index],DMX_BUFFER[index+1],DMX_BUFFER[index+2]);
//         }
//         k++;
//       }
      
//       leds.show();
//       delay(25);
//     }
//     delay(25);
//   }
// }