#include "device_driver.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#define BASE  (500) //msec
extern void TIM4_IRQHandler(void);
extern void Keypad_Pressed(void);
extern char Keypad_GetKey(void);
extern void Motor_Drive(int value);
extern void door_open(void);
static bool lcd_shown = false;
volatile bool row0_flag, row1_flag, row2_flag, row3_flag;
int row0_code, row1_code, row2_code, row3_code;
volatile bool ms1_flag;
volatile bool pir_detected = false;
static void Sys_Init(int baud) 
{
    SCB->CPACR |= (0x3 << 10*2)|(0x3 << 11*2); 
    Clock_Init();
    Uart2_Init(baud);
    setvbuf(stdout, NULL, _IONBF, 0);
    LED_Init();
    Keypad_Init();
    Motor_Init();
    Pir_Init();
    LCD_Init();
}
static void Buzzer_Beep(unsigned char tone, int duration)
{
    const static unsigned short tone_value[] = {261,277,293,311,329,349,369,391,415,440,466,493,523,554,587,622,659,698,739,783,830,880,932,987};
    TIM3_Out_Freq_Generation(tone_value[tone]);
    TIM2_Delay(duration);
    TIM3_Out_Stop();
}
static void LCD_ClearLine2(void)
{
    LCD_Send(0xC0, 0);
    LCD_Print("                "); 
    LCD_Send(0xC0, 0);            
}
void Main(void)
{
    Sys_Init(115200);
    printf("\nEXTI(Key) IRQ\n");
    Key_ISR_Enable(1);
    TIM4_Repeat_Interrupt_Enable(1,5);
    char password[4] = {'1','2','3','4'};
    char buf[5];
    static int idx = 0;
    static bool change_mode = false;  
    static bool allow_change = false;  
    int i;
    enum key{C1, C1_, D1, D1_, E1, F1, F1_, G1, G1_, A1, A1_, B1, C2, C2_, D2, D2_, E2, F2, F2_, G2, G2_, A2, A2_, B2};
    enum note{N16=BASE/4, N8=BASE/2, N4=BASE, N2=BASE*2, N1=BASE*4};
    const int song_open[][2] = {{C1, N8}, {E1, N8}, {G1, N8}, {C2, N8}, {E2, N4}, {G2, N4}, {C2, N2}};
    const int song_close[][2] = {{G1, N8}, {E1, N8}, {C1, N4},{G1, N8}, {C1, N4}};
    TIM3_Out_Init();
    for(;;)
    {
        if(pir_detected)
        {
            if(!lcd_shown)
            {
                LCD_Send(0x01, 0);         
                LCD_Print("Enter Password ");
                LCD_Send(0xC0, 0);         
                lcd_shown = true;
                allow_change = false;   // 새 세션 시작 시 초기화
                change_mode = false;
            }
            if(ms1_flag)
            {
                ms1_flag = false;
                Keypad_Pressed();
            }
            char key = Keypad_GetKey();
            if(key != 0)
            {
                if(key == '#' && idx == 0 && allow_change)
                {
                    change_mode = true;
                    allow_change = false;
                    idx = 0;
                    LCD_ClearLine2();
                    LCD_Print("New PW:");
                }
                else if(key == 'D')
                {
                    idx = 0;
                    LCD_ClearLine2();
                }
                else if(idx < 4)
                {
                    buf[idx] = key;
                    idx++;
                    printf("*");
                    LCD_Print("*");
                }
                else if(key == '*')
                {
                    idx = 0;
                    if(change_mode)
                    {
                        memcpy(password, buf, 4);
                        change_mode = false;
                        LCD_ClearLine2();
                        LCD_Print("PW Changed");
                        pir_detected = false;
                        lcd_shown = false;
                    }
                    else if(memcmp(buf, password, 4) == 0)
                    {
                        printf("\n비밀번호 pass\n");
                        LCD_ClearLine2();
                        LCD_Print("Open!/#:chgPW");   // 문 열리는 동안 표시
                        door_open();
                        allow_change = true;           // 성공 직후에만 # 허용
                        for(i = 0; i < (sizeof(song_open)/sizeof(song_open[0])); i++)
                            Buzzer_Beep(song_open[i][0], song_open[i][1]);
                        LCD_ClearLine2();
                        LCD_Print("Close!");            // 동작 다 끝난 뒤 표시
                    }
                    else
                    {
                        printf("\n비밀번호가 틀렸습니다. 다시 입력해 주세요.\n");
                        LCD_ClearLine2();
                        LCD_Print("Try Again");
                        for(i = 0; i < (sizeof(song_close)/sizeof(song_close[0])); i++)
                            Buzzer_Beep(song_close[i][0], song_close[i][1]);
                        pir_detected = false;
                        lcd_shown = false;
                        allow_change = false;   // 실패 시 # 권한도 같이 끔
                    }
                }
            }
        }
    }  
}

// #include "device_driver.h"
// #include <stdio.h>
// #include <stdbool.h>
// #include <stdlib.h>
// #include <string.h>

// #define BASE  (500) //msec

// extern void TIM4_IRQHandler(void);
// extern void Keypad_Pressed(void);
// extern void Motor_Drive(int value);
// extern void door_open(void);

// static bool lcd_shown = false;

// volatile bool row0_flag, row1_flag, row2_flag, row3_flag;
// int row0_code, row1_code, row2_code, row3_code;
// volatile bool ms1_flag;
// volatile bool pir_detected = false;

// static void Sys_Init(int baud) 
// {
//     SCB->CPACR |= (0x3 << 10*2)|(0x3 << 11*2); 
//     Clock_Init();
//     Uart2_Init(baud);
//     setvbuf(stdout, NULL, _IONBF, 0);
//     LED_Init();
//     Keypad_Init();
//     Motor_Init();
//     Pir_Init();
//     LCD_Init();
// }

// static void Buzzer_Beep(unsigned char tone, int duration)
// {
//     const static unsigned short tone_value[] = {261,277,293,311,329,349,369,391,415,440,466,493,523,554,587,622,659,698,739,783,830,880,932,987};
//     TIM3_Out_Freq_Generation(tone_value[tone]);
//     TIM2_Delay(duration);
//     TIM3_Out_Stop();
// }

// static void LCD_ClearLine2(void)
// {
//     LCD_Send(0xC0, 0);
//     LCD_Print("                ");  // 공백 16칸으로 이전 내용 지우기
//     LCD_Send(0xC0, 0);              // 다시 줄 맨 앞으로 커서 이동
// }

// void Main(void)
// {
//     Sys_Init(115200);
//     printf("\nEXTI(Key) IRQ\n");
//     Key_ISR_Enable(1);
//     TIM4_Repeat_Interrupt_Enable(1,5);

//     char password[4] = {'1','2','3','4'};
//     char keypad[4][4] =
//     { 
//       {'1', '2', '3', 'A'},
//       {'4', '5', '6', 'B'}, 
//       {'7', '8', '9', 'C'}, 
//       {'#', '0', '*', 'D'}
//     };
//     char buf[5];
//     char new_password[4];
//     static int idx = 0;
//     static int cnt = 0;
//     int i;

//     enum key{C1, C1_, D1, D1_, E1, F1, F1_, G1, G1_, A1, A1_, B1, C2, C2_, D2, D2_, E2, F2, F2_, G2, G2_, A2, A2_, B2};
//     enum note{N16=BASE/4, N8=BASE/2, N4=BASE, N2=BASE*2, N1=BASE*4};
//     const int song_open[][2] = {{C1, N8}, {E1, N8}, {G1, N8}, {C2, N8}, {E2, N4}, {G2, N4}, {C2, N2}};
//     const int song_close[][2] = {{G1, N8}, {E1, N8}, {C1, N4},{G1, N8}, {C1, N4}};
//     const char * note_name[] = {"C1", "C1#", "D1", "D1#", "E1", "F1", "F1#", "G1", "G1#", "A1", "A1#", "B1", "C2", "C2#", "D2", "D2#", "E2", "F2", "F2#", "G2", "G2#", "A2", "A2#", "B2"};

//     TIM3_Out_Init();

//     for(;;)
//     {
//         if(pir_detected)
//         {
//             if(!lcd_shown)
//             {
//                 LCD_Send(0x01, 0);           // 전체 지우기
//                 LCD_Print("Enter Password ");
//                 LCD_Send(0xC0, 0);           // 커서를 2번째 줄로 이동
//                 lcd_shown = true;
//             }

//             printf("PD2 = %d\n", Macro_Check_Bit_Set(GPIOD->IDR, 2));

//             if(ms1_flag)
//             {
//                 ms1_flag = false;
//                 Keypad_Pressed();
//             }

//             if(row0_flag)
//             {
//                 row0_flag = false;
//                 switch(row0_code)
//                 {
//                     case 0:
//                         if(idx < 4)
//                         {
//                             buf[idx] = keypad[0][0];
//                             idx++;
//                             printf("*");
//                             LCD_Print("*");
//                         }
//                         break;
//                     case 1:
//                         if(idx < 4)
//                         {
//                             buf[idx] = keypad[0][1];
//                             idx++;
//                             printf("*");
//                             LCD_Print("*");
//                         }
//                         break;
//                     case 2:
//                         if(idx < 4)
//                         {
//                             buf[idx] = keypad[0][2];
//                             idx++;
//                             printf("*");
//                             LCD_Print("*");
//                         }
//                         break;
//                     case 3:
//                         if(idx < 4)
//                         {
//                             buf[idx] = keypad[0][3];
//                             idx++;
//                             printf("*");
//                             LCD_Print("*");
//                         }
//                         break;
//                 }
//             }

//             if(row1_flag)
//             {
//                 row1_flag = false;
//                 switch(row1_code)
//                 {
//                     case 0:
//                         if(idx < 4)
//                         {
//                             buf[idx] = keypad[1][0];
//                             idx++;
//                             printf("*");
//                             LCD_Print("*");
//                         }
//                         break;
//                     case 1:
//                         if(idx < 4)
//                         {
//                             buf[idx] = keypad[1][1];
//                             idx++;
//                             printf("*");
//                             LCD_Print("*");
//                         }
//                         break;
//                     case 2:
//                         if(idx < 4)
//                         {
//                             buf[idx] = keypad[1][2];
//                             idx++;
//                             printf("*");
//                             LCD_Print("*");
//                         }
//                         break;
//                     case 3:
//                         if(idx < 4)
//                         {
//                             buf[idx] = keypad[1][3];
//                             idx++;
//                             printf("*");
//                             LCD_Print("*");
//                         }
//                         break;
//                 }
//             }

//             if(row2_flag)
//             {
//                 row2_flag = false;
//                 switch(row2_code)
//                 {
//                     case 0:
//                         if(idx < 4)
//                         {
//                             buf[idx] = keypad[2][0];
//                             idx++;
//                             printf("*");
//                             LCD_Print("*");
//                         }
//                         break;
//                     case 1:
//                         if(idx < 4)
//                         {
//                             buf[idx] = keypad[2][1];
//                             idx++;
//                             printf("*");
//                             LCD_Print("*");
//                         }
//                         break;
//                     case 2:
//                         if(idx < 4)
//                         {
//                             buf[idx] = keypad[2][2];
//                             idx++;
//                             printf("*");
//                             LCD_Print("*");
//                         }
//                         break;
//                     case 3:
//                         if(idx < 4)
//                         {
//                             buf[idx] = keypad[2][3];
//                             idx++;
//                             printf("*");
//                             LCD_Print("*");
//                         }
//                         break;
//                 }
//             }

//             if(row3_flag)
//             {
//                 row3_flag = false;
//                 switch(row3_code)
//                 {
//                     case 0:
//                         if(idx < 4)
//                         {
//                             buf[idx] = keypad[3][0];
//                             idx++;
//                             printf("*");
//                             LCD_Print("*");
//                         }
//                         else
//                         {
//                             buf[idx] = keypad[3][0];
//                             idx = 0;
//                             if(memcmp(buf, password, 4) == 0)
//                             {
//                                 printf("\n비밀번호 pass\n");
//                                 LCD_ClearLine2();
//                                 LCD_Print("Open!");
//                                 door_open();
//                                 // 열린 소리
//                                 for(i = 0; i < (sizeof(song_open)/sizeof(song_open[0])); i++)
//                                 {
//                                     Buzzer_Beep(song_open[i][0], song_open[i][1]);
//                                 }
//                             }
//                             else 
//                             {
//                                 printf("\n비밀번호가 틀렸습니다. 다시입력해주세요.\n");
//                                 LCD_ClearLine2();
//                                 LCD_Print("Try Again");
//                                 // 닫힌 소리
//                                 for(i = 0; i < (sizeof(song_close)/sizeof(song_close[0])); i++)
//                                 {
//                                     Buzzer_Beep(song_close[i][0], song_close[i][1]);
//                                 }
//                             }
//                             pir_detected = false;
//                             lcd_shown = false;
//                         }
//                         break;
//                     case 1:
//                         if(idx < 4)
//                         {
//                             buf[idx] = keypad[3][1];
//                             idx++;
//                             printf("*");
//                             LCD_Print("*");
//                         }
//                         break;
//                     case 2:
//                         if(idx < 4)
//                         {
//                             buf[idx] = keypad[3][2];
//                             idx++;
//                             printf("*");
//                             LCD_Print("*");
//                         }
//                         break;
//                     case 3:
//                         if(idx < 4)
//                         {
//                             buf[idx] = keypad[3][3];
//                             idx++;
//                             printf("*");
//                             LCD_Print("*");
//                         }
//                         break;
//                 }
//             }
//         }
//     }  
// }