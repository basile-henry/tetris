#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <timer.h>
#include <EasyBuzzer.h>
#include <EasyButton.h>
#include "characters.h"
#include "melody.h"
#include "shapes.h"

////////////////////////////////////////////////////////////////////////////////
// IO
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define POT           A3
#define BTN0           2
#define BTN1           3
#define BTN2           4
#define BUZZ           5

////////////////////////////////////////////////////////////////////////////////
// SETUP
Timer<5> timer;
EasyButton btn0(BTN0), btn1(BTN1), btn2(BTN2);


void setup() {
  // Serial.begin(9600);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    for(;;); // Don't proceed, loop forever
  }

  randomSeed(analogRead(POT));

  EasyBuzzer.setPin(BUZZ);

  btn0.begin();
  btn1.begin();
  btn2.begin();

  btn0.onPressed([](){ try_rotate(); });
  // btn1.onPressed([](){ fast_fall(); });
  btn2.onPressed([](){
    if (!btn1.isPressed()) game_setup();
    else drop();
    });

  play_note();
  game_setup();
  timer.every(20, game_update);
  block_update();
}

void loop() {
  EasyBuzzer.update();
  timer.tick();
  btn0.read();
  btn1.read();
  btn2.read();
}

////////////////////////////////////////////////////////////////////////////////
// AUDIO
int noteDuration = 100;
int music_ctr = 0;

void play_note() {
  int pitch = melody[music_ctr][0];
  int duration = melody[music_ctr][1] * noteDuration;
  music_ctr = (music_ctr + 1) % MELODY_LENGTH;

  if (pitch == 0) {
    timer.in(duration, play_note);
  } else {
    EasyBuzzer.singleBeep(pitch, duration, play_note);
  }
}

////////////////////////////////////////////////////////////////////////////////
// GAME LOGIC
#define HEIGHT 42
#define WIDTH  10

int x, y;
int cur_shape[4][4];
uint16_t buffer[HEIGHT];
int points;

#define buffer_get(x, y) ((buffer[y] & (1 << x)) > 0)
void buffer_set(int x, int y) { buffer[y] |= (1 << x); }

void game_setup() {
  x = WIDTH / 2;
  y = 0;
  points = 0;
  get_new_shape();
  memset(buffer, 0, HEIGHT * sizeof(uint16_t));
}

void game_update() {
  position_update();
  view();
}

void position_update() {
  int new_x = (analogRead(POT) >> 6) - 3;
  int tmp_x;

  while (x != new_x) {
    tmp_x = x < new_x ? x + 1: x - 1;
    if (is_colliding(tmp_x, y, cur_shape)) break;
    x = tmp_x;
  }
}

void block_update() {
  if (is_colliding(x, y + 1, cur_shape)) {
    // Copy shape to buffer
    for (int i=0; i<4; i++) {
      for (int j=0; j<4; j++) {
        if (cur_shape[i][j]) buffer_set(x+i, y+j);
      }
    }

    int skip_copy = 0;
    int i = HEIGHT-1;

    while (i >= 0) {
      if (buffer[i] == 0x03FF) {
        skip_copy++;
        points++;
      } else i--;

      if (i - skip_copy >= 0) buffer[i] = buffer[i - skip_copy];
    }

    get_new_shape();
    x = WIDTH / 2;
    y = 0;
  } else {
    y++;
  }

  int level = points / 10;
  noteDuration = max(30, 100 - 5 * level);
  int speed = max(30, 500 - 25 * level);
  int next_block_update = btn1.isPressed() ? speed : 30;
  timer.in(next_block_update, block_update);
}

void try_rotate() {
  int tmp_shape[4][4];
  memcpy(tmp_shape, cur_shape, 4 * 4 * sizeof(int));
  rotate(tmp_shape);
  if (!is_colliding(x, y, tmp_shape)) {
    rotate(cur_shape);
  }
}

void drop() {
  while (!is_colliding(x, y + 1, cur_shape)) y++;
}

bool is_colliding(int x, int y, int shape[4][4]) {
  for (int i=0; i<4; i++) {
    int x_ = x + i;
    for (int j=0; j<4; j++) {
      int y_ = y + j;

      if ((shape[i][j] != 0) &&
          (x_ < 0 || x_ >= WIDTH || y_ < 0 || y_ >= HEIGHT || buffer_get(x_, y_)))
        return true;
    }
  }
  return false;
}

void get_new_shape() {
  memcpy(cur_shape, shapes[random(NUM_SHAPES)], 4*4*sizeof(int));
  for (int r=random(4); r>=0; r--) { rotate(cur_shape); }
}

////////////////////////////////////////////////////////////////////////////////
// VIEW
#define SQUARE_SIZE     3

void draw_rect(int x, int y, uint16_t color) {
  for (int i=0; i<3; i++) {
    for (int j=0; j<3; j++) {
      if (i == 1 && j == 1) continue;

      int i_ = SQUARE_SIZE*x + i + 1;
      int j_ = SQUARE_SIZE*y + j + 1;

      // ! x and y swapped
      display.drawPixel(j_, i_, color);
    }
  }
}

void view() {
  display.clearDisplay();

  draw_rect(4, 8, 1);

  // Border
  display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 1);

  // Draw buffer
  for (int i=0; i<HEIGHT; ++i) {
    for (int j=0; j<WIDTH; ++j) {
      draw_rect(j, i, buffer_get(j, i));
    }
  }

  for (int i=0; i<4; ++i) {
    for (int j=0; j<4; ++j) {
      if (cur_shape[i][j]) draw_rect(x + i, y + j, 1);
    }
  }

  display.display();
}
