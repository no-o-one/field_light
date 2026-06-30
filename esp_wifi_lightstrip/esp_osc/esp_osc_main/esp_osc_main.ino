#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

char ssid[] = "nope";          // your network SSID (name)
char pass[] = "nope";                    // your network password

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;
//const IPAddress outIp(10,40,10,105);        // remote IP (not needed for receive)
//const unsigned int outPort = 9999;          // remote port (not needed for receive)
const unsigned int localPort = 6000;        // local port to listen for UDP packets (here's where we send the packets)


OSCErrorCode error;
unsigned int ledState = LOW;              // LOW means led is *on*

#ifndef BUILTIN_LED
#ifdef LED_BUILTIN
#define BUILTIN_LED LED_BUILTIN
#else
#define BUILTIN_LED 13
#endif
#endif

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, ledState);    // turn *on* led

  Serial.begin(115200);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
#ifdef ESP32
  Serial.println(localPort);
#else
  Serial.println(Udp.localPort());
#endif

}

void handler(OSCMessage &msg, int offset){
  Serial.println("Match: string");
  //string multiple 'route' methods together using the pattern offset parameter. 
  msg.route("/preset", preset_handler, offset);
}

void preset_handler(OSCMessage &msg, int offset){
  Serial.println("Match: preset");
  //string multiple 'route' methods together using the pattern offset parameter. 
  msg.route("/0", preset1_handler, offset);
  msg.route("/1", preset1_handler, offset);
  msg.route("/2", preset2_handler, offset);
  msg.route("/3", preset3_handler, offset);
  msg.route("/4", preset4_handler, offset);
}

void preset1_handler(OSCMessage &msg, int offset){
  Serial.println("PRES1");
}
void preset2_handler(OSCMessage &msg, int offset){
  Serial.println("PRES2");
}
void preset3_handler(OSCMessage &msg, int offset){
  Serial.println("PRES3");
}
void preset4_handler(OSCMessage &msg, int offset){
  Serial.println("PRES4");
}


void loop() {
  OSCMessage msg;
  int size = Udp.parsePacket();

  if (size > 0) {
    while (size--) {
      msg.fill(Udp.read());
    }
    if (!msg.hasError()) {
      msg.route("/strip", handler);
    } else {
      error = msg.getError();
      Serial.print("error: ");
      Serial.println(error);
    }
  }
}