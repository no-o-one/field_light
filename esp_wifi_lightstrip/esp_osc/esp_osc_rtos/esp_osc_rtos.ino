#include <stdio.h>
#include <string.h> // For memset
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>
#include <Adafruit_NeoPixel.h>
#include <stdlib.h>


//core 0 - wifi ans sys stuff
//core 1 - appcore

char ssid[] = "Field_Building2.4G";        
char pass[] = "helloguys";  

uint8_t SELF_ID = 0;       ///////////// STRIP SPECS       
const int LED_PIN        = 14;
const int NUM_LEDS       = 300;    

WiFiUDP Udp;

const unsigned int localPort = 6000;        // local port to listen for UDP packets (here's where we send the packets)

OSCErrorCode error;
unsigned int ledState = LOW;              // LOW means led is *on*

TaskHandle_t anim_task_handles[12] = {NULL};

Adafruit_NeoPixel leds(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

#define LED_BUILTIN 13

void kill_old_tasks(){
  for(int i = 0; i < 12; i ++){
    if (anim_task_handles[i] != NULL){
      vTaskDelete(anim_task_handles[i]);
      anim_task_handles[i] = NULL;
    }
  }
}

//////////////////////////////ANIMATIONS ///////////////////////////

void preset0_anim_player(void *parameter) {
  for (;;) {
    leds.clear();
    leds.show();
    vTaskDelay(30000);
  }
}

void preset1_anim_player(void *parameter) {
  for (;;) {
    for (int i = 0; i < NUM_LEDS; i++) leds.setPixelColor(i, leds.Color(255,255,255));
    leds.show();
    vTaskDelay(30000);
  }
}

void preset2_anim_player(void *parameter) {
  char* param;
  param = (char *) parameter;
  int numledstolight = atoi(param);
  
  leds.clear();
  for(;;){
    for (int i = 0; i < numledstolight; i++) leds.setPixelColor(NUM_LEDS-i, leds.Color(255,255,255));
    leds.show();
    vTaskDelay(30000);
  }
}

void preset3_anim_player(void *parameter) {
  char* param;
  param = (char *) parameter;
  int numledstolight = atoi(param);
  
  leds.clear();

  int j = 50;
  bool isincr = true;
  for(;;){  
    for (int i = 0; i < numledstolight; i++) leds.setPixelColor(NUM_LEDS-i, leds.Color(j, j, j));
    leds.show();

    if(j == 50){
      isincr = true;
    }
    if( j == 255){
      isincr = false;
    }
    if(isincr){
      j = j + 1;
    }else{
      j = j -1;
    }
    vTaskDelay(5);
  }
}

void preset10_anim_player(void *parameter) {
  char* param;
  param = (char *) parameter;
  int groupof = atoi(param);

  leds.clear();
  for (int i = -1*groupof; i < NUM_LEDS ; i++){
    for(int j = 0; j < groupof; j++){
      if (i+j >= 0 && i+j < NUM_LEDS){
         leds.setPixelColor(i+j, leds.Color(255,255,255));
      }
    }
    leds.show();
    vTaskDelay(10);
  } 


  for (;;) {
    vTaskDelay(30000);
  }
}

void preset11_anim_player(void *parameter) {
  char* param;
  param = (char *) parameter;
  int groupof = atoi(param);

  leds.clear();
  for (int i = -1*groupof; i < NUM_LEDS ; i++){
    leds.clear();
    for(int j = 0; j < groupof; j++){
      if (i+j >= 0 && i+j < NUM_LEDS){
         leds.setPixelColor(i+j, leds.Color(255,255,255));
      }
    }
    leds.show();
    vTaskDelay(10);
  } 

  leds.clear();
  leds.show();

  for (;;) {
    vTaskDelay(30000);
  }
}


void preset12_anim_player(void *parameter) {
  char* param;
  param = (char *) parameter;
  int timedelay = atoi(param);
  timedelay = timedelay;

  leds.clear();
  for (int i = NUM_LEDS+10; i > -10 ; i--){
    leds.clear();
    for(int j = 0; j < 10; j++){
      if (i-j >= 0 && i-j < NUM_LEDS){
         leds.setPixelColor(i-j, leds.Color(255,255,255));
      }
    }
    leds.show();
    vTaskDelay(timedelay);
  } 

  leds.clear();
  leds.show();

  for (;;) {
    vTaskDelay(30000);
  }
}

/////////////////////OSC HANDLERS///////////////////

void handler(OSCMessage &msg, int offset){
  Serial.println("Match: string");
  //string multiple 'route' methods together using the pattern offset parameter. 
  msg.route("/preset", preset_handler, offset);
}

void preset_handler(OSCMessage &msg, int offset){
  Serial.println("Match: preset");
  //string multiple 'route' methods together using the pattern offset parameter. 
  msg.route("/0", preset0_handler, offset);
  msg.route("/1", preset1_handler, offset);
  msg.route("/2", preset2_handler, offset);
  msg.route("/3", preset3_handler, offset);
  msg.route("/10", preset10_handler, offset);
  msg.route("/11", preset11_handler, offset);
  msg.route("/12", preset12_handler, offset);
}

////////////////////////////////////PRESET HANDLERS//////////////////////

void preset0_handler(OSCMessage &msg, int offset){
  Serial.println("PRES0");
  
  kill_old_tasks();
  Serial.print("1 in queue attemting to createa  atask");
  xTaskCreatePinnedToCore(preset0_anim_player, "anim1", 2048, NULL, 10,  &(anim_task_handles[0]), 0);

}
void preset1_handler(OSCMessage &msg, int offset){
  kill_old_tasks();
  Serial.print("1 in queue attemting to createa  atask");
  xTaskCreatePinnedToCore(preset1_anim_player, "anim1", 2048, NULL, 10,  &(anim_task_handles[1]), 0);

}
void preset2_handler(OSCMessage &msg, int offset){
  kill_old_tasks();

  const char* param = msg.getAddress() +offset +1 ;
  xTaskCreatePinnedToCore(preset2_anim_player, "anim2", 2048, (void *)param, 10,  &(anim_task_handles[2]), 0);
}
void preset3_handler(OSCMessage &msg, int offset){
  kill_old_tasks();
  
  const char* param = msg.getAddress() +offset +1 ;
  xTaskCreatePinnedToCore(preset3_anim_player, "anim2", 2048, (void *)param, 10,  &(anim_task_handles[2]), 0);
}
void preset10_handler(OSCMessage &msg, int offset){
  Serial.println("PRES10");
  kill_old_tasks();

  const char* param = msg.getAddress() +offset +1 ;
  xTaskCreatePinnedToCore(preset10_anim_player, "anim10", 2048, (void *)param, 10,  &(anim_task_handles[10]), 0);
}
void preset11_handler(OSCMessage &msg, int offset){
  kill_old_tasks();

  const char* param = msg.getAddress() +offset +1 ;
  xTaskCreatePinnedToCore(preset11_anim_player, "anim11", 2048, (void *)param, 10,  &(anim_task_handles[10]), 0);
}
void preset12_handler(OSCMessage &msg, int offset){
  kill_old_tasks();

  const char* param = msg.getAddress() +offset +1 ;
  xTaskCreatePinnedToCore(preset12_anim_player, "anim12", 2048, (void *)param, 10,  &(anim_task_handles[10]), 0);
}





/////////////////SETUP AND MAIN APP////////////////////////

void setup() {
  Serial.begin(115200);
  delay(1000);

  // WIFI
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, ledState);    // turn *on* led
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
  Serial.println(localPort);

  Serial.print("testing leds");
  leds.begin();
  for (int i = 0; i < NUM_LEDS; i++) leds.setPixelColor(i, leds.Color(255,0,0));
  leds.show();
  delay(1000);
  for (int i = 0; i < NUM_LEDS; i++) leds.setPixelColor(i, leds.Color(0,255,0));
  leds.show();
  delay(1000);
  for (int i = 0; i < NUM_LEDS; i++) leds.setPixelColor(i, leds.Color(0,0,255));
  leds.show();
  delay(1000);
  leds.clear();
  
  Serial.print("testing doen");
  leds.show();
}



void loop(){
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
  //yileds here
}