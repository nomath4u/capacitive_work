#include <Adafruit_NeoPixel.h>

/*********************************************************
This is a library for the MPR121 12-channel Capacitive touch sensor

Designed specifically to work with the MPR121 Breakout in the Adafruit shop 
  ----> https://www.adafruit.com/products/

These sensors use I2C communicate, at least 2 pins are required 
to interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada for Adafruit Industries.  
BSD license, all text above must be included in any redistribution
**********************************************************/

#include <Wire.h>
#include "Adafruit_MPR121.h"


#define NUM_LINES 4
#define TOUCH_THRESHHOLD 2
#define RELEASE_THRESHHOLD 2
#define OFFSET 2 //The value that the baseline is trying to cheat by
#define TAPE_WIDTH 9 //millimeters
#define TAPE_SPACE 0 //millimeters between edges of tapes
#define TAPE_WEIGHT 240 //How much to move over per tape
#define LINE_MAX (TAPE_WEIGHT * NUM_LINES)
#define MAX_CAP TAPE_WEIGHT
#define PIN 6


// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;
uint16_t last[NUM_LINES];
uint16_t curr[NUM_LINES];
uint16_t baseline_l[NUM_LINES];
Adafruit_NeoPixel strip = Adafruit_NeoPixel(8, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  while (!Serial);        // needed to keep leonardo/micro from starting too fast!

  Serial.begin(9600);
  Serial.println("Adafruit MPR121 Capacitive Touch sensor test"); 
  
  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    delay(50);
    while (1);
  }
  delay(500); //Wait for chip to be initialized with baselines
  /*USing this as our baseline allows us to be more sensitive, however beware of drift and thresholds*/
  Serial.println("MPR121 found!");
 
  //load_raw(curr); //initialize to some data
  //last = curr; //Make them the same just so we don't accidentally touch anything
  initialize_state();
  strip.begin();
  strip.show(); //Initialize them to off
}

void loop() {
  // Get the currently touched pads
  //currtouched = cap.touched();
  
  //for (uint8_t i=0; i<12; i++) {
    // it if *is* touched and *wasnt* touched before, alert!
  //  if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
  //    Serial.print(i); Serial.println(" touched");
  //  }
    // if it *was* touched and now *isnt*, alert!
  //  if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
  //    Serial.print(i); Serial.println(" released");
   // }
   //load_raw(curr);
   //Serial.print(cap.filteredData(0));
   //Serial.print("\t");
   //Serial.print(cap.baselineData(0));
   //Serial.print("\t");
   //Serial.print(baseline_l[0]);
   //Serial.print("\t");
   //Serial.println(check_touched());
   //Serial.println(center_of_mass());
   x_pos(center_of_mass());
   //Serial.println(center_of_mass());
   //delay(500);
  //}

  // reset our state
  //lasttouched = currtouched;
  
  //Serial.print(cap.filteredData(1)); Serial.print("\t");
  //Serial.println(cap.baselineData(1));

  // comment out this line for detailed data from the sensor!
  return;
  
  // debugging info, what
  Serial.print("\t\t\t\t\t\t\t\t\t\t\t\t\t 0x"); Serial.println(cap.touched(), HEX);
  Serial.print("Filt: ");
  for (uint8_t i=0; i<12; i++) {
    Serial.print(cap.filteredData(i)); Serial.print("\t");
  }
  Serial.println();
  Serial.print("Base: ");
  for (uint8_t i=0; i<12; i++) {
    Serial.print(cap.baselineData(i)); Serial.print("\t");
  }
  Serial.println();
  
  // put a delay so it isn't overwhelming
  delay(100);
}

void load_raw(uint16_t* dat){
  for(int i = 0; i < NUM_LINES; i++){
    dat[i] = cap.filteredData(i);
  }
}

void check_touched(int* a, int* b){
  //uint16_t retval = 0;
  int counter = 0;
  *a = -1;
  *b = -1; //Assuming errors
  for(int i = 0; i < NUM_LINES; i++){
    if(cap.filteredData(i) < (baseline_l[i] - TOUCH_THRESHHOLD)){
    //if(cap.filteredData(i) < baseline_l[i]){
      //retval |= ( 1 << i);
      if(counter == 0){
        *a = i;
        counter++;
      }else{
        *b = i;
        break; //Don't want to over assign just in case
      }
    }
  }
 
  //return retval;
}
    
    
    
void initialize_state(){
  /*Zero out arrays*/
  for(int i = 0; i < NUM_LINES; i++){
    curr[i] = 0;
    last[i] = 0;
  }
  /*Calibrate baseline value*/
  int prev [3][NUM_LINES];
  uint16_t done = 0;
  memset(prev, 0, sizeof(prev));
  /*Don't want to be checking for lines that aren't there*/
  for( int j = NUM_LINES; j < 16; j++){
    done |= (1 << j);
  }
  /*Only set baseline once it has settled*/
  while(done != 65535){
    for(int k = 0; k < NUM_LINES; k++){
      prev[0][k] = prev[1][k];
      prev[1][k] = prev[2][k];
      prev[2][k] = cap.baselineData(k);
      if((prev[0][k] == prev[1][k]) && (prev[0][k] == prev[2][k])){// They are all equal
        baseline_l[k] = prev[0][k] + OFFSET; 
        done |= (1 << k );
      }
    }
    Serial.println(done);
    delay(200);
  }
  cap.setThreshholds(5,5); //Experiment because using conflicting systems
  Serial.println(baseline_l[0]);
}

/*Center of mass equation*/
/*int center_of_mass(){
  int work = 0;
  int distance = 0;
  int ret = 0;
  for(int i = 0; i < NUM_LINES; i++){
    work += ((baseline_l[i] - cap.filteredData(i)) * tape_distance(i));
    distance += tape_distance(i);
  }
  ret = (work/distance);
  if((work == 0 ) || (distance == 0)){ return 0; }
  return ret;
  //return LINE_MAX - ret; //Because the numbers go down instead of up
}*/

int center_of_mass(){
  int work = 0;
  int distance = 0;
  int touched1 = 0;
  int touched2 = 0;
  check_touched(&touched1,&touched2);
  if( !((touched1 == -1) && (touched2 == -1))){
    work += ((baseline_l[touched1] - cap.filteredData(touched1)));
    //distance += tape_distance(touched1);
    if(touched2 != -1){
      work += ((baseline_l[touched2] - cap.filteredData(touched2)) * TAPE_WIDTH);
      distance += TAPE_WIDTH;
    }
  }
  //Serial.print(work); Serial.print("\t"); Serial.println(distance);
  //Serial.print(touched1); Serial.print("\t"); Serial.println(touched2);
  if((work == 0 )|| (distance == 0)){ //Catches our error case nicely
    return 0;
  }

  return (work/distance) + (touched1 * TAPE_WEIGHT); //Where work/distance should be some fraction of tape WEIGHT
  
}

int tape_distance(int tape_num){
  int ret = 0;
  //ret += (TAPE_WIDTH/2); //Strongest pull from center of tape but measuring from leftmost edge of first tape
  ret += (tape_num * TAPE_WIDTH); //1 full tape width between 2 mid tapes after offset 0 indexed
  ret += (tape_num * TAPE_SPACE); //Add in the gaps 0 indexed
  return ret;
}

/*NEOPixel stuff*/
void x_pos(int pos){
  if(pos < 0){pos = 0;}
  int sect = LINE_MAX / strip.numPixels();
  for(int i = 0; i < strip.numPixels(); i++){
    if(pos > sect){ //Full pixel
      strip.setPixelColor(i, strip.Color(255, 0, 0));
      pos -= sect;
    } else{ //Partial / no color
      strip.setPixelColor(i, strip.Color(pos * 8, 0 , 0));
      pos = 0;
    }
    strip.show();
  }
}
