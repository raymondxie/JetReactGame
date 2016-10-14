/**
 * JET-React Game
 * A 1-dimension game using a long LED strip to represents a world where you try to collect sparkling gems, and run from demons
 */

#include <Adafruit_NeoPixel.h>

#define PIN 6

Adafruit_NeoPixel theStrip = Adafruit_NeoPixel(20, PIN, NEO_GRB + NEO_KHZ800);
uint32_t sabreColor = theStrip.Color(64,0,64);

class Jetter
{
  uint16_t head;      // the head position of light sabre
  uint32_t on_color;  // the color of light sabre
  uint32_t off_color; // turn off light
  // Adafruit_NeoPixel strip;

  public:
  Jetter(uint16_t head0, uint32_t color0) {
    head = head0;
    on_color = color0;
  }

  void attachStrip(Adafruit_NeoPixel& strip) {
    off_color = strip.Color(0,0,0);

    jetTo(head, strip);
  }
  
  void jetTo(uint16_t pos, Adafruit_NeoPixel&  strip) {
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


Jetter sabre(3, sabreColor);

void setup() {
  Serial.begin(9600);
  theStrip.begin();
  theStrip.show();
  
  sabre.attachStrip(theStrip);
}


void loop() {
  uint16_t spot = random(1,12);
  Serial.print("looping  ");
  Serial.println( spot );
  
  sabre.jetTo(spot, theStrip);
  
  // theStrip.setPixelColor(spot, theStrip.Color(0,0,127));
  // theStrip.show();
  delay(2000);
}
