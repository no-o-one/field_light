#include <stdio.h>
#include <string.h> // For memset
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

//core 0 - wifi ans sys stuff
//core 1 - appcore

char ssid[] = "later";        
char pass[] = "later";   
uint8_t SELF_ID = 0;       //////////////ID         

WiFiUDP Udp;

const unsigned int localPort = 6000;        // local port to listen for UDP packets (here's where we send the packets)

OSCErrorCode error;
unsigned int ledState = LOW;              // LOW means led is *on*

#define LED_BUILTIN

void kill_old_tasks(){
  for(int i = 0; i < 12; i ++){
    if (anim_task_handles[i] != NULL){
      vTaskDelete(anim_task_handles[i]);
      anim_task_handles[i] = NULL;
    }
  }
}

//////////////////////////////ANIMATIONS ///////////////////////////

void preset1_anim_player(void *parameter) {
  for (;;) {
    Serial.println("Task1: LED1 ON");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    Serial.println("Task1: LED1 OFF");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    Serial.print("Task 1 running on core ");
    Serial.println(xPortGetCoreID());
  }
}

void preset2_anim_player(void *parameter) {
  for (;;) {
    Serial.println("Task2: LED2 ON");
    vTaskDelay(333 / portTICK_PERIOD_MS);
    Serial.println("Task2: LED2 OFF");
    vTaskDelay(333 / portTICK_PERIOD_MS);
    Serial.print("Task 2 running on core ");
    Serial.println(xPortGetCoreID());
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
  msg.route("/0", preset1_handler, offset);
  msg.route("/1", preset1_handler, offset);
  msg.route("/2", preset2_handler, offset);
  msg.route("/3", preset3_handler, offset);
  msg.route("/4", preset4_handler, offset);
}

void preset0_handler(OSCMessage &msg, int offset){
  Serial.println("PRES0");
  uint8_t num = 0;
//  xQueueSend(preset_queue, &num, 0);
}
void preset1_handler(OSCMessage &msg, int offset){
  Serial.println("PRES1");
  
  kill_old_tasks();
  Serial.print("1 in queue attemting to createa  atask");
  xTaskCreatePinnedToCore(preset1_anim_player, "anim1", 2048, NULL, 10,  &(anim_task_handles[1]), 0);

}
void preset2_handler(OSCMessage &msg, int offset){
  Serial.println("PRES2");
  
  kill_old_tasks();
  Serial.print("2 in queue attemting to createa  atask");
  xTaskCreatePinnedToCore(preset2_anim_player, "anim2", 2048, NULL, 10,  &(anim_task_handles[2]), 0);
}
void preset3_handler(OSCMessage &msg, int offset){
  Serial.println("PRES3");
}
void preset4_handler(OSCMessage &msg, int offset){
  Serial.println("PRES4");
}


/////////////////SETUP AND MAIN APP////////////////////////

void setup() {
  Serial.begin(115200);
  delay(1000);

  // WIFI
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, ledState);    // turn *on* led
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