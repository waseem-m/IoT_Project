#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// touch includes
#include <Wire.h>
#include <Adafruit_MPR121.h>
Adafruit_MPR121 cap1 = Adafruit_MPR121();
Adafruit_MPR121 cap2 = Adafruit_MPR121();

uint16_t lasttouched1 = 0;
uint16_t currtouched1 = 0;

uint16_t lasttouched2 = 0;
uint16_t currtouched2 = 0;

//#include <SoftwareSerial.h>
//SoftwareSerial mySerial(10, 11);

#define LED_PIN    6
// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 60
// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
int pin=6;
const uint32_t BAUD_RATE = 9600;

//MP3 includes, constants, and objects
#include <SoftwareSerial.h>
#include "RedMP3.h"

#define MP3_RX 7//RX of Serial MP3 module connect to D7 of Arduino
#define MP3_TX 8//TX to D8, note that D8 can not be used as RX on Mega2560, you should modify this if you do not use Arduino UNO
MP3 mp3(MP3_RX, MP3_TX);

int8_t volume = 0x1a;//0~0x1e (30 adjustable level)
int8_t folderName = 0x03;//folder name must be 01 02 03 04 ...
int8_t fileName = 0x01; // prefix of file name must be 001xxx 002xxx 003xxx 004xxx ...

bool isWaitingESP = false;
bool isWaitingPC = false;


void setup() {
  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  Serial.begin(BAUD_RATE);
  pinMode(pin,OUTPUT);
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
  
  // Setup touch sensors
  if (!cap1.begin(0x5A)) 
  {
    //Serial.println("MPR121 A not found, check wiring?");
    while (1);
  }
  //cap1.setThreshholds(50,25);
  cap1.setThreshholds(40,20);
  //cap1.setThreshholds(30,15);
  //cap1.setThreshholds(28,14);
  //cap1.setThreshholds(26,13);
  //cap1.setThreshholds(20,10);
  //cap1.setThreshholds(12,6);
  //cap1.setThreshholds(10,5);
  //cap1.setThreshholds(8,4);
  //Serial.println("MPR121 A found!");
  
  if (!cap2.begin(0x5B)) {
    //Serial.println("MPR121 B not found, check wiring?");
    while (1);
  }
 // cap2.setThreshholds(50,25);
  cap2.setThreshholds(40,20);
  //cap2.setThreshholds(30,15);
  //cap2.setThreshholds(28,14);
  //cap2.setThreshholds(26,13);
  //cap2.setThreshholds(20,10);
  //cap2.setThreshholds(12,6);
  
  //cap2.setThreshholds(10,5);
  //cap2.setThreshholds(8,4);
  //Serial.println("MPR121 B found!");

  // Setup MP3 module
  delay(500);//Requires 500ms to wait for the MP3 module to initialize  
  mp3.setVolume(volume);
  delay(50);//you should wait for >=50ms between two commands
  
}

void loop() {
  currtouched1 = cap1.touched();
  currtouched2 = cap2.touched();

  for (int i = 0; i < 12; i++) {
    int index = -1;

    // first sensor: i is an even number between 0 and 10
    if ((currtouched1 & _BV(i)) && !(lasttouched1 & _BV(i)) ) 
    {
      if(i%2 == 0){
        // index between 0 and 5
        index = i / 2;
      }
    }
    else if (!(currtouched1 & _BV(i)) && (lasttouched1 & _BV(i)) ) {
      //Serial.print(i);
      //Serial.println(" released of A");
      if(isWaitingESP == false){
        colorWipe(-1,0);
      } 
    }

    // second sensor: i is an even number between 0 and 10, or is 11.
    if ((currtouched2 & _BV(i)) && !(lasttouched2 & _BV(i)) ) 
    {
      if(i%2 == 0 && i != 4){
      //if(i%2 == 0){
        // index between 6 and 11
        index = 6 + (i / 2);
      } else if (i == 5){
        // using 3 instead of 4, because 4 is not working properly
        index = 8;
      } else if (i == 11){
        // if last
        index = 12;
      }
    }
    else if (!(currtouched2 & _BV(i)) && (lasttouched2 & _BV(i)) ) {
      //Serial.print(i);
      //Serial.println(" released of A");
      if(isWaitingESP == false){
        colorWipe(-1,0);
      } 
    }

    
    if(index != -1){
      String str = String((char)('a' + index));
      if(isWaitingESP == false){
        colorWipe(index,0);
        Serial.print(str);
      }
      if(index <= 8){
       mp3.playWithFileName(folderName,index+1);
      } else {
        mp3.playWithFileName(folderName+1,index-8);
      }
      delay(50);//you should wait for >=50ms between two MP3 commands
    }
  }

  lasttouched1 = currtouched1;
  lasttouched2 = currtouched2;

  if(Serial.available()>0){
    String info = Serial.readStringUntil('\n');
    if (info == "all wrong"){
      colorWipe(19,0);
    } else if(info == "not matched"){
      colorWipe(20,0);
    }
    else if(info=="matched"){
      colorWipe(21,0);
    }
    else if(info=="waiting esp"){
      isWaitingESP = true;
      isWaitingPC = true;
      colorWipe(22,0);
    } else if(info=="waiting pc"){
      isWaitingPC = true;
      isWaitingESP = true;
      colorWipe(23,0);
    } else if(info.length() == 1) {
      // if length is == 1 then got a note from PC, just turn on LED
      char c = info.charAt(0);
      int num = c - 'A';
      colorWipe(31,0);
      colorWipe(num,350);
      colorWipe(-1,150);
      isWaitingPC = false;
    } else { // got a song from ESP
        colorWipe(30,0);
        playSong(info);
        isWaitingESP = false;
    }
  }
}

//assumes notes are 'a' to 'm' for esp
void playSong(String song){
  int len = song.length()/2;
  for(int i=0; i<len; i++){
    char c = song.charAt(2*i);
    int num = c - 'a';
    if(num <= 8){
      mp3.playWithFileName(folderName,num+1);
    } else {
      mp3.playWithFileName(folderName+1,num-8);
    }
    //mp3.playWithFileName(folderName,num+1);
    colorWipe(num,350); //turn on led and wait 400
    colorWipe(-1,150); //clear and wait 100
  }
  colorWipe(-1,0); //clear all
}

void colorWipe(int i, int wait) {
  uint32_t color=strip.Color(0,   0,   0); //(G,R,B)
  //int offset=0;
  if(i==0){
    color = strip.Color(255,   0,   0); //Green
    //offset=0;
  }
  if(i==1){
    color = strip.Color(0,  255 , 0 ); //Red
    //offset=2;
  }
  if(i==2){
    color = strip.Color(0,  0 , 255 ); //Blue
    //offset=4;
  }
  if(i==3){
    color = strip.Color(100,  100 , 100 ); 
    //offset=6;
  }
  if(i==4){
    color = strip.Color(50,  250 , 0 ); //Orange
    //offset=8;
  }
  if(i==5){
    color = strip.Color(0,  120 , 50 ); //Pink
    //offset = 10;
  }
  if(i==6){
   color = strip.Color(70,   200,   80);
   //offset=12;
  }
  if(i==7){
   color = strip.Color(200,   150,   200); //White-ish
   //offset=14;
  }
  if(i==8){
   color = strip.Color(255,   50,   100); 
   //offset=14;
  }
  if(i==9){
   color = strip.Color(150,   50,   200); 
   //offset=14;
  }
  if(i==10){
   color = strip.Color(50,   100,   200); 
   //offset=14;
  }
  if(i==11){
   color = strip.Color(0,   200,   100); 
   //offset=14;
  }
  if(i==12){
   color = strip.Color(100,   0,   150); 
   //offset=14;
  }
  //---------------------------------------------
  if(i==19){ // all wrong
    color = strip.Color(255,   0,   0);  //Red
    
  }
  if(i==20){ // not matched
    color = strip.Color(250,  50 , 0 ); //Orange
    
  }
  if(i==21){ // matched
    color = strip.Color(0,   255,   0); //Green
  }
  //---------------------------------------------
  if(i==22){ // Waiting esp
    colorWipe(30,0);
    color = strip.Color(128,   128,   128); //Gray
  }
  if(i==23){ // Waiting pc
    colorWipe(31,0);
    color = strip.Color(128,   128,   128); //Gray
  }
  //---------------------------------------------
  if(i==30){ // esp system 
     color = strip.Color(150,   50,   200);
  }
  if(i==31){ // pc system
    color = strip.Color(0,   200,   100);
  }
  //---------------------------------------------
  if(i==-1) { //clear notes
    color = strip.Color(0,0,0);
    for(int k=0; k<13; k++){
      strip.setPixelColor(k,color);
    }
    strip.show();
    delay(wait);
    return;
  }
  //---------------------------------------------
  if(i>=0 && i<=12){
    colorWipe(-1,0); //clear notes
  }
  //---------------------------------------------

  
  if(i >= 19 && i <= 23) //Status change
  { 
    strip.setPixelColor(13, color);
    strip.show();                          //  Update strip to match
    if(i != 22 && i != 23){
      delay(3000);                           //  Pause for a moment
      colorWipe(-1,0);
      uint32_t colorOff = strip.Color(0,0,0);
      strip.setPixelColor(13,colorOff); //turn off status
      strip.show();
    }
  }
  else if(i==30){ //esp system
    strip.setPixelColor(14, color);
    uint32_t colorOff = strip.Color(0,0,0);
    strip.setPixelColor(15,colorOff); //turn off pc system
    strip.show();                  
  }
  else if(i==31){ //pc system
    strip.setPixelColor(15, color);
    uint32_t colorOff = strip.Color(0,0,0);
    strip.setPixelColor(14,colorOff); //turn off esp system
    strip.show();                  
  }
  else
  {
    //for(int j=offset; j<offset+2; j++) { // For each pixel in strip...
      //strip.setPixelColor(j, color);         //  Set pixel's color (in RAM)
      strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
      strip.show();                          //  Update strip to match
      delay(wait);                           //  Pause for a moment
    //}
  }
}
