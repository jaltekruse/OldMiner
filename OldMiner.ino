/*
Old Miner a clone of the flash game "Gold Minder"
https://www.crazygames.com/game/gold-miner

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.
*/

#include <Arduboy2.h>
#include <Tinyfont.h>//830 PROGMEM - 28 RAM
#include "sprites.h"

// make an instance of arduboy used for many functions
Arduboy2 arduboy;

Tinyfont tinyfont = Tinyfont(arduboy.sBuffer, Arduboy2::width(), Arduboy2::height());

const int FONT_HEIGHT = 10;
const int SM_FONT_HEIGHT = 8;

const int DEBUG_CONTROLS = 0;

enum GameState {
  TITLE,
  STORY,
  SHOP,
  MINING,
  BANKRUPT
};

GameState game_state = TITLE;

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
// TODO - fix up later, this isn't 8 in the sprite sheet
// it is drawn from 2 different things
const int MOUSE_DIAMOND = 8;

// TODO - fix up, not used right now, but this is 8 in the sprite sheet
const int CLAW = 8;

// currently all sprites are 16x16 and smaller things are just drawn in the center of that area
const int HALF_SPRITE = 8;

struct entity {
  int type;
  float x;
  float y;
  // movement direction - mouse only for now
  int dir;
  // only for the thrown dynamite for now
  float thrown_dist;
};

float angle = 0;
float length = 5;
int claw_x;
int claw_y;
int money = 0;
int level = 1;
int dynamite_sticks = 1;

const int FPS = 60;
int time_left = 60 * FPS;

int array_pos_obj_in_claw = -1;
entity* obj_in_claw;

// updated to a real type when needed
entity thrown_dynamite = {type: NOTHING};

const int LEFT = -1;
const int RIGHT = 1;
int direction = LEFT;

void reset_to_new_day() {
  angle = 0;
  length = 5;
  direction = LEFT;
  array_pos_obj_in_claw = -1;
  time_left = 20 * FPS;
  state = AIMING;
  thrown_dynamite.type = NOTHING;

}

void reset_game() {
  reset_to_new_day();
  level = 1;
  money = 0;
  dynamite_sticks = 1;
}

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
    case MOUSE_DIAMOND:
      return 5;
    case DYNAMITE:
      return 8;
  }
}

int entity_weight(int type) {
  switch(type){
    case BIG_ROCK:
    case BIG_GOLD:
      return 5;
    case SMALL_ROCK:
    case SMALL_GOLD:
      return 3;
    case DIAMOND:
      return 1;
    // TODO - refactor type vs sprite position in sheet to make animations work
    case MOUSE1:
    case MOUSE_DIAMOND:
      return 1;
  }
}

int entity_value(int type) {
  switch(type){
    case BIG_ROCK:
      return 10;
    case BIG_GOLD:
      return 500;
    case SMALL_ROCK:
      return 5;
    case SMALL_GOLD:
      return 100;
    case DIAMOND:
      return 600;
    case MOUSE_DIAMOND:
      return 602;
    // TODO - refactor type vs sprite position in sheet to make animations work
    case MOUSE1:
      return 2;
  }
}

int detect_collision(entity* e, int x, int y) {
  return sqrt(   (x-(e->x+HALF_SPRITE)) * (x-(e->x+HALF_SPRITE))
               + (y-(e->y+HALF_SPRITE)) * (y-(e->y+HALF_SPRITE)))
         <= entity_radius(e->type);
}

const int NUM_ENTITIES = 10;
entity entities[NUM_ENTITIES] = {
  {type: SMALL_ROCK, x: 20, y: 20},
  {type: BIG_GOLD, x: 10, y: 40},
  {type: BIG_GOLD, x: 50, y: 10},
  {type: SMALL_GOLD, x: 80, y: 50},
  {type: BIG_ROCK, x: 70, y: 20},
  {type: SMALL_ROCK, x: 60, y: 70},
  {type: DIAMOND, x: 50, y: 40},
  {type: SMALL_GOLD, x: 80, y: 20},
  {type: MOUSE1, x: 15, y: 15, dir: LEFT},
  {type: MOUSE_DIAMOND, x: 65, y: 45, dir: LEFT}
};

// const int NUM_ENTITIES = 10;
// entity entities[NUM_ENTITIES] = {
//   {type: SMALL_ROCK, x: 10, y: 50},
//   {type: BIG_GOLD, x: 20, y: 50},
//   {type: BIG_GOLD, x: 30, y: 50},
//   {type: SMALL_GOLD, x: 40, y: 50},
//   {type: BIG_ROCK, x: 50, y: 50},
//   {type: SMALL_ROCK, x: 60, y: 50},
//   {type: DIAMOND, x: 70, y: 50},
//   {type: SMALL_GOLD, x: 80, y: 50},
//   {type: SMALL_GOLD, x: 90, y: 50},
//   {type: SMALL_GOLD, x: 110, y: 50}
// };

// This function runs once in your game.
// use it for anything that needs to be set only once in your game.
void setup() {
  // initiate arduboy instance
  arduboy.begin();

  // TODO - this isn't working for me
  // Serial.begin(9600);
  // Serial.print("obj 3 type: ");

  // here we set the frame rate to 15, we do not need to run at
  // default 60 and it saves us battery life
  arduboy.setFrameRate(FPS);
}

float distance() {
  return 0;
}

void game_loop() {
  time_left--;

  if (time_left < 0) {
    if (money < 0)
      game_state = BANKRUPT;
    else {
      level++;
      init_shop();
      game_state = SHOP;
    }

  }

  tinyfont.setCursor(0, 2);
  tinyfont.print("T ");
  tinyfont.print(time_left / 60);

  // then we print to screen what is in the Quotation marks ""
  // arduboy.print(F("Hello, world!"));
  // arduboy.println(entities[3].type);

  // debugging
  //Serial.println("obj 3 type: ");
  //Serial.println(entites[3].type);

  claw_x = 64 + length*sin(angle);
  claw_y = 5 + length*cos(angle);
  render_money();

  entity* e;
  for (int i = 0; i < NUM_ENTITIES; i++) {
    e = &entities[i];
    if (e->type == NOTHING) continue;

    if (e->type == MOUSE1 || e->type == MOUSE_DIAMOND) {
      // transition animation frames every 10 draw frames
      Sprites::drawPlusMask(e->x, e->y, sprites_plus_mask, (time_left / 10) % 2 == 0 ? MOUSE1 : MOUSE2);

      if (e->type == MOUSE_DIAMOND) {
        Sprites::drawPlusMask(e->x, e->y-5, sprites_plus_mask, DIAMOND);
      }

      // keep running around unless currently being reeled in
      if (array_pos_obj_in_claw != i) {
        // only move a pixel every couple of frames
        e->x += e->dir * (time_left % 3 == 0 ? 1 : 0);
        // change movement direction when hitting the wall
        if (e->x > 110 || e->x < 0) e->dir = -e->dir;
      }
    } else {
      Sprites::drawPlusMask(e->x, e->y, sprites_plus_mask, e->type);
    }

    if (state == SHOOTING & detect_collision(e, claw_x, claw_y)) {
      array_pos_obj_in_claw = i;
      state = REELING_OBJ;
    }
  }

  arduboy.drawLine(64, 4, claw_x, claw_y, WHITE);

  if (state == AIMING) {
    // TODO - maybe add some acceleration, rather than constant change of angle
    if (direction == RIGHT ||
        (DEBUG_CONTROLS && arduboy.pressed(RIGHT_BUTTON))) {
      if (angle < PI/2) angle += 0.02;
      else if (!DEBUG_CONTROLS) direction = LEFT;
    }

    if (direction == LEFT ||
        (DEBUG_CONTROLS && arduboy.pressed(LEFT_BUTTON))) {
      if (angle > -PI/2) angle -= 0.02;
      else if (!DEBUG_CONTROLS) direction = RIGHT;
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

  if (state == REELING_OBJ && thrown_dynamite.type == DYNAMITE) {
    Sprites::drawPlusMask(thrown_dynamite.x, thrown_dynamite.y, sprites_plus_mask, DYNAMITE);
    thrown_dynamite.thrown_dist += .8*(time_left % 2 == 0 ? 1 : 0);
    thrown_dynamite.x = 64 - HALF_SPRITE + thrown_dynamite.thrown_dist*sin(angle);
    thrown_dynamite.y = -HALF_SPRITE + thrown_dynamite.thrown_dist*cos(angle);
  }

  if (state == REELING_OBJ && thrown_dynamite.type != NOTHING && detect_collision(&thrown_dynamite, claw_x, claw_y)) {
      entities[array_pos_obj_in_claw].type = NOTHING;
      thrown_dynamite.type = NOTHING;
      state = AIMING;
      length = 5;
  } 

  if (state == REELING_OBJ) {
    obj_in_claw = &entities[array_pos_obj_in_claw];
    obj_in_claw->x = claw_x - HALF_SPRITE;
    obj_in_claw->y = claw_y - HALF_SPRITE;

    length -= 1.0 / entity_weight(obj_in_claw->type);

    // can only throw dynamite when reeling in something
    if (arduboy.pressed(UP_BUTTON) && dynamite_sticks > 0 && thrown_dynamite.type == NOTHING) {
      dynamite_sticks--;
      thrown_dynamite.type = DYNAMITE;
      thrown_dynamite.thrown_dist = 0;
      thrown_dynamite.x = 64 - HALF_SPRITE;
      thrown_dynamite.y = -HALF_SPRITE;
    }
    if (length < 5) {
      length = 5;
      angle = -PI/4;
      money += entity_value(obj_in_claw->type);
      obj_in_claw->type = NOTHING;
      array_pos_obj_in_claw = -1;
      state = AIMING;
    }
  }
}

void render_money() {

  Sprites::drawPlusMask(72, -5, sprites_plus_mask, DYNAMITE);
  tinyfont.setCursor(82, 2);
  tinyfont.print(dynamite_sticks);

  arduboy.setCursor(90, 0);
  arduboy.print("$ ");
  tinyfont.setCursor(97, 2);
  tinyfont.print(money);
}

// shop state
enum ItemType {
  // Used as array terminator
  NADA,
  PERMIT,
  DYNAMITE_ITEM,
  DIAMOND_POLISH,
  LUCKY_CLOVER,
  STRENGTH_DRINK,
  START_DAY
};

struct Item {
  ItemType type;
  int price;
};

const int NUM_ITEMS = 6;
Item items[NUM_ITEMS] = {
  {type: PERMIT, price: 600},
  {type: DYNAMITE_ITEM, price: -1},
  {type: NOTHING, price: -1},
  {type: NOTHING, price: -1},
  {type: NOTHING, price: -1},
  {type: START_DAY, price: -1}
};

int shop_selection;

void init_shop(){
  shop_selection = 0;
  items[0].price = 600 * pow(1.1, level);
  if (level > 1) {
    items[1].price = random( -50, 250);
  }
}

void shop_loop() {
  render_money();
  tinyfont.setCursor(0, 0);
  tinyfont.print("Shop");
  arduboy.drawLine(0, 6, 20, 6, WHITE);

  if (arduboy.justPressed(UP_BUTTON) && shop_selection > 0) {
    shop_selection--;
  }

  if (arduboy.justPressed(DOWN_BUTTON) && shop_selection < NUM_ITEMS - 1) {
    shop_selection++;
  }

  if (arduboy.justPressed(A_BUTTON) && items[shop_selection].type != START_DAY) {
    if (items[shop_selection].price > 0) {
      money -= items[shop_selection].price;
      items[shop_selection].price = 0;
      switch(items[shop_selection].type) {
        case PERMIT:
          ; // to nothing, there is logic below that will avlid re-buying the permit automatically
            // on the start of the day
          break;
        case DYNAMITE_ITEM:
          dynamite_sticks++;
          break;
      }
    }
  }

  if (arduboy.justPressed(A_BUTTON) && items[shop_selection].type == START_DAY) {
    // if they didn't buy the permit, do it automatically
    if (items[0].price) {
      money -= items[0].price;
    }

    reset_to_new_day();
    game_state = MINING;

    entity* e;
    arduboy.initRandomSeed();
    // Randomize item locations
    for (int i = 0; i < NUM_ENTITIES; i++) {
      entities[i].x = random(5, 110);
      entities[i].y = random(20, 50);
      entities[i].type = random(0, MOUSE_DIAMOND + 1);
      // currently some things are coupled together poorly, dynamite and mouse2 aren't valid objects to put
      // on the screen, but they are in the sprite sheet
      if (entities[i].type == MOUSE2 || entities[i].type == DYNAMITE) entities[i].type = random(0, MOUSE1);

      // these are only accessed when there is a mouse at a given slot in the array, so safe to unconditionally set it
      entities[i].dir = LEFT;
    }
  }

  int icon_width = 8;
  Item* item;

  for (int i = 0; i < NUM_ITEMS; i++) {
    int y_pos = 9 + SM_FONT_HEIGHT * i;
    if (i == shop_selection) {
      int select_y = y_pos + SM_FONT_HEIGHT - 2;
      arduboy.drawLine(icon_width, select_y, 25, select_y, WHITE);
    }

    item = &items[i];
    if (item->type == NOTHING) continue;

    int price = item->price;
    if (item->type == DYNAMITE_ITEM) {
      Sprites::drawPlusMask(0 - 4, y_pos - 6, sprites_plus_mask, DYNAMITE);
    }
    tinyfont.setCursor(icon_width, y_pos);
    switch(item->type) {
      case PERMIT:
        tinyfont.print("Permit: Required");
        break;
      case START_DAY:
        tinyfont.print("Start Mining");
        continue;
        break;
      case DYNAMITE_ITEM:
        if (price < 0)
          tinyfont.print("Dynamite: Sold out");
        else
          tinyfont.print("Dynamite");
        break;
    }

    tinyfont.setCursor(128 - 3 * icon_width, y_pos);
    if (price > 0)
      tinyfont.print(price);
    else
      tinyfont.print("-");
  }
}

// our main game loop, this runs once every cycle/frame.
// this is where our game logic goes.
void loop() {
  // pause render until it's time for the next frame
  if (!(arduboy.nextFrame()))
    return;

  arduboy.pollButtons();

  // first we clear our screen to black
  arduboy.clear();
  
  switch(game_state) {
    case TITLE:
      arduboy.setCursor(30, 3 * FONT_HEIGHT);
      arduboy.print("Old Miner");
      arduboy.setCursor(30, 4 * FONT_HEIGHT);
      arduboy.print("By Jason");
      if (arduboy.justPressed(A_BUTTON)) {
        game_state = STORY;
      }
      break;
    case STORY:
      arduboy.setCursor(0, 0);
      arduboy.print("There's gold in them");
      arduboy.setCursor(0, 1 * FONT_HEIGHT);
      arduboy.print("hills! And you want");

      arduboy.setCursor(0, 2 * FONT_HEIGHT);
      arduboy.print("to strike it rich.");
      arduboy.setCursor(0, 3 * FONT_HEIGHT);
      arduboy.print("You need a permit");
      arduboy.setCursor(0, 4 * FONT_HEIGHT);
      arduboy.print("each day, and can't");
      arduboy.setCursor(0, 5 * FONT_HEIGHT);
      arduboy.print("end a day with debt.");

      if (arduboy.justPressed(A_BUTTON)) {
        init_shop();
        game_state = SHOP;
      }
      break;
    case SHOP:
      shop_loop();
      break;
    case BANKRUPT:
      arduboy.setCursor(0, 0);
      arduboy.print("You ended a day with");
      arduboy.setCursor(0, 1 * FONT_HEIGHT);
      arduboy.print("debt and had to");
      arduboy.setCursor(0, 2 * FONT_HEIGHT);
      arduboy.print("declare bankruptcy!");

      arduboy.setCursor(0, 4 * FONT_HEIGHT);
      arduboy.print("Play again?");

      if (arduboy.justPressed(A_BUTTON)) {
        reset_game();
        game_state = TITLE;
      }
      break;
    case MINING:
      game_loop();
  }

  // then we finally we tell the arduboy to display what we just wrote to the display
  arduboy.display();
}