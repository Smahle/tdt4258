#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "efm32gg.h"

/* TIMER1 interrupt handler */
void __attribute__ ((interrupt)) TIMER1_IRQHandler() 
{  
  /*
    TODO feed new samples to the DAC
	DAC0_CH0DATA and DAC0_CH1DATA
    remember to clear the pending interrupt by writing 1 to TIMER1_IFC
  */
  //(*GPIO_PA_DOUT)++;	
  static int t = 0;
 
  if (*GPIO_PC_DIN == 0xFFFFFFFE){ /*check which button is being pressed*/
	*DAC0_CH0DATA = 100*sin(t/4*3.14);
	}
  else if (*GPIO_PC_DIN == 0xFFFFFFFD){
	*DAC0_CH0DATA = 100*sin(t/8*3.14);
	}
  else if (*GPIO_PC_DIN == 0xFFFFFFFB){
	*DAC0_CH0DATA = 100*sin(t/16*3.14);
	}
  else if (*GPIO_PC_DIN == 0xFFFFFFF7){
	*DAC0_CH0DATA = 100*sin(t/32*3.14);
	}
  else if (*GPIO_PC_DIN == 0xFFFFFFEF){
	*DAC0_CH0DATA = 100*sin(t/64*3.14);
	}
  else if (*GPIO_PC_DIN == 0xFFFFFFDF){
	*DAC0_CH0DATA = 100*sin(t/128*3.14);
	}
  else if (*GPIO_PC_DIN == 0xFFFFFFBF){
	*DAC0_CH0DATA = 100*sin(t/4*3.14);
	}
  else if (*GPIO_PC_DIN == 0xFFFFFF7F){
	*DAC0_CH0DATA = 100*sin(t/4*3.14);
	}
  else{
	*TIMER1_CMD = 0;
	} 
  
	
  
  //*DAC0_CH0DATA = 
  //*DAC0_CH1DATA =
  t++;
  *TIMER1_IFC = 1; /* clear interrupt */
}

/* GPIO even pin interrupt handler */
void __attribute__ ((interrupt)) GPIO_EVEN_IRQHandler() 
{
    /* TODO handle button pressed event, remember to clear pending interrupt */
	//*TIMER1_CMD = 1;
	*GPIO_PA_DOUT = 0xFF00;
	*GPIO_IFC = *GPIO_IF; /* clear interrupt */
	
}

/* GPIO odd pin interrupt handler */
void __attribute__ ((interrupt)) GPIO_ODD_IRQHandler() 
{
    /* TODO handle button pressed event, remember to clear pending interrupt */
	//*TIMER1_CMD = 1;
	*GPIO_PA_DOUT = 0xFF00;
	*GPIO_IFC = *GPIO_IF; /* clear interrupt */
	
}
