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
#include <ArxTypeTraits.h>

// make an instance of arduboy used for many functions
Arduboy2 arduboy;

Tinyfont tinyfont = Tinyfont(arduboy.sBuffer, Arduboy2::width(), Arduboy2::height());

const int FONT_HEIGHT = 10;
const int SM_FONT_HEIGHT = 8;
const int FPS = 30;

const int DEBUG_CONTROLS = 0;

const int SMALL_ROCK = 2;


// enum class Trigger { KEY_PRESSED };

struct TriggerOptions {
  int key_code;
};

const int KEY_PRESSED = 1;

const int LEFT_ARROW = 1;
const int RIGHT_ARROW = 2;

class Sprite {
  public:
    class Trigger {
      public:
        // TODO - do more research on scoped enums, doing this hack for now
        // https://arduino.stackexchange.com/questions/21934/how-do-i-correctly-use-enum-within-a-class
        // https://stackoverflow.com/questions/26452485/c-enum-names-overlap
        // const int KEY_PRESSED = 1;
        int trigger;
        TriggerOptions trigger_options;
        Sprite* sprite;
        typedef void(Sprite::*callbackFunc)();
        callbackFunc func;
        Trigger(int trigger_, TriggerOptions trigger_options_, Sprite* sprite_, callbackFunc func_) {
          trigger = trigger_;
          trigger_options = trigger_options_;
          sprite = sprite_;
          func = func_;
        }
    };
    Trigger* triggers;
    int width, height, x, y;
    void set_xy (int x,int y);
    void set_width_height(int width, int height);
    int area() {return width*height;}
    void leftArrow() {}
    void rightArrow() {}
};

class Sprite1 : public Sprite {
  
  public:
    // std::aligned_storage<3 * sizeof(Trigger), alignof(Trigger)>::type triggers_struct;

    Sprite1(int x_, int y_) {
      // DataBase* triggers_ = reinterpret_cast<Trigger*>(&triggers_struct);
      // new (triggers_ + 0) Trigger(KEY_PRESSED, {key: LEFT_ARROW}, Sprite1_leftArrow);
      // new (triggers_ + 1) Trigger(KEY_PRESSED, {key: RIGHT_ARROW}, Sprite1_leftArrow);
      x = x_;
      y = y_;
      Trigger triggers_[] = {
        Trigger(KEY_PRESSED, {key_code: LEFT_ARROW}, this, &Sprite::leftArrow),
        Trigger(KEY_PRESSED, {key_code: RIGHT_ARROW}, this, &Sprite::rightArrow)
      };
    }
    void leftArrow() {
      x += 5;
    }
    void rightArrow() {
      x += 5;
    }
};

const int NUM_SPRITES = 1;
Sprite sprites[NUM_SPRITES] = {
  Sprite1(50, 50)
};


/*
export default class Sprite1 extends Sprite {
  constructor(...args) {
    super(...args);

    this.costumes = [
      new Costume("costume1", "./Sprite1/costumes/costume1.svg", {
        x: 48,
        y: 50,
      }),
      new Costume("costume2", "./Sprite1/costumes/costume2.svg", {
        x: 46,
        y: 53,
      }),
    ];

    this.sounds = [new Sound("Meow", "./Sprite1/sounds/Meow.wav")];

    this.triggers = [
      new Trigger(
        Trigger.KEY_PRESSED,
        { key: "left arrow" },
        this.whenKeyLeftArrowPressed
      ),
      new Trigger(
        Trigger.KEY_PRESSED,
        { key: "right arrow" },
        this.whenKeyRightArrowPressed
      ),
    ];
  }

  *whenKeyLeftArrowPressed() {
    this.x -= 5;
  }

  *whenKeyRightArrowPressed() {
    this.x += 5;
  }
}
*/

struct entity {
  int type;
  float x;
  float y;
  // movement direction - mouse only for now
  int dir;
  // only for the thrown dynamite for now
  float thrown_dist;
};

// int detect_collision(entity* e, int x, int y) {
//   return sqrt(   (x-(e->x+HALF_SPRITE)) * (x-(e->x+HALF_SPRITE))
//                + (y-(e->y+HALF_SPRITE)) * (y-(e->y+HALF_SPRITE)))
//          <= entity_radius(e->type);
// }

const int NUM_ENTITIES = 1;
entity entities[NUM_ENTITIES] = {
  {type: SMALL_ROCK, x: 80, y: 20},
};

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

void game_loop() {

  entity* e;
  for (int i = 0; i < NUM_ENTITIES; i++) {
    e = &entities[i];
    Sprites::drawPlusMask(e->x, e->y, sprites_plus_mask, e->type);
  }

  Sprite* s;
  for (int i = 0; i < NUM_SPRITES; i++) {
    s = &sprites[i];
    Sprites::drawPlusMask(s->x, s->y, sprites_plus_mask, SMALL_ROCK);
  }

  if (arduboy.pressed(RIGHT_BUTTON)) {
    Sprite1* s;
    for (int i = 0; i < NUM_SPRITES; i++) {
      s = (Sprite1*) &sprites[i];
      s->rightArrow();
    }
  }
  if (arduboy.pressed(LEFT_BUTTON)) {
  }
  if (arduboy.pressed(DOWN_BUTTON)) {
  }

  //arduboy.drawLine(64, 4, claw_x, claw_y, WHITE);
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
  
  game_loop();

  // then we finally we tell the arduboy to display what we just wrote to the display
  arduboy.display();
}