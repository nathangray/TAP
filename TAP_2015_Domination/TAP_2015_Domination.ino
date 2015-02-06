/**
Domination / Timer for Tactical Anarchy Paintball

Copyright (c) 2015 Nathan Gray

There are 2 teams, 2 buttons and 2 timers.  Some blinkenlights too.

States:
SET -> Play -> Win -> Idle -> Sleep

On power on we enter into SET mode, where you can press the buttons to
increase or decrease the target score / time for the game.  Red decreases,
Blue increase.  Press both to start.

When the game starts,
this thing is neutral.  As soon as one team presses their button, 
their timer starts.  If the other team presses their button, the 
first team's timer stops, and the second team's timer starts.  First
team to the set time for the game wins.


*/

// For interrupt based timing
#include <Metro.h>

// Using I2C ...
#include <Wire.h>
// ... To talk to the 7-Segment displays
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

// Blinkenlight strips
#include <FastLED.h>

// For software debouncing button presses
#include <Bounce2.h>

/*
 *System defines
 */
#define LED_COUNT 46
#define TEAM_COUNT 2
#define BOUNCE_TIME 50
#define MAX_SCORE 2*60*60 // Max 2 hour

// System states
#define STATE_SET  0
#define STATE_PLAY 1
#define STATE_WIN  2
#define STATE_IDLE 3

#define RED_TEAM 0
#define BLUE_TEAM 1
// These can't be variables due to FastPixel
#define RED_LED_PIN 11
#define BLUE_LED_PIN 13

// Structure for teams
struct Team {
  int score;
  byte top_led;
  
  byte button_pin;
  byte led_pin;
  Bounce bounce;
  Adafruit_7segment display;
  byte display_address;
  byte hue;
  CRGB leds[LED_COUNT];
};

Team teams[] = {
  {0, 0, 10, RED_LED_PIN, Bounce(), Adafruit_7segment(), 0x70, HUE_RED},
  {0, 0, 12, BLUE_LED_PIN, Bounce(), Adafruit_7segment(), 0x71, HUE_BLUE},
};

Metro game_timer = Metro(1000);
Metro led_animate = Metro(50);

// State variables
byte state = STATE_SET;
int target_score = 5*60; // 5 minutes
Team* owner = NULL;              // Who's owning

// Light animation
int led_step = 0;
int center = 0;
#define FADE_STEPS 16
uint8_t fade[] = {128,173,215,243,254,245,219,179,131,83,41,13,2,11,37,77};
//{128,173,215,243,254,245,219,179,131,83,41,13,2,11,37,77};

void setup() {
  // Set up lights
  FastLED.addLeds<NEOPIXEL, RED_LED_PIN>(teams[RED_TEAM].leds, LED_COUNT);
  FastLED.addLeds<NEOPIXEL, BLUE_LED_PIN>(teams[BLUE_TEAM].leds, LED_COUNT);
  
  for(byte i = 0; i < TEAM_COUNT; i++)
  {
    // Set up button
    pinMode(teams[i].button_pin, INPUT_PULLUP);
    teams[i].bounce.attach(teams[i].button_pin);
    teams[i].bounce.interval(BOUNCE_TIME);
    
    // Set up 7 segment displays
    teams[i].display.begin(teams[i].display_address);
    teams[i].display.drawColon(true);
  }
  
  // More LED
  for(int j = 0; j < LED_COUNT; j++)
  {
    for(byte i = 0; i < TEAM_COUNT; i++)
    {
      teams[i].leds[j] = CHSV(teams[i].hue, 255, 128);
    }
    FastLED.show();
    delay(25);
  }
}

void loop() {
  switch(state)
  {
    case STATE_SET: set(); break;
    case STATE_PLAY: play(); break;
    case STATE_WIN: win(); break;
    case STATE_IDLE: idle(); break;
  }
  
}

// This keeps everything ticking
void loop_checks()
{
  for(byte i = 0; i < TEAM_COUNT; i++)
  {
    teams[i].bounce.update();
  }
  if(led_animate.check())
  {
    
    if(owner != NULL)
    {
      ripple(owner);
      owner->leds[owner->top_led] = CHSV(owner->hue, 255,255);
    }
    FastLED.show();
  }
}

/**
 * Just after power-on, we'd like to change the setup
 */
void set() {
  // Start in set mode
  for(byte i = 0; i < TEAM_COUNT; i++)
  {
    teams[i].display.blinkRate(2);
  }

  while (state == STATE_SET)
  {
    loop_checks();
    
    // Done setup -> go to PLAY
    if(!teams[RED_TEAM].bounce.read() && !teams[BLUE_TEAM].bounce.read())
    {
      state = STATE_PLAY;
      
      return;
    }
      
    if(teams[RED_TEAM].bounce.fell())
    {
      target_score -= 60;
    }
    if(teams[BLUE_TEAM].bounce.fell())
    {
      target_score += 60;
    }
    constrain(target_score, 60, MAX_SCORE);
    // Set displays directly
    for(byte i = 0; i < TEAM_COUNT; i++)
    {
      show_time(teams[i].display, target_score);
    }
  }
}

/**
 * Time to play
 */
void play() {  
  for(byte i = 0; i < TEAM_COUNT; i++)
  {
    teams[i].display.blinkRate(0);
    show_time(teams[i].display, 0);
    // More LED
    for(int j = 0; j < LED_COUNT; j++)
    {
      teams[i].leds[j] = CRGB::Black;
    }
    FastLED.show();
  }
  
  while (state == STATE_PLAY)
  {
    loop_checks();
    
    for(byte i = 0; i < TEAM_COUNT; i++)
    {
      if(teams[i].bounce.fell())
      {
        // Dim
        if(owner != NULL)
        {
          fill_solid(&(owner->leds[0]), owner->top_led, CHSV(owner->hue, 255, 100));
        }
        owner =& teams[i];
        fill_solid(&(owner->leds[0]), owner->top_led, CHSV(owner->hue, 255, 255));
      }
    }
    
    if(game_timer.check())
    {
      if(owner != NULL)
      {
        owner->score++;
        owner->top_led = (owner->score * LED_COUNT) / target_score;
        show_time(owner->display, owner->score);
        if(owner->score >= target_score)
        {
          state = STATE_WIN;
          return;
        }
      }
    }
  }
}

/**
 * Yay, someone won
 */
void win() {
  for(byte i = 0; i < TEAM_COUNT; i++)
  {
    for(byte j = 0; j < LED_COUNT; j++)
    {
      teams[i].leds[j]= CHSV(owner->hue, 255,255);
      FastLED.show();
    }
  }
}

/**
 * Nothing happening, look cool
 */
void idle() {
}

/**
 * Display the given time on the given display
 */
void show_time(Adafruit_7segment &disp, int number)
{
  disp.writeDigitNum(0,number/600);
  disp.writeDigitNum(1,(number/60)%10);
  disp.writeDigitNum(3,(number%60)/10);
  disp.writeDigitNum(4,(number%60)%10);
  disp.writeDisplay();
}

void ripple(struct Team *t) {
  if(t->top_led < 3) return;
  for(byte i = 0; i < FADE_STEPS && i < t->top_led; i++)
  {
    t->leds[wrap(t->top_led,i+led_step)] = CHSV(t->hue, 255, fade[i]);
  }
  led_step = wrap(t->top_led,led_step + 1);
}
int wrap(byte top_led, int step) {
  if(step < 0) return top_led + step;
  if(step > top_led -1) return step - top_led ;
  return step;
} // wrap()
