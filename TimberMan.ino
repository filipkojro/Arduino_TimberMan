#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>


#define OLED_MOSI 10
#define OLED_CLK 8
#define OLED_DC 7
#define OLED_CS 5
#define OLED_RST 9

#define L_BUTTON 4
#define R_BUTTON 3


// Create the OLED display
Adafruit_SH1107 display = Adafruit_SH1107(64, 128,OLED_MOSI, OLED_CLK, OLED_DC, OLED_RST, OLED_CS);


static const unsigned char PROGMEM branchLsprite[] = {
  0b00000111, 0b11111110, 0b00000000, 
  0b00011111, 0b11111111, 0b11111111, 
  0b00111111, 0b11111111, 0b11111111, 
  0b01111111, 0b00000000, 0b00001111, 
  0b01110000, 0b00000000, 0b00000001, 
  0b01100000, 0b00000000, 0b00000000, 
  0b01000000, 0b00000000, 0b00000000, 
  0b01000000, 0b00000000, 0b00000000
};

static const unsigned char PROGMEM branchRsprite[] = {
  0b00000000, 0b01111111, 0b11100000,
  0b11111111, 0b11111111, 0b11111000,
  0b11111111, 0b11111111, 0b11111100,
  0b11110000, 0b00000000, 0b11111110,
  0b10000000, 0b00000000, 0b00001110,
  0b00000000, 0b00000000, 0b00000110,
  0b00000000, 0b00000000, 0b00000010,
  0b00000000, 0b00000000, 0b00000010
};

static const unsigned char PROGMEM jack[] = {
  0b00011000, 
  0b00111100, 
  0b00111100, 
  0b00011000, 
  0b01111110, 
  0b11111111, 
  0b11011011, 
  0b10011001, 
  0b00011000, 
  0b00011000, 
  0b00011000, 
  0b00111100, 
  0b00111100, 
  0b01100110, 
  0b01100110, 
  0b01000010, 
};

//if tree on a given idex is 0 log should be on a left, 1 right
//every user input 0 log are overwritten by 1, 1 by 2, 2 by 3
//3 in random
bool tree[4];

unsigned long duration;

bool lastStateL = false;
bool lastStateR = false;

int sameSite = 0;
bool lastSite;

bool playerSite = 0;

char button;

bool alive = false;

float health = 64;
float depletion = 1;

int lastScore = 0;


void setup()   {
  Serial.begin(9600);

  //display.setContrast (0); // dim display

  pinMode(L_BUTTON, INPUT);
  pinMode(R_BUTTON, INPUT);

  // Start OLED
  display.begin(0, true); // we dont use the i2c address but we will reset!

  Serial.println("display start");

  display.setRotation(2);

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(2000);
}

void loop() {

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);

  display.println("last best");
  display.println(lastScore);
  display.println();
  display.println("start?");

  display.display();

  if(controll() != 'N'){
    alive = true;
    health = 64;
    depletion = 1;
    display.clearDisplay();

    treeInit();
    display.clearDisplay();
    display.fillRect(24, 0, 16, 96, SH110X_WHITE);
    display.fillRect(30, 96, 4, 32, SH110X_WHITE);
  }

  while (alive) {
    duration = millis();

    //clearing last branches
    for(int i = 0; i < 3; i++){
      if(tree[i]) display.drawBitmap(40, 32 * i + 10, branchRsprite, 24, 8, SH110X_BLACK);
      else display.drawBitmap(0, 32 * i + 10, branchLsprite, 24, 8, SH110X_BLACK);
    }

    display.drawBitmap(32 * playerSite + 11,104, jack, 8, 16, SH110X_BLACK);

    health -= depletion;

    button = controll();

    if(button == 'R' && tree[2] == 1){
      alive = false;
    }
    if(button == 'L' && tree[2] == 0){
      alive = false;
    }

    if(button == 'R' && tree[2] == 0){
      nextBranch();
      playerSite = 1;

      health += 5;
      depletion += 0.01;
    }
    if(button == 'L' && tree[2] == 1){
      nextBranch();
      playerSite = 0;

      health += 5;
      depletion += 0.01;
    }

    if(health > 64)health = 64;
    if(health <= 0)alive = false;
    if(!alive)if((depletion - 1) * 100 + 1 > lastScore)lastScore = (depletion - 1) * 100 + 1;

    display.drawBitmap(32 * playerSite + 11,104, jack, 8, 16, SH110X_WHITE);

    for(int i = 0; i < 3; i++){
      if(tree[i]) display.drawBitmap(40, 32 * i + 10, branchRsprite, 24, 8, SH110X_WHITE);
      else display.drawBitmap(0, 32 * i + 10, branchLsprite, 24, 8, SH110X_WHITE);
    }


    display.writeFastHLine(0, 127, health, SH110X_WHITE);
    display.writeFastHLine(health, 127, 64 - health, SH110X_BLACK);

    display.display();


    Serial.print(millis() - duration);
    Serial.print(" - ");
    Serial.print(lastScore);
    Serial.print(" - ");
    Serial.println((depletion - 1) * 100);
  }
}

void treeInit(){
  for(int i = 0; i < 3; i++){
    tree[i] = random(2);

    delay(random(5));
  }
  lastSite = tree[0];
}

void nextBranch(){
  for(int i = 2; i >= 0; i--){
    tree[i] = tree[i - 1];
  }
  tree[0] = random(2);

  if(tree[0] == lastSite)sameSite++;
  else sameSite = 0;

  if(sameSite >= 3) tree[0] = !tree[0];

  lastSite = tree[0];
}

char controll(){
  char command = 'N';

  bool nextStateL = digitalRead(L_BUTTON);
  bool nextStateR = digitalRead(R_BUTTON);

  if(! nextStateL && lastStateL)command = 'L';
  if(! nextStateR && lastStateR)command = 'R';

  lastStateL = nextStateL;
  lastStateR = nextStateR;

  return command;
}