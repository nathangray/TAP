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
#define LED_COUNT 60
#define TEAM_COUNT 2
#define BOUNCE_TIME 50
#define MAX_SCORE 60*60 // Max 1 hour

// System states
#define STATE_SET  0
#define STATE_PLAY 1
#define STATE_WIN  2
#define STATE_IDLE 3

#define RED_TEAM 0
#define BLUE_TEAM 1

// Structure for teams
struct Team {
  int score;
  
  byte button_pin;
  Bounce bounce;
  Adafruit_7segment display;
  byte display_address;
  CRGB leds[LED_COUNT];
};

Team teams[] = {
  {0, 10, Bounce(), Adafruit_7segment(), 0x70},
  {0, 11, Bounce(), Adafruit_7segment(), 0x71},
};

// State variables
byte state = STATE_SET;
byte target_score = 5*60; // 5 minutes
Team* owner;

void setup() {
  for(byte i = 0; i < TEAM_COUNT; i++)
  {
    // Set up button
    pinMode(teams[i].button_pin, INPUT_PULLUP);
    teams[i].bounce.attach(teams[i].button_pin);
    teams[i].bounce.interval(BOUNCE_TIME);
    
    // Set up 7 segment displays
    teams[i].display.begin(teams[i].display_address);
    teams[i].display.drawColon(true);
    
    // Start in set mode
    teams[i].display.blinkRate(2);
  }
}

void loop() {
  for(byte i = 0; i < TEAM_COUNT; i++)
  {
    teams[i].bounce.update();
  }
  
  switch(state)
  {
    case STATE_SET: set(); break;
    case STATE_PLAY: play(); break;
    case STATE_WIN: win(); break;
    case STATE_IDLE: idle(); break;
  }
}

/**
 * Just after power-on, we'd like to change the setup
 */
void set() {
  if(teams[RED_TEAM].bounce.fell())
  {
    target_score -= 60;
  }
  if(teams[BLUE_TEAM].bounce.fell())
  {
    target_score += 60;
  }
  constrain(target_score, 0, MAX_SCORE);
  
  // Set displays directly
  for(byte i = 0; i < TEAM_COUNT; i++)
  {
    teams[i].display.writeDigitNum(0,target_score/60,true);
    teams[i].display.writeDigitNum(1,(target_score/6)%10,true);
    teams[i].display.writeDigitNum(3,0,true);
    teams[i].display.writeDigitNum(4,0,true);
    teams[i].display.writeDisplay();
  }
}

/**
 * Time to play
 */
void play() {
}

/**
 * Yay, someone won
 */
void win() {
}

/**
 * Nothing happening, look cool
 */
void idle() {
}
