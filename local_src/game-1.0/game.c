#include <stdint.h>
#include <stdlib.h>

#include "game.h"

static char *screen;                    // Memory mapped screen space.
static int   xres, yres;                // Screen resolution.
static int   stride;                    // Bytes per display line.
static int   bits;                      // Colour resolution.

static int   boxes_x, boxes_y;          // Number of boxes.
static bool *boxes = NULL;              // Map of non-destroyed boxes.
static int   box_count;                 // Number of non-destroyed boxes.
static int   box_width, box_height;     // Size of a single box.

static int paddle_width, paddle_height; // Paddle size.
static int paddle_pos_x, paddle_pos_y;  // Paddle centre.
static int paddle_speed;                // Paddle player control.
static int ball_halfwidth;              // Ball radius. Note: the ball is actually a square.
static int ball_pos_x, ball_pos_y;      // Ball centre.
static int ball_speed_x, ball_speed_y;  // Ball delta per frame.

static int background = 0x000003;       // Background colour.

static int random_colour()
{
  int r = rand() & 0xFF;
  int g = rand() & 0xFF;
  int b = rand() & 0xFF;

  while (r+g+b < 0x50) {
    r *= 2;
    g *= 2;
    b *= 2;
  }

  return (r << 16) | (g << 8) | b;
}

static int value(int colour, int bits, int component)
{
  int value = 0;
  switch (component) {
  case 0: value = colour & 0xFF0000; break; // Red
  case 1: value = colour & 0x00FF00; break; // Green
  case 2: value = colour & 0x0000FF; break; // Blue
  }

  return value >> (8-bits);
}

static void set_pixel(int x, int y, int colour) {
  int byte = (y * stride) + ((x * bits) / 8);

  switch (bits) {
  case 8:
    {
      screen[byte] = (value(colour, 2, 0) << 6 |    // Red (2 bits).
                      value(colour, 3, 1) << 3 |    // Green (3 bits).
                      value(colour, 3, 2));         // Blue (3 bits).
    } break;

  case 16:
    {
      *(uint16_t*)(screen + byte) = (value(colour, 5, 0) << 11 | // Red (5 bits).
                                     value(colour, 6, 1) << 5  | // Green (6 bits).
                                     value(colour, 5, 2));       // Blue (5 bits).
    } break;

  case 24:
    {
      screen[byte]   = (colour & 0xFF0000) >> 16;   // Red.
      screen[byte+1] = (colour & 0x00FF00) >> 8;    // Green.
      screen[byte+2] = (colour & 0x0000FF);         // Blue.
    } break;

  case 32:
    {
      *(uint32_t*)(screen + byte) = colour;         // Direct copy.
    } break;
  }
}

static void draw_box(int x, int y, int width, int height, int colour)
{
  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      set_pixel(i + x, j + y, colour);
    }
  }
}

static void draw_paddle(bool clear)
{
  int colour = clear ? background : (background ^ 0xFFFFFF) | 0xF0;
  draw_box(paddle_pos_x - paddle_width/2, paddle_pos_y - paddle_height/2,
           paddle_width, paddle_height, colour);
}

static void draw_ball(bool clear)
{
  int colour = clear ? background : (background ^ 0xFFFFFF) | 0xF000;
  int r_square = ball_halfwidth * ball_halfwidth;

  for (int y = -ball_halfwidth; y <= ball_halfwidth; y++) {
    for (int x = -ball_halfwidth; x <= ball_halfwidth; x++) {
      if (x*x + y*y <= r_square) {
        set_pixel(ball_pos_x + x, ball_pos_y + y, colour);
      }
    }
  }

}

static void draw_boxes(bool restore)
{
  for (int y = 0; y < boxes_y; y++) {
    for (int x = 0; x < boxes_x; x++) {

      if (restore || boxes[y*boxes_x + x]) {
        int colour = random_colour();
        draw_box(x * box_width, y * box_height, box_width, box_height, colour);
      }

      if (restore) {
        boxes[y*boxes_x + x] = true;
      }
    }
  }

  if (restore) {
    box_count = boxes_x * boxes_y;
  }
}

static bool is_box(int x, int y) {
  if (x < 0)        return false;
  if (x >= boxes_x) return false;
  if (y < 0)        return false;
  if (y >= boxes_y) return false;
  return boxes[y*boxes_x + x];
}

static void remove_box(int x, int y) {
  boxes[y*boxes_x + x] = false;
  draw_box(x * box_width, y * box_height, box_width, box_height, background);

  box_count--;
}

static bool box_box_intersect(int a_left, int a_right, int a_top, int a_bottom,
                              int b_left, int b_right, int b_top, int b_bottom)
{
  if (a_right < b_left) return false;
  if (b_right < a_left) return false;
  if (a_bottom < b_top) return false;
  if (b_bottom < a_top) return false;
  return true;
}

static bool resolve_paddle_collision()
{
  if (box_box_intersect(ball_pos_x - ball_halfwidth, ball_pos_x + ball_halfwidth,
                        ball_pos_y - ball_halfwidth, ball_pos_y + ball_halfwidth,
                        paddle_pos_x - paddle_width/2, paddle_pos_x + paddle_width/2,
                        paddle_pos_y - paddle_height/2, paddle_pos_y + paddle_height/2)) {
    /* Set speed depending on relative ball-to-paddle position. */
    ball_speed_x = 4 * (ball_pos_x - paddle_pos_x) / paddle_width;
    ball_speed_y = 10 - ball_speed_x;

    if (ball_speed_y >= -1) {
      ball_speed_y = -2;
    }
    return true;
  }

  return false;
}

static bool resolve_boxes_collision()
{
  bool result = false;

  /* Calculate ball box centre edges. */
  int ball_left   = ball_pos_x - ball_halfwidth;
  int ball_right  = ball_pos_x + ball_halfwidth;
  int ball_top    = ball_pos_y - ball_halfwidth;
  int ball_bottom = ball_pos_y + ball_halfwidth;

  /* Calculate nearest possible collision. */
  int box_x = (2 * ball_pos_x - box_width)  / (2 * box_width);
  int box_y = (2 * ball_pos_y - box_height) / (2 * box_height);
  int box_left   = box_x * box_width;
  int box_right  = box_left + box_width;
  int box_top    = box_y * box_height;
  int box_bottom = box_top + box_height;

  /* Check right and left collisions. */
  if (ball_speed_x > 0) {
    if (is_box(box_x + 1, box_y) && ball_right >= box_right) {
      remove_box(box_x + 1, box_y);
      ball_speed_x *= -1;
      result = true;
    }
  } else if (ball_speed_x < 0) {
    if (is_box(box_x - 1, box_y) && ball_left < box_left) {
      remove_box(box_x - 1, box_y);
      ball_speed_x *= -1;
      result = true;
    }
  }

  /* Check top and bottom collisions. */
  if (ball_speed_y > 0) {
    if (is_box(box_x, box_y + 1) && ball_top >= box_top) {
      remove_box(box_x, box_y + 1);
      ball_speed_y *= -1;
      result = true;
    }
  } else if (ball_speed_y < 0) {
    if (is_box(box_x, box_y - 1) && ball_bottom < box_bottom) {
      remove_box(box_x, box_y - 1);
      ball_speed_y *= -1;
      result = true;
    }
  }

  return result;
}

static bool resolve_screen_collision(bool *dead)
{
  bool result = false;

  if (ball_pos_x - ball_halfwidth < 0) {
    if (ball_speed_x < 0) ball_speed_x *= -1;
    result = true;

  } else if (ball_pos_x + ball_halfwidth >= xres) {
    if (ball_speed_x > 0) ball_speed_x *= -1;
    result = true;
  }

  if (ball_pos_y - ball_halfwidth < 0) {
    if (ball_speed_y < 0) ball_speed_y *= -1;
    result = true;

  } else if (ball_pos_y + ball_halfwidth >= yres) {
    if (ball_speed_y > 0) ball_speed_y *= -1;
    result = true;
    *dead = true;
  }

  return result;
}

int init_game(char* mmap, int x_resolution, int y_resolution, int line_length, int bits_per_pixel)
{
  if (boxes != NULL) {
    free_game();
  }

  screen = mmap;
  xres = x_resolution;
  yres = y_resolution;
  bits = bits_per_pixel;
  stride = line_length;
  paddle_speed = 0;

  boxes_x = 2;
  boxes_y = 2;
  box_width = xres >> 1;
  box_height = yres >> 2;

  while (box_width > PADDLE_WIDTH || box_height > (PADDLE_WIDTH / 2)) {
    boxes_x <<= 1;
    boxes_y <<= 1;
    box_width >>= 1;
    box_height >>= 1;
  }

  boxes = malloc(boxes_x * boxes_y * sizeof(bool));
  if (boxes == NULL) {
    return -1;
  }

  srand(123);

  /* Some things that change when you win. */
  background = 0;
  paddle_width = PADDLE_WIDTH;

  return 0;
}

void free_game()
{
  if (boxes != NULL) {
    free(boxes);
  }
}

void reset_level()
{
  /* Blank screen. */
  for (int y = 0; y < yres ; y++) {
    for (int x = 0; x < xres ; x++) {
      set_pixel(x, y, background);
    }
  }

  /* Reset sizes and positions. */
  paddle_height = PADDLE_HEIGHT;
  paddle_pos_x  = xres / 2;
  paddle_pos_y  = yres - yres / 10;

  ball_halfwidth = BALL_RADIUS;
  ball_pos_y   = boxes_y * box_height + 2 * ball_halfwidth;
  ball_pos_x   = paddle_pos_x - (paddle_pos_y - ball_pos_y);
  if (ball_pos_x < 2 * ball_halfwidth) {
    ball_pos_x = 2 * ball_halfwidth;
  }
  ball_speed_x = BALL_RADIUS / 2;
  ball_speed_y = BALL_RADIUS / 2;

  /* Draw initial paddle and ball. */
  draw_paddle(false);
  draw_ball(false);

  /* Draw all boxes, marking them as restored. */
  draw_boxes(true);
}

void set_paddle_speed(int speed)
{
  paddle_speed = speed;
}

bool tick()
{
  /* Paddle moves first. */
  if (paddle_speed != 0) {
    draw_paddle(true);  // Clear old position.

    /* Move and limit to edges. */
    paddle_pos_x += paddle_speed;
    if ((paddle_pos_x + paddle_width/2) >= xres) {
      paddle_pos_x = xres - paddle_width/2;
    } else if ((paddle_pos_x - paddle_width/2) < 0) {
      paddle_pos_x = paddle_width/2;
    }

    draw_paddle(false);  // Redraw at new position.
  }

  /* Remove ball from current position. */
  draw_ball(true);

  /* Moving the paddle may have caused a collision. Check before moving ball. */
  resolve_paddle_collision();

  /* Move ball until there are no collisions. */
  bool dead = false;
  int stuck_counter = 0;
  do {
    ball_pos_x += ball_speed_x;
    ball_pos_y += ball_speed_y;
    if (++stuck_counter > 40) {
      return false;
    }
  } while (resolve_screen_collision(&dead)
           || resolve_boxes_collision()
           || resolve_paddle_collision());

  /* Check win condition. */
  if (box_count <= 0) {
    background <<= 2;
    background |= 3;
    paddle_width = (paddle_width * 9) / 10;
    if (paddle_width < PADDLE_MIN) {
      paddle_width = PADDLE_MIN;
    }
    reset_level();
    dead = false;

  } else {
    /* Draw ball at final position. */
    draw_ball(false);
  }

  return !dead;
}
