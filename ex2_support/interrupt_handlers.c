#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "efm32gg.h"

#define VOLUME        (100.0)
#define PI            (3.14159)
#define SAMPLE_PERIOD (0.00002267573696145124)

/* TIMER1 interrupt handler */
void __attribute__ ((interrupt)) TIMER1_IRQHandler() 
{  
  /*
    TODO feed new samples to the DAC
	DAC0_CH0DATA and DAC0_CH1DATA
    remember to clear the pending interrupt by writing 1 to TIMER1_IFC
  */
  static int t = 0;

  uint_fast8_t buttons = (*GPIO_PC_DIN & 0xFF);
  *GPIO_PA_DOUT = buttons << 8;

  double angular_frequency;
  if (buttons == 0xFE){ /*check which button is being pressed*/
    angular_frequency = 27.5 * PI;
  }
  else if (buttons == 0xFD){
    angular_frequency = 55.0 * PI;
  }
  else if (buttons == 0xFB){
    angular_frequency = 110.0 * PI;
  }
  else if (buttons == 0xF7){
    angular_frequency = 220.0 * PI;
  }
  else if (buttons == 0xEF){
    angular_frequency = 440.0 * PI;
  }
  else if (buttons == 0xDF){
    angular_frequency = 880.0 * PI;
  }
  else if (buttons == 0xBF){
    angular_frequency = 1760.0 * PI;
  }
  else if (buttons == 0x7F){
    angular_frequency = 3520.0 * PI;
  }
  else{
    *TIMER1_CMD = 2;
    t = 0;
  } 
  
  double result = VOLUME * sin(angular_frequency * (t * SAMPLE_PERIOD));

  *DAC0_CH0DATA = result;
  *DAC0_CH1DATA = result;
  
  t++;
  *TIMER1_IFC = 1; /* clear interrupt */
}

/* GPIO even pin interrupt handler */
void __attribute__ ((interrupt)) GPIO_EVEN_IRQHandler() 
{
  /* TODO handle button pressed event, remember to clear pending interrupt */
  *TIMER1_CMD = 1;
  *GPIO_IFC = *GPIO_IF; /* clear interrupt */
}

/* GPIO odd pin interrupt handler */
void __attribute__ ((interrupt)) GPIO_ODD_IRQHandler() 
{
  /* TODO handle button pressed event, remember to clear pending interrupt */
  *TIMER1_CMD = 1;
  *GPIO_IFC = *GPIO_IF; /* clear interrupt */
	
}
