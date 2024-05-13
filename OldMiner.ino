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

struct entity {
  int type;
  int x;
  int y;
};

float angle = 0;
float length = 5;
int claw_x;
int claw_y;
int array_pos_obj_in_claw = -1;
entity obj_in_claw;

const int LEFT = 1;
const int RIGHT = 2;
int direction = LEFT;

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
const int CLAW = 8;

int entity_radius(int type) {
  switch(type){
    case BIG_ROCK:
    case BIG_GOLD:
      return 8;
    case SMALL_ROCK:
    case SMALL_GOLD:
      return 5;
    case DIAMOND:
      return 4;
    // TODO - refactor type vs sprite position in sheet to make animations work
    case MOUSE1:
      return 5;
  }
}

int detect_collision(entity e, int x, int y) {
  return sqrt( (x-e.x) * (x-e.x) + (y-e.y) * (y-e.y)) <= entity_radius(e.type);
}

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

float distance() {
  return 0;
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

  claw_x = 64 + length*sin(angle);
  claw_y = 5 + length*cos(angle);

  entity e;
  for (int i = 0; i < NUM_ENTITIES; i++) {
    e = entities[i];
    if (e.type == NOTHING) continue;
    Sprites::drawOverwrite(e.x, e.y, sprites, e.type);

    if (detect_collision(e, claw_x, claw_y)) {
      array_pos_obj_in_claw = i;
      state = REELING_OBJ;
    }
  }

  arduboy.drawLine(64, 4, claw_x, claw_y, WHITE);

  if (state == AIMING) {
    // TODO - maybe add some acceleration, rather than constant change of angle
    if (direction == RIGHT) {
      if (angle < PI/2) angle += 0.02;
      else direction = LEFT;
    }

    if (direction == LEFT) {
      if (angle > -PI/2) angle -= 0.02;
      else direction = RIGHT;
    }
    if (arduboy.pressed(DOWN_BUTTON)) {
      state = SHOOTING;
    }
  }

  if (state == SHOOTING) {
    length += 2;

    if (claw_x < 0 ||
        claw_x > 128 ||
        claw_y > 64) {
      state = REELING_EMPTY;
    }
  }

  if (state == REELING_EMPTY) {
    length -= 1;
    if (length < 5) {
      length = 5;
      angle = -PI/4;
      state = AIMING;
    }
  }

  if (state == REELING_OBJ) {
    obj_in_claw = entities[array_pos_obj_in_claw];
    obj_in_claw.x = claw_x;
    obj_in_claw.y = claw_y;

    length -= 1;
    if (length < 5) {
      length = 5;
      angle = -PI/4;
      state = AIMING;
    }
  }

  // then we finally we tell the arduboy to display what we just wrote to the display
  arduboy.display();
}
