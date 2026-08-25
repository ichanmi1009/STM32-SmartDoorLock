#include "device_driver.h"
#include <stdio.h>

void Pir_Init(void)
{    
    Macro_Set_Bit(RCC->AHB1ENR, 3); // GPIOD CLK 
	Macro_Write_Block(GPIOD->MODER, 0x3, 0x0, 4);

    Macro_Set_Bit(RCC->APB2ENR, 14);
    Macro_Write_Block(SYSCFG->EXTICR[0], 0xf, 0x3, 8);
    EXTI->RTSR |= 0x1<<2;
    EXTI->PR = 0x1<<2;
    EXTI->IMR |= 0x1<<2;

    NVIC_ClearPendingIRQ((IRQn_Type)8);
    NVIC_EnableIRQ((IRQn_Type)8);
}