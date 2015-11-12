#ifndef H_GAME
#define H_GAME

#include <stdbool.h>

#define PADDLE_WIDTH  (64)
#define PADDLE_HEIGHT (12)
#define PADDLE_MIN    (32)
#define BALL_RADIUS   (4)

int init_game(char* mmap, int x_resolution, int y_resolution, int line_length, int bits_per_pixel);
void free_game();

void reset_level();
void set_paddle_speed(int speed);
bool tick();

#endif
