/*
Old Miner a clone of the flash game "Gold Minder"
https://www.crazygames.com/game/gold-miner

Hello, World! example
June 11, 2015
Copyright (C) 2015 David Martinez
All rights reserved.
This code is the most basic barebones code for writing a program for Arduboy.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.
*/

#include <Arduboy2.h>
#include "sprites.h"

// make an instance of arduboy used for many functions
Arduboy2 arduboy;

float angle = 0;
float length = 5;

const int AIMING = 1;
const int SHOOTING = 2;
const int REELING_EMPTY = 3;
const int REELING_OBJ = 4;
int state = AIMING;

// used as placeholder when somthing gets removed from scene
const int NOTHING = -1;
const int BIG_ROCK = 0;
const int BIG_GOLD = 1;
const int SMALL_ROCK = 2;
const int SMALL_GOLD = 3;
const int DIAMOND = 4;
const int MOUSE1 = 5;
const int MOUSE2 = 6;
const int DYNAMITE = 7;

struct entity {
  int type;
  int x;
  int y;
};

const int NUM_ENTITIES = 4;
entity entities[NUM_ENTITIES] = {
  {type: SMALL_ROCK, x: 20, y: 20},
  {type: BIG_GOLD, x: 10, y: 40},
  {type: BIG_GOLD, x: 50, y: 10},
  {type: SMALL_GOLD, x: 80, y: 50}
};

// This function runs once in your game.
// use it for anything that needs to be set only once in your game.
void setup() {
  // initiate arduboy instance
  arduboy.begin();

  // here we set the frame rate to 15, we do not need to run at
  // default 60 and it saves us battery life
  arduboy.setFrameRate(60);
}


// our main game loop, this runs once every cycle/frame.
// this is where our game logic goes.
void loop() {
  // pause render until it's time for the next frame
  if (!(arduboy.nextFrame()))
    return;

  // first we clear our screen to black
  arduboy.clear();

  // we set our cursor 5 pixels to the right and 10 down from the top
  // (positions start at 0, 0)
  arduboy.setCursor(4, 9);

  // then we print to screen what is in the Quotation marks ""
  //arduboy.print(F("Hello, world!"));

  entity e;
  for (int i = 0; i < NUM_ENTITIES; i++) {
    e = entities[i];
    if (e.type == NOTHING) continue;
    Sprites::drawOverwrite(e.x, e.y, sprites, e.type);
  }

  arduboy.drawLine(64, 4, 64 + length*sin(angle), 5 + length*cos(angle), WHITE);

  if (state == AIMING) {
    // TODO - maybe add some acceleration, rather than constant change of angle
    if (arduboy.pressed(RIGHT_BUTTON)) {
      if (angle < PI/2) angle += 0.05;
    }

    if (arduboy.pressed(LEFT_BUTTON)) {
      if (angle > -PI/2) angle -= 0.05;
    }
    if (arduboy.pressed(A_BUTTON)) {
      state = SHOOTING;
    }
  }

  if (state == SHOOTING) {
    length += 2;

    if (64 + length*sin(angle) < 0 ||
        64 + length*sin(angle) > 128 ||
        5 + length*cos(angle) > 64) {
      state = REELING_EMPTY;
    }
  }

    if (state == REELING_EMPTY) {
    length -= 1;
    if (length < 5) {
      length = 5;
      state = AIMING;
    }
  }

  // then we finally we tell the arduboy to display what we just wrote to the display
  arduboy.display();
}
