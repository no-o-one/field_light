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


#include <password_secrets.h>

//core 0 - wifi ans sys stuff
//core 1 - appcore

const char* ssid      = secret_ssid;
const char* pass  = secret_password;
const IPAddress bcastIp(192,168,0,255);   


const uint8_t SELF_ID = 0;       ///////////// STRIP SPECS   
const uint8_t LAST_ID = 3;    
uint8_t ANIM_DELAY = 10; //ms, dleay between frames of the animation
const int LED_PIN        = 14;
const int NUM_LEDS       = 900;    
const bool IS_KNOCKOFF = true; 

WiFiUDP Udp;

const unsigned int localPort = 6000;        // local port to listen for UDP packets (here's where we send the packets)

OSCErrorCode error;
unsigned int ledState = LOW;              // LOW means led is *on*

TaskHandle_t anim_task_handles[20] = {NULL};

Adafruit_NeoPixel leds(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

#define LED_BUILTIN 13



bool tracking_delay(TickType_t ticks) {
  uint32_t notified = ulTaskNotifyTake(pdTRUE, ticks); //will stall while tracking notifs
  return (notified > 0); //return if was notified to stop (0 - no notification)
}

void kill_old_tasks(){
  for(int i = 0; i < 21; i ++){
    if (anim_task_handles[i] != NULL){

      TaskHandle_t handle = anim_task_handles[i];
      anim_task_handles[i] = NULL;

      xTaskNotifyGive(handle); // notify to stop 

      int tries = 0;
      while (eTaskGetState(handle) != eDeleted && tries < 200) { // wait for task to destruct
        vTaskDelay(pdMS_TO_TICKS(5));
        tries++;
      }  

    }
  }
}

const char* get_safe_param(OSCMessage &msg, int offset){
  const char* addr = msg.getAddress();
  int addrlen = strlen(addr);
  if (offset < addrlen && addr[offset] == '/') {
    return addr + offset + 1;   
  }
  return addr + addrlen;        
}


void my_loop(void *parameter){
  Serial.println("LOOP REACHED!");
  for (;;){
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
        //Serial.print("error: ");
        //Serial.println(error);
     }
    }
  //yileds here
    vTaskDelay(20);
  }
}

//////////////////////////////ANIMATIONS ///////////////////////////

void preset0_anim_player(void *parameter) {

  String myresp = String("/strip/");
  myresp = myresp + (SELF_ID+1);
  myresp = myresp + "/preset/0";
  Serial.println(myresp.c_str());

  OSCMessage msg(myresp.c_str());
  Udp.beginPacket(bcastIp, localPort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();

  leds.clear();
  leds.show();


  for (;;) {
    if (tracking_delay(30000)) { //wait
        vTaskDelete(NULL); 
        return;
      }
  }
}


void preset1_anim_player(void *parameter) {
  char* param;
  param = (char *) parameter;
  char* color_str = NULL;
  char* parambackup = NULL;

  if(strlen(param) > 0 ){
  parambackup = strdup(param);
  color_str = strtok(param, "/");
  }

  uint8_t color[3] = {255, 255, 255};

  if(color_str != NULL){
     color[0] = (uint8_t)atoi(strtok(color_str, ","));
     color[1] = (uint8_t)atoi(strtok(NULL, ","));
     color[2] = (uint8_t)atoi(strtok(NULL, ","));
     if (IS_KNOCKOFF){
      uint8_t temp = color[0];
      color[0] = color[1];
      color[1] = temp;
     }
  }
 

  String myresp = String("/strip/");
  myresp = myresp + (SELF_ID+1);
  myresp = myresp + "/preset/1/";
  if (parambackup != NULL) myresp = myresp + parambackup;
  Serial.println(myresp.c_str());

  OSCMessage msg(myresp.c_str());
  Udp.beginPacket(bcastIp, localPort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();

  leds.clear();

  for (int i = 0; i < NUM_LEDS; i++) leds.setPixelColor(i, leds.Color(color[0],color[1],color[2]));
  leds.show();

  for (;;) {
    if (tracking_delay(30000)) { //wait
        vTaskDelete(NULL); 
        return;
    }
  }
}


void preset2_anim_player(void *parameter) {
  char* param;
  param = (char *) parameter;
  char* parambackup = strdup(param);
  
  int numledstolight = atoi(strtok(param, "/"));
  char* color_str = strtok(NULL, "/");
  uint8_t color[3] = {255, 255, 255};


  if(color_str != NULL){
     color[0] = (uint8_t)atoi(strtok(color_str, ","));
     color[1] = (uint8_t)atoi(strtok(NULL, ","));
     color[2] = (uint8_t)atoi(strtok(NULL, ","));
     if (IS_KNOCKOFF){
      uint8_t temp = color[0];
      color[0] = color[1];
      color[1] = temp;
     }
  }



  String myresp = String("/strip/");
  myresp = myresp + (SELF_ID+1);
  myresp = myresp + "/preset/2/";
  myresp = myresp + parambackup;
  Serial.println(myresp.c_str());

  OSCMessage msg(myresp.c_str());
  Udp.beginPacket(bcastIp, localPort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();


  leds.clear();

  if(SELF_ID == LAST_ID){
    for (int i = 0; i < numledstolight; i++) leds.setPixelColor(NUM_LEDS-i-1, leds.Color(color[0], color[1], color[2]));
    leds.show();

    for(;;){
      if (tracking_delay(30000)) { //wait
        vTaskDelete(NULL); 
        return;
      }
    }
  

  }else{

    for(;;){
      if (tracking_delay(30000)) { //wait
        leds.clear();
        leds.show();
        vTaskDelete(NULL); 
        return;
      }
    }
  }
}


void preset3_anim_player(void *parameter) {
  char* param;
  param = (char *) parameter;
  char* parambackup = strdup(param);

  
  int numledstolight = atoi(strtok(param, "/"));

 
  String myresp = String("/strip/");
  myresp = myresp + (SELF_ID+1);
  myresp = myresp + "/preset/3/";
  myresp = myresp + parambackup;
  Serial.println(myresp.c_str());

  OSCMessage msg(myresp.c_str());
  Udp.beginPacket(bcastIp, localPort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();


  leds.clear();


  if (SELF_ID == LAST_ID){
    int j = 255;
    bool isincr = true;
    for(;;){  
      for (int i = 0; i < numledstolight; i++) leds.setPixelColor(NUM_LEDS-1-i, leds.Color(j, j, j));
      leds.show();

      if(j == 20){
        isincr = true;
      }
      if( j == 255){
        isincr = false;
      }
      if(isincr){
        j = j + 1;
      }else{
        j = j - 1;
      }

      if (tracking_delay(ANIM_DELAY)) { //wait
        vTaskDelete(NULL); 
        return;
      }
    }
  }else{
    for(;;){
      if (tracking_delay(30000)) { //wait
        vTaskDelete(NULL); 
        return;
      }
    }
  }
}


void preset4_anim_player(void *parameter) {
  char* param;
  param = (char *) parameter;
  char* parambackup = strdup(param);

  int groupof = atoi(strtok(param, "/"));
  char* color_str = strtok(NULL, "/");
  uint8_t color[3] = {255, 255, 255};

  if(color_str != NULL){
     color[0] = (uint8_t)atoi(strtok(color_str, ","));
     color[1] = (uint8_t)atoi(strtok(NULL, ","));
     color[2] = (uint8_t)atoi(strtok(NULL, ","));
     if (IS_KNOCKOFF){
      uint8_t temp = color[0];
      color[0] = color[1];
      color[1] = temp;
     }
  }
 

  String myresp = String("/strip/");
  myresp = myresp + (SELF_ID+1);
  myresp = myresp + "/preset/4/";
  myresp = myresp + parambackup;
  Serial.println(myresp.c_str());

  OSCMessage msg(myresp.c_str());
  Udp.beginPacket(bcastIp, localPort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();

  leds.clear();

  int chunksize = groupof*12;
  int chunkamount = NUM_LEDS / chunksize; 
  for (;;) {

    for (int i = 0; i < (chunksize + NUM_LEDS); i += chunksize){
      int offset = random(0, (chunksize - groupof));
      for (int j = offset; j < (offset + groupof); j ++){
        
        if (i+j >= 0 && i+j < NUM_LEDS) leds.setPixelColor(i+j, leds.Color(color[0],color[1],color[2]));
      }

    }
    leds.show();
    leds.clear();
    if (tracking_delay(ANIM_DELAY + 5)) { //wait, kill self if told to
      vTaskDelete(NULL); 
      return;
    }

  }
}



void preset10_anim_player(void *parameter) {
  char* param;
  param = (char *) parameter;
  char* parambackup = strdup(param);

  int groupof = atoi(strtok(param, "/"));
  char* color_str = strtok(NULL, "/");
  uint8_t color[3] = {255, 255, 255};

  if(color_str != NULL){
     color[0] = (uint8_t)atoi(strtok(color_str, ","));
     color[1] = (uint8_t)atoi(strtok(NULL, ","));
     color[2] = (uint8_t)atoi(strtok(NULL, ","));
     if (IS_KNOCKOFF){
      uint8_t temp = color[0];
      color[0] = color[1];
      color[1] = temp;
     }
  }


  leds.clear();
  leds.show();

  //vTaskDelay((ANIM_DELAY*2)*(NUM_LEDS/groupof)*SELF_ID);

  int i = 0;
  while (i < NUM_LEDS+groupof){
    for(int j = 0; j < groupof; j++){

      if (i+j >= 0 && i+j < NUM_LEDS) leds.setPixelColor(i+j, leds.Color(color[0], color[1], color[2]));
      
    }
    i = i + groupof;
    leds.show();

    if (tracking_delay(ANIM_DELAY)) { //wait
        vTaskDelete(NULL); 
        return;
    }
  } 

  String myresp = String("/strip/");
  myresp = myresp + (SELF_ID+1);
  myresp = myresp + "/preset/10/";
  myresp = myresp + parambackup;
  Serial.println(myresp.c_str());

  OSCMessage msg(myresp.c_str());
  Udp.beginPacket(bcastIp, localPort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();


  for (;;) {
    if (tracking_delay(30000)) { //wait
        vTaskDelete(NULL); 
        return;
    }
  }
}


void preset11_anim_player(void *parameter) {
  char* param;
  param = (char *) parameter;
  char* parambackup = strdup(param);

  int groupof = atoi(strtok(param, "/"));
  char* color_str = strtok(NULL, "/");
  uint8_t color[3] = {255, 255, 255};

  if(color_str != NULL){
     color[0] = (uint8_t)atoi(strtok(color_str, ","));
     color[1] = (uint8_t)atoi(strtok(NULL, ","));
     color[2] = (uint8_t)atoi(strtok(NULL, ","));
     if (IS_KNOCKOFF){
      uint8_t temp = color[0];
      color[0] = color[1];
      color[1] = temp;
     }
  }

  // Serial.println(groupof);
  // Serial.println(color[0]);
  // Serial.println(color[1]);
  // Serial.println(color[2]);


  leds.clear();
  leds.show();

 // vTaskDelay(((ANIM_DELAY*2)*(NUM_LEDS/groupof)*SELF_ID));

  int i = 0;
  while (i < NUM_LEDS+groupof){
    leds.clear();//just the list

    for(int j = 0; j < groupof; j++){

      if (i+j >= 0 && i+j < NUM_LEDS) leds.setPixelColor(i+j, leds.Color(color[0], color[1], color[2]));
    }
    i = i + groupof;
    leds.show();

    if (tracking_delay(ANIM_DELAY)) { //wait

      vTaskDelete(NULL); 
      return;
    }
  } 

  String myresp = String("/strip/");
  myresp = myresp + (SELF_ID+1);
  myresp = myresp + "/preset/11/";
  myresp = myresp + parambackup;
  Serial.println(myresp.c_str());

  OSCMessage msg(myresp.c_str());
  Udp.beginPacket(bcastIp, localPort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();

  leds.clear();
  leds.show();

  for (;;) {
    if (tracking_delay(30000)) { //wait
        vTaskDelete(NULL); 
        return;
      }
  }
}


void preset12_anim_player(void *parameter) {
  char* param;
  param = (char *) parameter;
  char* parambackup = strdup(param);

  int groupof = atoi(strtok(param, "/"));
  char* color_str = strtok(NULL, "/");
  uint8_t color[3] = {255, 255, 255};

  if(color_str != NULL){
     color[0] = (uint8_t)atoi(strtok(color_str, ","));
     color[1] = (uint8_t)atoi(strtok(NULL, ","));
     color[2] = (uint8_t)atoi(strtok(NULL, ","));
     if (IS_KNOCKOFF){
      uint8_t temp = color[0];
      color[0] = color[1];
      color[1] = temp;
     }
  }


  leds.clear();
  leds.show();

 // vTaskDelay((timedelay*2)*NUM_LEDS*((-1*SELF_ID)+LAST_ID));

  leds.clear();
  int i = NUM_LEDS-1;
  while (i >= (-1*groupof)){
    leds.clear();
    for(int j = 0; j < groupof; j++){

      if (i-j >= 0 && i-j < NUM_LEDS) leds.setPixelColor(i-j, leds.Color(color[0], color[1], color[2]));
      
    }
    leds.show();
    if (tracking_delay(ANIM_DELAY)) { //wait, kill self if signal recieved 
        vTaskDelete(NULL); 
        return;
    }

    i = i - groupof;
  } 

  String myresp = String("/strip/");
  myresp = myresp + (SELF_ID-1);
  myresp = myresp + "/preset/12/";
  myresp = myresp + parambackup;
  Serial.println(myresp.c_str());

  OSCMessage msg(myresp.c_str());
  Udp.beginPacket(bcastIp, localPort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();

  leds.clear();
  leds.show();

  for (;;) {
    if (tracking_delay(30000)) { //wait
        vTaskDelete(NULL); 
        return;
      }
  }
}


void preset20_anim_player(void *parameter) {

  String myresp = String("/strip/");
  myresp = myresp + (SELF_ID+1);
  myresp = myresp + "/preset/4";
  Serial.println(myresp.c_str());

  OSCMessage msg(myresp.c_str());
  Udp.beginPacket(bcastIp, localPort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();

  for (;;) {
    leds.clear();

    for (int j = 0; j <256; j+=5){
      leds.clear();
      for (int o = 0; o < NUM_LEDS; o++) leds.setPixelColor(o, leds.Color(255,j,0));
      leds.show();

      if (tracking_delay(ANIM_DELAY)) { //wait
        vTaskDelete(NULL); 
        return;
      }  

    }
  
    for (int i = 255; i > -1; i-= 5){
      leds.clear();
      for (int o = 0; o < NUM_LEDS; o++) leds.setPixelColor(o, leds.Color(i,255,0));
      leds.show();
      
      if (tracking_delay(ANIM_DELAY)) { //wait
        vTaskDelete(NULL); 
        return;
      }

    }

    for (int j = 0; j <256; j+=5){
      leds.clear();
      for (int o = 0; o < NUM_LEDS; o++) leds.setPixelColor(o, leds.Color(0,255,j));
      leds.show();

      if (tracking_delay(ANIM_DELAY)) { //wait
        vTaskDelete(NULL); 
        return;
      }

    }

    for (int i = 255; i > -1; i-= 5){
      leds.clear();
      for (int o = 0; o < NUM_LEDS; o++) leds.setPixelColor(o, leds.Color(0,i,255));
      leds.show();

      if (tracking_delay(ANIM_DELAY)) { //wait
        vTaskDelete(NULL); 
        return;
      }

    }
   
    for (int j = 0; j <256; j+= 5){
      leds.clear();
      for (int o = 0; o < NUM_LEDS; o++) leds.setPixelColor(o, leds.Color(j, 0, 255));
      leds.show();

      if (tracking_delay(ANIM_DELAY)) { //wait
        vTaskDelete(NULL); 
        return;
      }

    }
  
    for (int i = 255; i > -1; i-= 5){
      leds.clear();
      for (int o = 0; o < NUM_LEDS; o++) leds.setPixelColor(o, leds.Color(255,0,i));
      leds.show();

      if (tracking_delay(ANIM_DELAY)) { //wait
        vTaskDelete(NULL); 
        return;
      }

    }
    if (tracking_delay(ANIM_DELAY)) { //wait
        vTaskDelete(NULL); 
        return;
      }
  } 

}

/////////////////////OSC HANDLERS///////////////////

void handler(OSCMessage &msg, int offset){
  String myreq = String("/");
  myreq = myreq + SELF_ID;
  myreq = myreq + "/preset";
  Serial.println(myreq.c_str());

  msg.route(myreq.c_str(), preset_handler, offset);
}

void preset_handler(OSCMessage &msg, int offset){
  msg.route("/0", preset0_handler, offset);
  msg.route("/1", preset1_handler, offset);
  msg.route("/2", preset2_handler, offset);
  msg.route("/3", preset3_handler, offset);
  msg.route("/4", preset4_handler, offset);
  msg.route("/10", preset10_handler, offset);
  msg.route("/11", preset11_handler, offset);
  msg.route("/12", preset12_handler, offset);
  msg.route("/20", preset20_handler, offset);
}

////////////////////////////////////PRESET HANDLERS//////////////////////

void preset0_handler(OSCMessage &msg, int offset){
 // Serial.println("PRES0");
  
  kill_old_tasks();
  xTaskCreatePinnedToCore(preset0_anim_player, "anim1", 2048, NULL, 10,  &(anim_task_handles[0]), 1);

}
void preset1_handler(OSCMessage &msg, int offset){
  
  kill_old_tasks();
  //Serial.println("HNDLER 1");
  const char* param = get_safe_param(msg, offset);
  xTaskCreatePinnedToCore(preset1_anim_player, "anim1", 2048, (void *)param, 10,  &(anim_task_handles[1]), 1);

}
void preset2_handler(OSCMessage &msg, int offset){
  kill_old_tasks();

  const char* param = msg.getAddress() +offset +1 ;
  xTaskCreatePinnedToCore(preset2_anim_player, "anim2", 2048, (void *)param, 10,  &(anim_task_handles[2]), 1);
}
void preset3_handler(OSCMessage &msg, int offset){
  kill_old_tasks();
  
  const char* param = msg.getAddress() +offset +1 ;
  xTaskCreatePinnedToCore(preset3_anim_player, "anim3", 2048, (void *)param, 10,  &(anim_task_handles[3]), 1);
}
void preset4_handler(OSCMessage &msg, int offset){
  kill_old_tasks();
  
  const char* param = msg.getAddress() +offset +1 ;
  xTaskCreatePinnedToCore(preset4_anim_player, "anim4", 2048, (void *)param, 10,  &(anim_task_handles[4]), 1);
}
void preset10_handler(OSCMessage &msg, int offset){
  //Serial.println("PRES10");
  kill_old_tasks();

  const char* param = msg.getAddress() +offset +1 ;
  xTaskCreatePinnedToCore(preset10_anim_player, "anim10", 2048, (void *)param, 10,  &(anim_task_handles[10]), 1);
}
void preset11_handler(OSCMessage &msg, int offset){
  kill_old_tasks();

  const char* param = msg.getAddress() +offset +1 ;
  xTaskCreatePinnedToCore(preset11_anim_player, "anim11", 2048, (void *)param, 10,  &(anim_task_handles[11]), 1);
}
void preset12_handler(OSCMessage &msg, int offset){
  kill_old_tasks();

  const char* param = msg.getAddress() +offset +1 ;
  xTaskCreatePinnedToCore(preset12_anim_player, "anim12", 2048, (void *)param, 10,  &(anim_task_handles[12]), 1);
}
void preset20_handler(OSCMessage &msg, int offset){
  //Serial.println("PRES0");
  
  kill_old_tasks();
  //Serial.print("rgb!");
  xTaskCreatePinnedToCore(preset20_anim_player, "anim rgb 20", 2048, NULL, 10,  &(anim_task_handles[20]), 1);
}

/////////////////LED TESTING SCRIPT////////////////////////

void initTest() {
  uint32_t colors[] = {
    leds.Color(255, 0, 0),
    leds.Color(0, 255, 0),
    leds.Color(0, 0, 255),
    leds.Color(0, 0, 0),
    //leds.Color(255, 255, 255)
    
  };
  for (uint32_t c : colors) {
    for (int i = 0; i < NUM_LEDS; i++) leds.setPixelColor(i, c);
    leds.show();
    delay(400);
  }
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
  initTest();
  Serial.print("testing done");
  xTaskCreatePinnedToCore(my_loop, "loop", 2048, NULL, 1,  NULL, 1);
}


//no loop code apparantly
void loop(){

}