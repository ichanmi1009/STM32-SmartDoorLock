#include "device_driver.h"
#include <stdbool.h>
#include <stdio.h>
#define KEY_READ (((GPIOB->IDR>>4) & (0xf)))

extern volatile bool row0_flag, row1_flag, row2_flag, row3_flag;
extern int row0_code, row1_code, row2_code, row3_code;

/*
 * [문제 상황]
 * 버튼을 한번 누르면 값 두개가 연속적으로 나오는 상황
 * 메타스테이블 현상
 * ODR 값을 쓴 타이밍에 읽어버려서 이전 값과, 바뀐 값을 둘다 출력해버리는 상황 발생
 * [해결 방안 1]
 * TIM2_DELAY(1)로 읽는 타이밍을 늦춤
 * [해결방안 1 실패]
 * DELAY를 기다리는 방식이라서 CPU가 멈춤
 * [해결 방안 2]
 * 타이밍을 늦추지 않고, 값을 쓰는 STATE(SET_ROW), 읽는 STATE(READ_ROW)를 나눔
 */

/*
 * [문제 상황]
 * for(i=0; i<4; i++)
            {
                if((col & (0x1<<i))!=0)
                {
                    row2_flag = 1;
                    row2_code = i;
                    state = KEY_RELEASE;
                }
            } 
            state = SCANROW3;
            break;
    로 if문을 만족하면 KEY_RELEASE state로 넘어가야하는데 밑의 코드때문에 SCANROW3 state로 덮어씌어버려져서 KEY_RELEASE state로 넘어가지 않음
    값도 33333처럼 연속해서 나옴
 * [해결 방안 1]
 * state = (row0_flag == 0) ? SET_ROW1 : KEY_RELEAS계;
 * 조건을 두고, 조건 만족하면 단계로 분기되도록 설계
 */

typedef enum
{
    IDLE, KEY_ISPRESS, SET_ROW0, READ_ROW0, SET_ROW1, READ_ROW1, SET_ROW2, READ_ROW2, SET_ROW3, READ_ROW3, KEY_RELEASE 
}  state_t;
volatile state_t state = IDLE; 

void Keypad_Init(void)
{
	Macro_Set_Bit(RCC->AHB1ENR, 0); // GPIOA CLK 
	Macro_Set_Bit(RCC->AHB1ENR, 1); // GPIOB CLK 
	Macro_Write_Block(GPIOA->MODER, 0xff, 0x55, 12); // KEYPAD R OUTPUT
	Macro_Write_Block(GPIOB->MODER, 0xff, 0x0, 8); // KEYPAD C INPUT
    Macro_Write_Block(GPIOB->PUPDR, 0xff, 0xaa, 8);
}

void Keypad_Pressed(void)
{
    int i;
    int col = 0;

    switch(state)
    {
        case IDLE:
            Macro_Write_Block(GPIOA->ODR, 0xf, 0xf, 6);
            //printf("1vfg%x\n",Macro_Extract_Area(GPIOB->IDR, 0xf, 4));
            if(KEY_READ!=0)
            {   
                state = KEY_ISPRESS;   
            }
            break;
            
        case KEY_ISPRESS:
            //printf(".");
            //printf("2vfg%x\n",Macro_Extract_Area(GPIOB->IDR, 0xf, 4));
            if(KEY_READ!=0)
            {
                state = SET_ROW0;
            }
            else state = IDLE;
            break;

        case SET_ROW0:
            Macro_Write_Block(GPIOA->ODR, 0xf, 0x1, 6);
            state = READ_ROW0;
            break;

        case READ_ROW0:
            // Macro_Write_Block(GPIOA->ODR, 0xf, 0x1, 6);
            // TIM2_Delay(1);
            //printf("3vfg%x\n",Macro_Extract_Area(GPIOB->IDR, 0xf, 4));
            col = KEY_READ;
            for(i=0; i<4; i++)
            {
                if((col & (0x1<<i))!=0)
                {
                    row0_flag = 1;
                    row0_code = i;
                    break;
                }
            }
            state = (row0_flag == 0) ? SET_ROW1 : KEY_RELEASE;
            break;

        case SET_ROW1:
            Macro_Write_Block(GPIOA->ODR, 0xf, 0x2, 6);
            state = READ_ROW1;
            break;

        case READ_ROW1:
            // Macro_Write_Block(GPIOA->ODR, 0xf, 0x2, 6);
            // TIM2_Delay(1);
            //printf("4vfg%x\n",Macro_Extract_Area(GPIOB->IDR, 0xf, 4));
            col = KEY_READ;
            for(i=0; i<4; i++)
            {
                if((col & (0x1<<i))!=0)
                {
                    row1_flag = 1;
                    row1_code = i;
                    break;
                }
            } 
            state = (row1_flag == 0) ? SET_ROW2 : KEY_RELEASE;
            break;

        case SET_ROW2:
            Macro_Write_Block(GPIOA->ODR, 0xf, 0x4, 6);
            state = READ_ROW2;
            break;

        case READ_ROW2:
            // Macro_Write_Block(GPIOA->ODR, 0xf, 0x4, 6);
            // TIM2_Delay(1);
            //printf("5vfg%x\n",Macro_Extract_Area(GPIOB->IDR, 0xf, 4));
            col = KEY_READ;
            for(i=0; i<4; i++)
            {
                if((col & (0x1<<i))!=0)
                {
                    row2_flag = 1;
                    row2_code = i;
                    break;
                }
            } 
            state = (row2_flag == 0) ? SET_ROW3 : KEY_RELEASE;
            break;

        case SET_ROW3:
            Macro_Write_Block(GPIOA->ODR, 0xf, 0x8, 6);
            state = READ_ROW3;
            break;
        
        case READ_ROW3:
            // Macro_Write_Block(GPIOA->ODR, 0xf, 0x8, 6);
            // TIM2_Delay(1);
            //printf("6vfg%x\n",Macro_Extract_Area(GPIOB->IDR, 0xf, 4));
            col = KEY_READ;
            // if (col == 0)
            //     state = SCANROW3;
            for(i=0; i<4; i++)
            {
                if((col & (0x1<<i))!=0)
                {
                    row3_flag = true;
                    row3_code = i;
                    state = KEY_RELEASE;
                    break;
                }
            } 
            state = (row3_flag == 0) ? IDLE : KEY_RELEASE;
            // state = KEY_RELEASE;
            break;
        
        case KEY_RELEASE:
            //printf("7vfg%x\n",KEY_READ);
            if(KEY_READ==0)
            {   
                state = IDLE;   
            }
            break;
    }
}


char Keypad_GetKey(void)
{
    static const char keymap[4][4] =
    {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
    };
    char key = 0;

    if(row0_flag)      
    { 
        row0_flag = false; 
        key = keymap[0][row0_code]; 
    }
    else if(row1_flag) 
    { 
        row1_flag = false; 
        key = keymap[1][row1_code]; 
    }
    else if(row2_flag) 
    { 
        row2_flag = false; 
        key = keymap[2][row2_code]; 
    }
    else if(row3_flag) 
    { 
        row3_flag = false; 
        key = keymap[3][row3_code]; 
    }

    return key;
}