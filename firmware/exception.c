#include "device_driver.h"
#include <stdio.h>
#include <stdbool.h>
extern volatile bool ms1_flag;
// extern volatile int TIM4_Expired;
// extern volatile int r_flag; // 보통 flag 변수에 volatile 선언
// extern int r; // irq에서 값 변화를 안한다.
// dma변수, 인터럽트 flag, 인터럽트 내부의 변수들, 메모리 mapped io(__IO), 멀티 프로세스(RTOS 운영체제)
// CPU에서 변화 감지를 못하는 상황일 경우에
// extern volatile int Uart_Data_In;
// extern volatile unsigned char Uart_Data;
// extern volatile int Key_Pressed;
extern volatile int Key1_flag;
// extern volatile bool count_flag;
extern volatile bool pir_detected;

void _Invalid_ISR(void)
{
	unsigned int r = Macro_Extract_Area(SCB->ICSR, 0x1ff, 0);
	printf("\nInvalid_Exception: %d!\n", r);
	printf("Invalid_ISR: %d!\n", r - 16);
	for(;;);
}


void EXTI15_10_IRQHandler(void)
{
	// Key_Pressed = 1;
	// Key1_flag = 1;
	EXTI->PR = 0x1 << 13;
	NVIC_ClearPendingIRQ(40);
}

void EXTI2_IRQHandler(void)
{
	pir_detected = true;
	printf("PIR triggered!\n");   // 임시로 추가
	EXTI->PR = 0x1 << 2;
	NVIC_ClearPendingIRQ(8);
}

void USART2_IRQHandler(void)
{
	// Uart_Data = (unsigned char)USART2->DR;
	// Uart_Data_In = 1;
	NVIC_ClearPendingIRQ(38);
}


void TIM4_IRQHandler(void)
{
	// static int cnt=0;
	// cnt++;
	// if(cnt==10){
	// 	cnt=0;
	// 	count_flag = true;
	// }
	// int r = rand() % 8000 + 500;
	static int cnt = 0;
	cnt++;
	if(cnt==1){
		cnt=0;
		ms1_flag = true;
	}
	// TIM4 Interrupt Pending Clear
	TIM4->SR = TIM_SR_UIF_Pos;
	// NVIC Pending Clear
	NVIC_ClearPendingIRQ((IRQn_Type)TIM4_IRQn);
	
	// TIM4_Expired = 1;
}





