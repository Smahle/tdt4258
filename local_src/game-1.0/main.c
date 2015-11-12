#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/timerfd.h>
#include <linux/fb.h>
#include <linux/input.h>

#include "main.h"
#include "game.h"

static char *fb_mem;      // Pointer to memory map.
static int   fb_mem_size; // Size of memory map.
static int   framebuffer; // File handler for framebuffer device.
static int   timer;       // File handler for periodic timer.
static int   keyboard;    // File handler for keyboard

static int   current_key; // Last key pressed. We only handle release for this one.

static int initialise_framebuffer(const char* bin_name)
{
  /* Open framebuffer device. */
  framebuffer = open("/dev/fb0", O_RDWR);
  if (framebuffer == -1) {
    fprintf(stderr, "%s: %s\n", bin_name, strerror(errno));
    return -1;
  }

  /* Get fixed information, and current configuration. */
  struct fb_var_screeninfo fb_var;    // Configurable screen information.
  struct fb_fix_screeninfo fb_fix;    // Fixed screen information.
  if (ioctl(framebuffer, FBIOGET_VSCREENINFO, &fb_var) == -1) {
    fprintf(stderr, "%s: %s\n", bin_name, strerror(errno));
    close(framebuffer);
    return -2;
  }
  if (ioctl(framebuffer, FBIOGET_FSCREENINFO, &fb_fix) == -1) {
    fprintf(stderr, "%s: %s\n", bin_name, strerror(errno));
    close(framebuffer);
    return -2;
  }

  /* Check colour resolution. We don't support unaligned sizes. */
  if (fb_var.bits_per_pixel % 8 != 0 || fb_var.bits_per_pixel > 32) {
    fprintf(stderr, "%s: %s\n", bin_name, "colour depth not supported");
    close(framebuffer);
    return -3;
  }

  /* Map open framebuffer device to memory, for easy manipulation. */
  fb_mem_size = fb_fix.smem_len;
  fb_mem = (char *)mmap(0, fb_mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, framebuffer, 0);
  if (fb_mem == MAP_FAILED) {
    fprintf(stderr, "%s: %s\n", bin_name, strerror(errno));
    close(framebuffer);
    return -4;
  }

  /* Get offset to visible area. */
  char *screen = fb_mem + ((fb_var.xoffset + fb_var.yoffset * fb_fix.line_length) * fb_var.bits_per_pixel / 8);

  /* Initialise game module. */
  printf("Got resolution %dx%d (%d bits).\n", fb_var.xres, fb_var.yres, fb_var.bits_per_pixel);
  if (init_game(screen, fb_var.xres, fb_var.yres, fb_fix.line_length, fb_var.bits_per_pixel) != 0) {
    fprintf(stderr, "%s: %s\n", bin_name, "init game failed");
    munmap(fb_mem, fb_mem_size);
    close(framebuffer);
    return -5;
  }

  return 0;
}

static int initialise_timer(const char *bin_name)
{
  /* Create timer. */
  timer = timerfd_create(CLOCK_MONOTONIC, 0);
  if (timer == -1) {
    fprintf(stderr, "%s: %s\n", bin_name, strerror(errno));
    return -6;
  }

  /* Set timer period. */
  struct timespec period = { 0, 25000000 };     // 40 frames per second.
  struct itimerspec spec = { period, period };
  if (timerfd_settime(timer, 0, &spec, NULL) != 0) {
    fprintf(stderr, "%s: %s\n", bin_name, strerror(errno));
    close(timer);
    return -7;
  }

  return 0;
}

static int initialise_keyboard(const char *bin_name)
{
  current_key = 0;

  /* Open keyboard in non-block mode, so read won't block for input. */
  keyboard = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
  if (keyboard == -1){
    fprintf(stderr, "%s: %s\n", bin_name, strerror(errno));
    return -8;
  }

  /* Turn on grabbing, so key presses don't go anywhere else. */
  if (ioctl(keyboard, EVIOCGRAB, 1) == -1) {
    fprintf(stderr, "%s: %s\n", bin_name, strerror(errno));
    close(keyboard);
    return -9;
  }

  return 0;
}

int main(int argc, const char **argv)
{
  int result = 0;   // 0 = all OK.

  /* Open framebuffer. */
  result = initialise_framebuffer(argv[0]);
  if (result != 0) {
    goto FAIL_FB;
  }

  /* Initialise input device. */
  result = initialise_keyboard(argv[0]);
  if (result != 0) {
    goto FAIL_KBD;
  }

  /* Create playing field. */
  reset_level();

  /* Create periodic timer, to limit game speed. */
  result = initialise_timer(argv[0]);
  if (result != 0) {
    goto FAIL_TIMER;
  }

  /* Run game. */
  bool run = true;
  while (run && tick()) {
    /* Wait for next timer overrun. */
    uint64_t overruns;
    result = read(timer, &overruns, sizeof(overruns));
    if (result == -1) {
      fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
      result = -10;
      break;
    }

    /* Check keyboard for new events. */
    struct input_event event;
    while (read(keyboard, &event, sizeof(event)) >= 0) {
      if (event.type == EV_KEY) {
        /* Got key input. */
        if (event.value == 0 && event.code == current_key) {
          /* Last pressed key was released. */
          set_paddle_speed(0);
          current_key = 0;

        } else if (event.value == 1) {
          /* A key was pressed. */
          bool stateful = true;
          switch (event.code) {
          case KEY_LEFT:
            set_paddle_speed(-3);
            break;

          case KEY_RIGHT:
            set_paddle_speed(3);
            break;

          case KEY_ESC:
          case KEY_Q:
            run = false;

          default:
            stateful = false;
          }

          if (stateful) {
            current_key = event.code;
          }
        }
      }
    }

    if (errno != EAGAIN) {
      /* When reading failed, it was not because no key was pressed. */
      fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
      result = -11;
      break;
    }
  }

  /* Clean up and close. */
  close(timer);
 FAIL_TIMER:
  close(keyboard);
 FAIL_KBD:
  free_game();
  munmap(fb_mem, fb_mem_size);
  close(framebuffer);
 FAIL_FB:
  return result;
}
