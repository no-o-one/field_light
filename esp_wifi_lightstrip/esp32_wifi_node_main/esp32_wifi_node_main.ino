#include <WiFi.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>
#include <password_secrets.h>

const char* ssid      = secret_ssid;
const char* password  = secret_password;


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

#define ARTNET_OPDMX 0x5000

const int NUM_UNIVERSES = 2;

WiFiUDP udp;
uint8_t packetBuffer[MAX_BUFFER];

Adafruit_NeoPixel leds(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);


bool universesReceived[2] = {false};  // bool(ean) status array to track - initialized at false.
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

void initTest() {
  uint32_t colors[] = {
    // leds.Color(255, 0, 0),
    // leds.Color(0, 255, 0),
    // leds.Color(0, 0, 255),
    // leds.Color(0, 0, 0),
    leds.Color(255, 255, 255)
    
  };
  for (uint32_t c : colors) {
    for (int i = 0; i < NUM_LEDS; i++) leds.setPixelColor(i, c);
    leds.show();
    delay(400);
  }
}

bool allUniversesReceived() {
  for (int i = 0; i < NUM_UNIVERSES; i++) {
    if (!universesReceived[i])
        return false;
  }
  return true;
}

void handleArtDmx(uint8_t* buf, int len) {
  if (len < 18) return;

  uint16_t universe = buf[14] | (buf[15] << 8);
  uint16_t dmxLen   = (buf[16] << 8) | buf[17];
  uint8_t* dmx      = &buf[18];

  int relativeUniverse = universe - START_UNIVERSE;

  if (relativeUniverse < 0 || relativeUniverse >= NUM_UNIVERSES) {
    //Serial.printf("  universe %d out of range\n", universe);
    return;
  }

  //Serial.printf("  Got universe %d (%d/%d)\n", universe, relativeUniverse + 1, NUM_UNIVERSES);

  int startLed = relativeUniverse; //which led does this univesrse start with
  // Serial.print("startled");
  // Serial.println(startLed);
  for (int i = startLed; i + 2 < ((int)(dmxLen)); i += (ADDR_SPACE * XFERED)) {
    int ledIndex = startLed + (i / ADDR_SPACE);// always an int becuase incremented in fractions of 3
    if (ledIndex >= MAX_INDEX) break;
  }
    
  int buffindex = 0;
  for (int i = START_VAL;  i < (int)((NUM_LEDS * ADDR_SPACE)/XFERED);i++){
    led_buffer[buffindex] = dmx[i];
    buffindex++;
  }
  // Serial.println("reached end of loop");
  universesReceived[relativeUniverse] = true;
  lastFrameTime = millis();

  //ADD DATA FLAG TO TELL IF OK TO WRITE TO NEOPIXELS OR NA

  // make sure both universes recieved or else its fractured data
  // if (allUniversesReceived()) {
  //leds.show();
  //  memset(universesReceived, 0, sizeof(universesReceived));  // reset for next frame
  //  }
}

void parseArtNet(uint8_t* buf, int len) {
  if (len < 10) return;
  if (memcmp(buf, "Art-Net\0", 8) != 0) return;

  uint16_t opcode = buf[8] | (buf[9] << 8);
  if (opcode == ARTNET_OPDMX) {
    handleArtDmx(buf, len);
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  leds.begin();
  leds.clear();
  leds.show();

  //initialize led buffer to all 0s
  for(int i=0;i<NUM_LEDS;i++){
    led_buffer[i] = 0;
  }

  if (!connectWifi()) return;

  udp.begin(ARTNET_PORT);
  Serial.printf("Listening on port %d | %d LEDs across %d universes (u%d–u%d)\n",
    ARTNET_PORT, NUM_LEDS, NUM_UNIVERSES, START_UNIVERSE, START_UNIVERSE + NUM_UNIVERSES - 1);
  initTest();

  xTaskCreate(readpackets,"read artnet packets and add data to buffer",10000,NULL,1,&Task_Handle1);
  xTaskCreate(updateleds,"update leds",15000,NULL,2,&Task_Handle2);
}



void loop() {}

void readpackets(void *pvParameters){
  for(;;){
    // udp.begin(ARTNET_PORT);
    // int packetSize = udp.parsePacket();
    // if (packetSize) {
    //   // Serial.print("Received packet of size ");
    //   // Serial.println(packetSize);
    //   int len = udp.read(packetBuffer, MAX_BUFFER);
    //   parseArtNet(packetBuffer, len);
    //   lastFrameTime = millis(); // upd last packet recieve status
    //   udp.begin(ARTNET_PORT);
    // }
    // delay(25);
    // // timeout for dropped universes
    // bool anyReceived = false; // reset
    // for (int i = 0; i < NUM_UNIVERSES; i++) {
    //   if (universesReceived[i]) { anyReceived = true; break; }
    // }

    // if (anyReceived && (millis() - lastFrameTime > FRAME_TIMEOUT_MS)) {
    //   // Serial.println("timeout this is a fractured frame");
    //   memset(universesReceived, 0, sizeof(universesReceived));
    // }
    delay(25);
  //   for(int i=0;i<NUM_LEDS;i++){
  //     led_buffer[i] = 10;
  // }
  }
}
void updateleds(void *pvParameters){
  for(;;){
    for(int i=0;i<NUM_LEDS;i++){
      led_buffer[i] = 10;
    }
    if(true){
      int k = 0;
      for(int i=0;i<(int)(NUM_LEDS/XFERED);i+=ADDR_SPACE){
        for(int j=0;j<XFERED;j++){
          leds.setPixelColor(((k*XFERED)+j),led_buffer[i],led_buffer[i+1],led_buffer[i+2]);
        }
        k++;
      }
      
      leds.show();
      delay(25);
    }
    
  }
}