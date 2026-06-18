#include <WiFi.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>

const char* ssid      = "i cant commit this shit to github";
const char* password  = "nad especially not this shit";

const int ARTNET_PORT    = 6454;
const int MAX_BUFFER     = 530;

const int LED_PIN        = 14;
const int NUM_LEDS       = 150;       
const int START_UNIVERSE = 0;


#define ARTNET_OPDMX 0x5000

const int NUM_UNIVERSES = 2;

WiFiUDP udp;
uint8_t packetBuffer[MAX_BUFFER];

Adafruit_NeoPixel leds(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);


bool universesReceived[2] = {false};  // boolean status array to track - initialized at false
unsigned long lastFrameTime = 0;       //track when the previous frame was recieve - initialized at 0
const int FRAME_TIMEOUT_MS = 100;    

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
    leds.Color(127, 0, 0),
    leds.Color(0, 127, 0),
    leds.Color(0, 0, 127),
    leds.Color(0, 0, 0)
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
    Serial.printf("  universe %d out of range\n", universe);
    return;
  }

  Serial.printf("  Got universe %d (%d/%d)\n", universe, relativeUniverse + 1, NUM_UNIVERSES);

  int startLed = relativeUniverse * 170; //whic led does this univesrse start with

  for (int i = 0; i + 2 < (int)dmxLen; i += 3) {
    int ledIndex = startLed + (i / 3);// always an int becuase incremented in fractions of 3
    if (ledIndex >= NUM_LEDS) break;
    leds.setPixelColor(ledIndex, dmx[i], dmx[i + 1], dmx[i + 2]);
  }

  // update recieve status of the unievrse
  universesReceived[relativeUniverse] = true;
  lastFrameTime = millis();

  // make sure both universes recieved or else its fractured data
  if (allUniversesReceived()) {
    leds.show();
    memset(universesReceived, 0, sizeof(universesReceived));  // reset for next frame
  }
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

  if (!connectWifi()) return;

  udp.begin(ARTNET_PORT);
  Serial.printf("Listening on port %d | %d LEDs across %d universes (u%d–u%d)\n",
    ARTNET_PORT, NUM_LEDS, NUM_UNIVERSES, START_UNIVERSE, START_UNIVERSE + NUM_UNIVERSES - 1);

  initTest();
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(packetBuffer, MAX_BUFFER);
    parseArtNet(packetBuffer, len);
    lastFrameTime = millis(); // upd last packet r ecieve status
  }
  // tineout for dropped universes
  bool anyReceived = false; // reset
  for (int i = 0; i < NUM_UNIVERSES; i++) {
    if (universesReceived[i]) { anyReceived = true; break; }
  }

  if (anyReceived && (millis() - lastFrameTime > FRAME_TIMEOUT_MS)) {
    Serial.println("timeout this is a fractured frame");
    leds.show();
    memset(universesReceived, 0, sizeof(universesReceived));
  }

}