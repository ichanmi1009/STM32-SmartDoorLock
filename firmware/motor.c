#include "device_driver.h"
#include <stdbool.h>
#include <stdio.h>

#define TIM5_TICK          (1)                     // usec
#define TIME5_PLS_OF_1ms    (1000/TIM5_TICK)         // 1ms당 틱 수

void Motor_Init(void)
{
	TIM5_PWM_Init();
    
    Macro_Set_Bit(RCC->AHB1ENR, 0); // GPIOA CLK 
	Macro_Write_Block(GPIOA->MODER, 0x3, 0x2, 0);
    Macro_Write_Block(GPIOA->AFR[0], 0xf, 0x2, 0);
}

void Motor_Drive(int value)
{
    if(value < 0)   value = 0;
    if(value > 180) value = 180; // 1000~2000사이의 범위내의 값
    TIM5->CCR1 = TIME5_PLS_OF_1ms + (TIME5_PLS_OF_1ms * value / 180);
}

void Motor_Stop(void)
{
    Motor_Drive(90);
}

void door_open(void)
{
    Motor_Drive(180);
    TIM2_Delay(410);
    Motor_Stop();
    TIM2_Delay(1000);
    Motor_Drive(0);    
    TIM2_Delay(380);
    Motor_Stop();
}
