/**
 * JET-React Game
 * A 1-dimension game using a long LED strip to represents a world where you try to collect sparkling gems, and run from demons
 */

#include <Adafruit_NeoPixel.h>

#define PIN 6
#define STRIP_LEN 180

Adafruit_NeoPixel theStrip = Adafruit_NeoPixel(STRIP_LEN, PIN, NEO_GRB + NEO_KHZ800);
uint32_t sabreColor = theStrip.Color(0,127,127);
uint32_t headColor = theStrip.Color(0,255,255);
uint32_t rubyColor = theStrip.Color(255,0,0);
uint32_t emeraldColor = theStrip.Color(0,255,0);
uint32_t sapphireColor = theStrip.Color(0,0,255);
uint32_t goldColor = theStrip.Color(255,127,0);

uint32_t offColor = theStrip.Color(0,0,0);

class Jetter
{
  uint16_t head;      // the head position of light sabre
  uint32_t head_color;  
  uint32_t on_color;  // the color of light sabre
  uint32_t off_color; // turn off light
  // Adafruit_NeoPixel strip;

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
  
  void jetTo_original(uint16_t pos, Adafruit_NeoPixel&  strip) {
    uint16_t i=0;

    if(head == pos) {
      Serial.println("no change");
      Serial.println();
      return;
    }
/*    
    Serial.print("JetTo position ");
    Serial.println( pos );
    Serial.print("From head ");
    Serial.println(head);
*/    
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


  void jetTo(uint16_t pos, Adafruit_NeoPixel&  strip) {
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
        strip.setPixelColor(i+2, on_color);
        strip.setPixelColor(i+1, head_color);
        strip.setPixelColor(i, head_color);
        strip.setPixelColor(i-1, head_color);
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
        strip.setPixelColor(i+1, head_color);
        strip.setPixelColor(i, head_color);
        strip.setPixelColor(i-1, head_color);
        strip.setPixelColor(i-2, on_color);
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

  public:
  Sparkle(uint32_t c, long d) {
    color = c;
    duration = d;
    pos = random(1,STRIP_LEN);
    lightTime = millis();
    darkTime = millis();
    // start with dark
    darkDuration = random(2000, 10000);
    state = 0;
  }

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
        darkDuration = random(5000, 7000);
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
      }
    }
  }
};


Jetter sabre(3, sabreColor, headColor);
Sparkle ruby(rubyColor, 2000);
Sparkle emerald(emeraldColor, 3000);
Sparkle sapphire(sapphireColor, 4000);
Sparkle gold(goldColor, 2000);

void setup() {
  Serial.begin(9600);
  theStrip.begin();
  theStrip.show();
  
  sabre.attachStrip(theStrip);
}


void loop() {
  uint16_t spot = random(1,180);
  Serial.print("looping  ");
  Serial.println( spot );
  
  sabre.jetTo(spot, theStrip);
  ruby.updateLight(theStrip);
  emerald.updateLight(theStrip);
  sapphire.updateLight(theStrip);
  gold.updateLight(theStrip);
  
  delay(500);
}
