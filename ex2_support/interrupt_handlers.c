#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "efm32gg.h"

#define VOLUME        (10)
#define SAMPLING_FREQ (44100)
#define NOTES         (4)

static unsigned int t = 0;
static unsigned int frequency[NOTES] = {0};
static unsigned int length = 0;

/* TIMER1 interrupt handler */
void __attribute__ ((interrupt)) TIMER1_IRQHandler() 
{  
  *TIMER1_IFC = 1; /* clear interrupt */

  int note = t / length;
  if (note >= NOTES || frequency[note] == 0) {
    *TIMER1_CMD = 2;
    t = 0;
    return;
  }

  int16_t value;
  if ((((t * 2 * frequency[note]) / SAMPLING_FREQ) % 2) == 0) {
    value = VOLUME;
  } else {
    value = 0;
  }

  *DAC0_CH0DATA = value & 0xFFF;
  *DAC0_CH1DATA = value & 0xFFF;
  
  t++;
  *TIMER1_IFC = 1; /* clear interrupt */
}

void startupMelody()
{
  length = SAMPLING_FREQ / 4;
  frequency[0] = 523;
  frequency[1] = 659;
  frequency[2] = 784;
  frequency[3] = 1046;
  *TIMER1_CMD = 1;
}

void handleButton()
{
  *GPIO_IFC = *GPIO_IF; /* clear interrupt */

  uint_fast8_t buttons = (*GPIO_PC_DIN & 0xFF);
  *GPIO_PA_DOUT = buttons << 8;

  if (buttons == 0xFE) { //sw1 walltouch
    frequency[0] = 660;
    frequency[1] = 0;
    length = SAMPLING_FREQ / 8;

  } else if (buttons == 0xFD) { //sw2 failsound
    frequency[0] = 55;
    frequency[1] = 0;
    length = SAMPLING_FREQ / 2;

  } else if (buttons == 0xFB) { //sw3
    frequency[0] = 110;
    frequency[1] = 0;
    length = SAMPLING_FREQ / 2;

  } else if (buttons == 0xF7) { //sw4
    frequency[0] = 220;
    frequency[1] = 0;
    length = SAMPLING_FREQ / 2;

  } else if (buttons == 0xEF) { //sw5 blocktouch
    frequency[0] = 440;
    frequency[1] = 0;
    length = SAMPLING_FREQ / 8;

  } else if (buttons == 0xDF) { //sw6 paddletouch
    frequency[0] = 880;
    frequency[1] = 0;
    length = SAMPLING_FREQ / 8;

  } else if (buttons == 0xBF) { //sw7
    frequency[0] = 1760;
    frequency[1] = 0;
    length = SAMPLING_FREQ / 8;

  } else if (buttons == 0x7F) { //sw8
    frequency[0] = 3520;
    frequency[1] = 0;
    length = SAMPLING_FREQ / 8;

  } else {
    return;
  }

  *TIMER1_CMD = 1;
}

/* GPIO even pin interrupt handler */
void __attribute__ ((interrupt)) GPIO_EVEN_IRQHandler() 
{
  handleButton();
}


/* GPIO odd pin interrupt handler */
void __attribute__ ((interrupt)) GPIO_ODD_IRQHandler() 
{
  handleButton();
}

