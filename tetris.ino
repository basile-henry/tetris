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
#define HEIGHT 30
#define WIDTH  10

int x, y;
int cur_shape[4][4];
int next_shape[4][4];
uint16_t buffer[HEIGHT];
int points;

#define buffer_get(x, y) ((buffer[y] & (1 << x)) > 0)
void buffer_set(int x, int y) { buffer[y] |= (1 << x); }

void game_setup() {
  x = WIDTH / 2;
  y = 0;
  points = 0;
  get_new_shape();
  get_new_shape();
  memset(buffer, 0, HEIGHT * sizeof(uint16_t));
}

void game_update() {
  position_update();
  view();
}

void position_update() {
  int new_x = 13 - (analogRead(POT) >> 6);
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
  memcpy(cur_shape, next_shape, 4*4*sizeof(int));
  memcpy(next_shape, shapes[random(NUM_SHAPES)], 4*4*sizeof(int));
  for (int r=random(4); r>=0; r--) { rotate(cur_shape); }
}

////////////////////////////////////////////////////////////////////////////////
// VIEW
#define SQUARE_SIZE     3
#define GAME_X_OFF      1
#define GAME_Y_OFF     37
#define PREVIEW_X_OFF   1
#define PREVIEW_Y_OFF  22

void draw_rect(int x, int y, int width, int height) {
  display.drawRect(y, SCREEN_HEIGHT - width - x, height, width, 1);
}

void draw_block(int x_off, int y_off, int x, int y, uint16_t color) {
  for (int i=0; i<3; i++) {
    for (int j=0; j<3; j++) {
      if (i == 1 && j == 1) continue;

      int i_ = y_off + SQUARE_SIZE*y + i;
      int j_ = x_off + SQUARE_SIZE*x + j;

      display.drawPixel(i_, SCREEN_HEIGHT - 1 - j_, color);
    }
  }
}

void draw_shape(int x_off, int y_off, int x, int y, int shape[4][4]) {
  for (int i=0; i<4; ++i) {
    for (int j=0; j<4; ++j) {
      if (shape[j][i]) draw_block(x_off, y_off, x + j, y + i, 1);
    }
  }
}

void view() {
  display.clearDisplay();

  // Header

  // Preview
  draw_rect(0, PREVIEW_Y_OFF - 1, 2 + 4 * SQUARE_SIZE, 2 + 4 * SQUARE_SIZE);
  draw_shape(PREVIEW_X_OFF, PREVIEW_Y_OFF, 0, 0, next_shape);

  // Game border
  draw_rect(0, GAME_Y_OFF - 1, SCREEN_HEIGHT, HEIGHT * SQUARE_SIZE + 2);

  // Draw buffer
  for (int i=0; i<HEIGHT; ++i) {
    for (int j=0; j<WIDTH; ++j) {
      draw_block(GAME_X_OFF, GAME_Y_OFF, j, i, buffer_get(j, i));
    }
  }

  draw_shape(GAME_X_OFF, GAME_Y_OFF, x, y, cur_shape);

  display.display();
}
