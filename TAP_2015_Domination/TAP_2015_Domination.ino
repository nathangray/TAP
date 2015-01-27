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

#include <Wire.h> // Enable this line if using Arduino Uno, Mega, etc.
//#include <TinyWireM.h> // Enable this line if using Adafruit Trinket, Gemma, etc.

#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"



#define STATE_SET  0
#define STATE_PLAY 1
#define STATE_WIN  2
#define STATE_IDLE 3

// Structure for team
typedef struct {
  byte button;
  Adafruit_7segment 7seg;
} Team;
