/**
 * JET-React Game
 * A 1-dimension game using a long LED strip to represents a world where you try to collect sparkling gems, and run from demons
 */

#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>

#define PIN_LED 6
#define PIN_START 7
#define STRIP_LEN 180
#define PIN_UP 8
#define PIN_DOWN 9
#define PIN_LEFT 10
#define PIN_RIGHT 11

// game state
#define GAME_STARTED 1
#define GAME_OVER 2
#define GAME_IDLE 0

SoftwareSerial mySerial(16, 17); // RX, TX
Adafruit_NeoPixel theStrip = Adafruit_NeoPixel(STRIP_LEN, PIN_LED, NEO_GRB + NEO_KHZ800);
uint32_t sabreColor = theStrip.Color(0,127,127);
uint32_t headColor = theStrip.Color(0,255,255);
uint32_t offColor = theStrip.Color(0,0,0);

uint32_t waspColor = theStrip.Color(255,0,0);
uint32_t emeraldColor = theStrip.Color(0,255,0);
uint32_t sapphireColor = theStrip.Color(0,0,255);
uint32_t goldColor = theStrip.Color(255,127,0);
uint32_t amethystColor = theStrip.Color(255,0,255);

class Jetter
{
  uint16_t head;      // the head position of light sabre
  uint32_t head_color;  
  uint32_t on_color;  // the color of light sabre
  uint32_t off_color; // turn off light

  public:
  Jetter(uint16_t head0, uint32_t on_color0, uint32_t head_color0) {
    head = head0;
    on_color = on_color0;
    head_color = head_color0;
  }

  void attachStrip(Adafruit_NeoPixel& strip) {
    off_color = strip.Color(0,0,0);

    jetTo(head, strip);
  }

  // This saber will show a bright head + the full length of dim saber
  void jetTo_original(uint16_t pos, Adafruit_NeoPixel&  strip) {
    uint16_t i=0;

    if(head == pos) {
      Serial.println("no change");
      Serial.println();
      return;
    }

    if(head < pos) {
      for(i=head; i>head-3; i--) {
        strip.setPixelColor(i, on_color);
      }
      
      // turn on color from head -> pos
      Serial.println("Go up");
      for(i=head; i<=pos; i++) {
        strip.setPixelColor(i, on_color);
        strip.show();
      }
    }
    else {
      // turn off light from head -> pos
      Serial.println("Go down");
      for(i=head; i>pos; i--) {
        strip.setPixelColor(i, off_color);
        strip.show();
      }      
    }

    for(i=pos; i > pos-3; i--) {
      strip.setPixelColor(i, head_color);
    }
    strip.show();

    head = pos;
  }

  // this saber will just show the bright head
  void jetTo(uint16_t pos, Adafruit_NeoPixel&  strip) {
    uint16_t i=0;

    if(head == pos) {
      Serial.println("no change");
      Serial.println();
      return;
    }

    if(head < pos) {
      // turn on color from head -> pos
      Serial.println("Go up");
      for(i=head; i<=pos; i++) {
        strip.setPixelColor(i+2, head_color);
        strip.setPixelColor(i+1, head_color);
        strip.setPixelColor(i, head_color);
        strip.setPixelColor(i-1, on_color);
        strip.setPixelColor(i-2, on_color);
        // turn off tail
        strip.setPixelColor(i-3, off_color);
        strip.show();
      }
    }
    else {
      // turn off light from head -> pos
      Serial.println("Go down");
      for(i=head; i>pos; i--) {
        strip.setPixelColor(i+2, on_color);
        strip.setPixelColor(i+1, on_color);
        strip.setPixelColor(i, head_color);
        strip.setPixelColor(i-1, head_color);
        strip.setPixelColor(i-2, head_color);
        // turn off tail        
        strip.setPixelColor(i+3, off_color);
        strip.show();
      }      
    }

    head = pos;
  }
};


class Joystick 
{
  int upPin;
  int downPin;
  int leftPin;
  int rightPin;
  int stickDir = 0;   // joystick move direction
  
  public:
  Joystick(int up, int down, int left, int right) {
    upPin = up;
    downPin = down;
    leftPin = left;
    rightPin = right;

    pinMode(upPin, INPUT_PULLUP);
    pinMode(downPin, INPUT_PULLUP);
    pinMode(leftPin, INPUT_PULLUP);
    pinMode(rightPin, INPUT_PULLUP);
  }

  int checkDir() {
    int readUp = digitalRead(upPin);
    int readDown = digitalRead(downPin);
    int readLeft = digitalRead(leftPin);
    int readRight = digitalRead(rightPin);

    if(readUp==1 && readDown==1 && readLeft==1 && readRight==1) {
      stickDir = 0;     // no move
    }
    else if(readUp==0 && readDown==1 && readLeft==1 && readRight==1) {
      stickDir = 1;     // up
    }
    else if(readUp==1 && readDown==0 && readLeft==1 && readRight==1) {
      stickDir = 2;     // down
    }
    else if(readUp==1 && readDown==1 && readLeft==0 && readRight==1) {
      stickDir = 3;     // left
    }
    else if(readUp==1 && readDown==1 && readLeft==1 && readRight==0) {
      stickDir = 4;     // right
    }

    return stickDir;
  }
};


class Sparkle {
  uint16_t pos;
  uint32_t color;
  long  duration;
  long  lightTime;
  long  darkTime;
  long  darkDuration;
  int   state = 0;   // 0 - dark, 1 - light
  int   stingCount;

  public:
  Sparkle(uint32_t c, long d) {
    color = c;
    duration = d;
    pos = random(1,STRIP_LEN);
    lightTime = millis();
    darkTime = millis();
    // start with dark
    darkDuration = random(500, 3000);
    state = 0;
    stingCount = 0;
  }

  // got a gem!
  void celebrate(uint16_t head, Adafruit_NeoPixel&  strip) {
    for(int i=0; i<10; i++) {
        int r = random(10,255); 
        int g = random(10,255);
        int b = random(10,255);
        uint32_t color = strip.Color(r,g,b);
        strip.setPixelColor(head+2, color);
        strip.setPixelColor(head+1, color);
        strip.setPixelColor(head, color);
        strip.setPixelColor(head-1, color);
        strip.setPixelColor(head-2, color);
        strip.show();
        delay(100);
     }
  }

  // stung by wasp
  void blinkHead(uint16_t head, Adafruit_NeoPixel& strip) {
    uint32_t color = waspColor;
    
    for(int k=0; k<10; k++) {
        if( k%2 == 0 ) {
          color = offColor;
        }
        else {
          color = waspColor;
        }
        strip.setPixelColor(head+2, color);
        strip.setPixelColor(head+1, color);
        strip.setPixelColor(head, color);
        strip.setPixelColor(head-1, color);
        strip.setPixelColor(head-2, color);
        strip.show();
        delay(150);
    }
  }
  
  // see if captured gem
  void detect(uint16_t head, Adafruit_NeoPixel&  strip) {
    if( state == 1 ) {
      int hitPoint = 0;
      if( abs(head-pos) == 0 ) {
        hitPoint = 100;
      }
      else if( abs(head-pos) == 1 ) {
        hitPoint = 50;
      }
      else if( abs(head-pos) == 2 ) {
        hitPoint = 25;
      }
      else {
        hitPoint = 0;
      }

      if( hitPoint > 0 ) {
        // report points - write it through serial port to NodeMCU
        mySerial.print("C");
        mySerial.println(hitPoint);
        
        // celebrate
        celebrate(head, strip);

        // now turn off this gem and make it dark
        Serial.println("going to dark");
        strip.setPixelColor(pos, offColor);
        strip.show();
  
        darkTime= millis();
        darkDuration = random(5000, 7000);
        state = 0;        
      }
    }
  }

  // For Wasp to move around
  void moveLight(uint16_t head, Adafruit_NeoPixel& strip) {
    if( state == 1 ) {
      strip.setPixelColor(pos, offColor);
      if( pos < head ) {
        pos++;
      }
      else {
        pos--;
      }

      // detect wasp sting
      if( abs(pos-head) < 3 ) {
        stingCount++;      
      }
      if( stingCount > 3 ) {
        // report sting
        mySerial.println("D100");

        // show light effect - blink head
        blinkHead(head, strip);
      }
    }
  }


  // turn on/off light randomly
  void updateLight(Adafruit_NeoPixel&  strip) {
    unsigned long currMills = millis();

    if( state == 1 ) {
      // light state
      if( currMills - lightTime < duration ) {
        // light up for duration
        strip.setPixelColor(pos, color);
        strip.show();
      }
      else {
        // make it dark
        Serial.println("going to dark");
        strip.setPixelColor(pos, offColor);
        strip.show();
  
        darkTime= millis();
        darkDuration = random(1500, 3500);
        state = 0;
      }
    }
    else if( state == 0 ) {
      // dark state
      if( currMills - darkTime < darkDuration) {
        strip.setPixelColor(pos, offColor);
        strip.show();      
      } 
      else {
        // now light it up
        Serial.println("going to light....");
        pos = random(1,STRIP_LEN);
        
        strip.setPixelColor(pos, color);
        strip.show();
  
        lightTime = millis();
        state = 1;
        // reset sting count
        stingCount = 0;

      }
    }
  }
};


class GameState {
  int state;
  long startTime;       // game start
  long endTime;         // game over time
  long idleTime;        // mark idle time start
  long lastActiveTime;  // last time the user moved joystick
  long IDLE_TIME_START = 10000; // if no activity for 20 secs, go in idle mode
  long GAME_DURATION = 15000; // 30 seconds
  
  public:
  GameState() {
    // start with game-over mode
    state = GAME_OVER;
    startTime = millis();
    endTime = millis();
    lastActiveTime = millis();
  }

  void setActive() {
    lastActiveTime = millis();  
  }

  void start() {
    startTime = millis();
    state = GAME_STARTED;
    Serial.println("Game started");
  }

  int check() {
    long currTime = millis();
    long duration = currTime - startTime;

    // game time up
    if( state == GAME_STARTED && duration > GAME_DURATION ) {
      state = GAME_OVER;
      endTime = currTime;
      Serial.println("Game over");
    }

    if( state == GAME_OVER && (currTime - endTime) > IDLE_TIME_START ) {
      state = GAME_IDLE;
      Serial.println("Game going idle mode"); 
    }

    return state;
  }
};
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return theStrip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return theStrip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return theStrip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<theStrip.numPixels(); i++) {
      theStrip.setPixelColor(i, Wheel((i+j) & 255));
    }
    theStrip.show();
    delay(wait);
  }
}

void clearStrip() {
  uint16_t i = 0;
  for(i=0; i<STRIP_LEN; i++) {
    theStrip.setPixelColor(i, offColor); 
  }
  theStrip.show();
}

void showHead(uint16_t head) {
  
}

Jetter sabre(60, sabreColor, headColor);
Joystick stick(PIN_UP, PIN_DOWN, PIN_LEFT, PIN_RIGHT);
GameState game;

Sparkle gold(goldColor, 1500);  // what color, stay for how long
Sparkle amethyst(amethystColor, 2000);
Sparkle emerald(emeraldColor, 2500);
Sparkle sapphire(sapphireColor, 3000);
Sparkle wasp(waspColor, 3500);

// Left move to capture gem
void detectLeftMove(uint16_t head, Adafruit_NeoPixel& strip)
{
  // current head position
  amethyst.detect(head, strip);
  emerald.detect(head, strip);
  sapphire.detect(head, strip);
  gold.detect(head, strip);
}

// Right move to knock off wasp
void detectRightMove(uint16_t head, Adafruit_NeoPixel& strip)
{
  // current head position
  wasp.detect(head, strip);
}

uint16_t spot = 0;
void setup() {
  Serial.begin(19200);
  mySerial.begin(9600);
  
  theStrip.begin();
  theStrip.show();

  // to start game
  pinMode(PIN_START, INPUT_PULLUP);
  
  sabre.attachStrip(theStrip);
  spot = random(1,180);
}


void loop() {

  int state = game.check();
  Serial.print("Game state:");
  Serial.println( state );
  

  if( digitalRead(PIN_START) == LOW && state != GAME_STARTED) {
    Serial.println( "Starting game........");
    game.start();
    clearStrip();
    spot = 90;
    state = game.check();
  }
  
  int dir = stick.checkDir();
  switch(dir) {
    case 1: // up
      spot = spot + 2;  // move up 3 lights
      break;
    case 2: //down
      spot = spot - 2;
      break;
    case 3: // gem
      detectLeftMove(spot, theStrip);
      break;
    case 4: // wasp
      detectRightMove(spot, theStrip);
      break;
    default:
      // no movement, do nothing
      break;
  }
  
  if( dir > 0 && dir < 5) {
    game.setActive();
  }

  // protect strip light spot
  if(spot < 0 ) {
    spot = 3;
  }
  if(spot > STRIP_LEN) {
    spot = STRIP_LEN-3;
  }

  if( state == GAME_IDLE) {
    Serial.println("TBD: playing GAME_IDLE effect");
    spot = random(3, 177);
  }

  if( state == GAME_STARTED || state == GAME_IDLE) {
    sabre.jetTo(spot, theStrip);
    
    amethyst.updateLight(theStrip);
    emerald.updateLight(theStrip);
    sapphire.updateLight(theStrip);
    gold.updateLight(theStrip);
    
    wasp.moveLight(spot, theStrip);
    wasp.updateLight(theStrip);
  }
  else if( state == GAME_OVER) {
    Serial.println("TBD: playing GAME_OVER effect");
    rainbow(1);
  }

  delay(10);
}
