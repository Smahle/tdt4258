#include <linux/types.h>

// GPIO

#define GPIO_PA_BASE 0x40006000
#define GPIO_PB_BASE 0x40006024
#define GPIO_PC_BASE 0x40006048

#define GPIO_PA_CTRL     ((u32*)(GPIO_PA_BASE + 0x00))
#define GPIO_PA_MODEL    ((u32*)(GPIO_PA_BASE + 0x04))
#define GPIO_PA_MODEH    ((u32*)(GPIO_PA_BASE + 0x08))
#define GPIO_PA_DOUT     ((u32*)(GPIO_PA_BASE + 0x0c))
#define GPIO_PA_DOUTSET  ((u32*)(GPIO_PA_BASE + 0x10))
#define GPIO_PA_DOUTCLR  ((u32*)(GPIO_PA_BASE + 0x14))
#define GPIO_PA_DOUTTGL  ((u32*)(GPIO_PA_BASE + 0x18))
#define GPIO_PA_DIN      ((u32*)(GPIO_PA_BASE + 0x1c))
#define GPIO_PA_PINLOCKN ((u32*)(GPIO_PA_BASE + 0x20))

#define GPIO_PB_CTRL     ((u32*)(GPIO_PB_BASE + 0x00))
#define GPIO_PB_MODEL    ((u32*)(GPIO_PB_BASE + 0x04))
#define GPIO_PB_MODEH    ((u32*)(GPIO_PB_BASE + 0x08))
#define GPIO_PB_DOUT     ((u32*)(GPIO_PB_BASE + 0x0c))
#define GPIO_PB_DOUTSET  ((u32*)(GPIO_PB_BASE + 0x10))
#define GPIO_PB_DOUTCLR  ((u32*)(GPIO_PB_BASE + 0x14))
#define GPIO_PB_DOUTTGL  ((u32*)(GPIO_PB_BASE + 0x18))
#define GPIO_PB_DIN      ((u32*)(GPIO_PB_BASE + 0x1c))
#define GPIO_PB_PINLOCKN ((u32*)(GPIO_PB_BASE + 0x20))

#define GPIO_PC_CTRL     ((u32*)(GPIO_PC_BASE + 0x00))
#define GPIO_PC_MODEL    ((u32*)(GPIO_PC_BASE + 0x04))
#define GPIO_PC_MODEH    ((u32*)(GPIO_PC_BASE + 0x08))
#define GPIO_PC_DOUT     ((u32*)(GPIO_PC_BASE + 0x0c))
#define GPIO_PC_DOUTSET  ((u32*)(GPIO_PC_BASE + 0x10))
#define GPIO_PC_DOUTCLR  ((u32*)(GPIO_PC_BASE + 0x14))
#define GPIO_PC_DOUTTGL  ((u32*)(GPIO_PC_BASE + 0x18))
#define GPIO_PC_DIN      ((u32*)(GPIO_PC_BASE + 0x1c))
#define GPIO_PC_PINLOCKN ((u32*)(GPIO_PC_BASE + 0x20))

#define GPIO_EXTIPSELL ((u32*)(GPIO_PA_BASE + 0x100))
#define GPIO_EXTIPSELH ((u32*)(GPIO_PA_BASE + 0x104))
#define GPIO_EXTIRISE  ((u32*)(GPIO_PA_BASE + 0x108))
#define GPIO_EXTIFALL  ((u32*)(GPIO_PA_BASE + 0x10c))
#define GPIO_IEN       ((u32*)(GPIO_PA_BASE + 0x110))
#define GPIO_IF        ((u32*)(GPIO_PA_BASE + 0x114))
#define GPIO_IFC       ((u32*)(GPIO_PA_BASE + 0x11c))

// CMU

#define CMU_BASE2 0x400c8000

#define CMU_HFPERCLKDIV  ((u32*)(CMU_BASE2 + 0x008))
#define CMU_HFCORECLKEN0 ((u32*)(CMU_BASE2 + 0x040))
#define CMU_HFPERCLKEN0  ((u32*)(CMU_BASE2 + 0x044))
#define CMU_CMD          ((u32*)(CMU_BASE2 + 0x024))

#define CMU2_HFPERCLKEN0_DAC0   (1 << 17)
#define CMU2_HFPERCLKEN0_PRS    (1 << 15)
#define CMU2_HFPERCLKEN0_GPIO   (1 << 13)
#define CMU2_HFPERCLKEN0_TIMER1 (1 << 6)

#define CMU_HFCORECLKEN0_DMA (1 << 0)

// TIMER1

#define TIMER1_BASE 0x40010400

#define TIMER1_CMD ((u32*)(TIMER1_BASE + 0x04))
#define TIMER1_IEN ((u32*)(TIMER1_BASE + 0x0c))
#define TIMER1_IFC ((u32*)(TIMER1_BASE + 0x18))
#define TIMER1_TOP ((u32*)(TIMER1_BASE + 0x1c))
#define TIMER1_CNT ((u32*)(TIMER1_BASE + 0x24))

// NVIC

#define ISER0 ((u32*)0xe000e100)
#define ISER1 ((u32*)0xe000e104)
#define ICER0 ((u32*)0xe000e180)
#define ICER1 ((u32*)0xe000e184)
#define ISPR0 ((u32*)0xe000e200)
#define ISPR1 ((u32*)0xe000e204)
#define ICPR0 ((u32*)0xe000e280)
#define ICPR1 ((u32*)0xe000e284)
#define IABR0 ((u32*)0xe000e300)
#define IABR1 ((u32*)0xe000e304)

// IPR

#define IPR_BASE  0xe000e400

#define IPR0 ((u32*)(IPR_BASE + 0x00))
#define IPR1 ((u32*)(IPR_BASE + 0x04))
#define IPR2 ((u32*)(IPR_BASE + 0x08))
#define IPR3 ((u32*)(IPR_BASE + 0x0c))

// EMU

#define EMU_BASE2 0x400c6000

#define EMU_CTRL ((u32*)(EMU_BASE2 + 0x000))

// DAC0

#define DAC0_BASE2 0x40004000

#define DAC0_CTRL     ((u32*)(DAC0_BASE2 + 0x000))
#define DAC0_CH0CTRL  ((u32*)(DAC0_BASE2 + 0x008))
#define DAC0_CH1CTRL  ((u32*)(DAC0_BASE2 + 0x00c))
#define DAC0_IEN      ((u32*)(DAC0_BASE2 + 0x010))
#define DAC0_IF       ((u32*)(DAC0_BASE2 + 0x014))
#define DAC0_IFS      ((u32*)(DAC0_BASE2 + 0x018))
#define DAC0_IFC      ((u32*)(DAC0_BASE2 + 0x01c))
#define DAC0_CH0DATA  ((u32*)(DAC0_BASE2 + 0x020))
#define DAC0_CH1DATA  ((u32*)(DAC0_BASE2 + 0x024))
#define DAC0_COMBDATA ((u32*)(DAC0_BASE2 + 0x028))

// DMA

#define DMA_BASE 0x400c2000

#define DMA_STATUS      ((u32*)(DMA_BASE + 0x0000))
#define DMA_CONFIG      ((u32*)(DMA_BASE + 0x0004))
#define DMA_CTRLBASE    ((u32*)(DMA_BASE + 0x0008))
#define DMA_ALTCTRLBASE ((u32*)(DMA_BASE + 0x000c))
#define DMA_CHUSEBURSTS ((u32*)(DMA_BASE + 0x0018))
#define DMA_CHUSEBURSTC ((u32*)(DMA_BASE + 0x001c))
#define DMA_REQMASKC    ((u32*)(DMA_BASE + 0x0024))
#define DMA_CHENS       ((u32*)(DMA_BASE + 0x0028))
#define DMA_CHALTC      ((u32*)(DMA_BASE + 0x0034))
#define DMA_IFC         ((u32*)(DMA_BASE + 0x1008))
#define DMA_IEN         ((u32*)(DMA_BASE + 0x100c))
#define DMA_CH0_CTRL    ((u32*)(DMA_BASE + 0x1100))

// PRS

#define PRS_BASE 0x400cc000

#define PRS_CH0_CTRL ((u32*)(PRS_BASE + 0x010))

// System Control Block

#define SCR          ((u32*)0xe000ed10)
#define SYSTICK_CTRL ((u32*)0xe000e010)
#define SYSTICK_LOAD ((u32*)0xe000e014)

