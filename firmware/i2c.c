#include "device_driver.h"

/* freg = 5000 ~ 400000 */

#define LCD_I2CADDR									0x27
#define LCD_I2CADDR_WR								(LCD_I2CADDR << 1)
#define LCD_I2CADDR_RD								((LCD_I2CADDR <<1) |0x1)

/* I2C2_SCL => PB10, I2C2_SDA => PB9 */

void I2C_LCD_Init(unsigned int freq)
{
	unsigned int r;
	volatile int i;
	
	Macro_Set_Bit(RCC->AHB1ENR, 1); 					// Port-B Clock On
	Macro_Set_Bit(RCC->APB1ENR, 22); 					// I2C2 Clock On

	Macro_Clear_Bit(RCC->APB1RSTR, 22); 				// I2C2 Reset
	Macro_Set_Bit(RCC->APB1RSTR, 22);
	for(i = 0; i < 1000; i++);
	Macro_Clear_Bit(RCC->APB1RSTR, 22);

	Macro_Write_Block(GPIOB->MODER, 0xf, 0xa, 18);  	// PB[10:9] => ALT
	Macro_Write_Block(GPIOB->AFR[1], 0xf, 0x4, 8); 	    // PB[10], SCL => AF04
	Macro_Write_Block(GPIOB->AFR[1], 0xf, 0x9, 4);      // PB[9], SDA => AF09
	Macro_Write_Block(GPIOB->OTYPER, 0x3, 0x3, 9); 		// PB[10:9] => Open Drain
	Macro_Write_Block(GPIOB->OSPEEDR, 0xf, 0xa, 18); 	// PB[10:9] => Fast Speed
	Macro_Write_Block(GPIOB->PUPDR, 0xf, 0x5, 18); 		// PB[10:9] => Internal Pull-up

	Macro_Write_Block(I2C2->CR2, 0x3f, PCLK1 / 1000000, 0);
	Macro_Clear_Bit(I2C2->CR1, 0);
	I2C2->TRISE = (PCLK1 / 1000000) + 1;
	r = PCLK1 / (freq * 2);
	I2C2->CCR = ((r < 4) ? 4 : r);

	Macro_Clear_Bit(I2C2->CR1, 1);
	Macro_Set_Bit(I2C2->CR1, 0);
	Macro_Set_Bit(I2C2->CR1, 10);
}

void I2C_LCD_Write(unsigned int data)
{
	while(Macro_Check_Bit_Set(I2C2->SR2, 1)); 					// Idle OK

	Macro_Set_Bit(I2C2->CR1, 8); 								// Start
	while(Macro_Check_Bit_Clear(I2C2->SR1, 0));					// Check Start

	I2C2->DR = LCD_I2CADDR_WR;									// Send WR Address
	while(Macro_Check_Bit_Clear(I2C2->SR1, 1));					// Check Address
	(void)I2C2->SR2;											// Clear ADDR flag by reading SR2

	while(Macro_Check_Bit_Clear(I2C2->SR1, 7));					// Check TxE	
	I2C2->DR = data;											// Send Data
	while(Macro_Check_Bit_Clear(I2C2->SR1, 2));					// Check Byte Transfer Finished

	Macro_Set_Bit(I2C2->CR1, 9); 								// Stop
	while(Macro_Check_Bit_Set(I2C2->CR1, 9));					// Check Stop(Auto Cleared)
}

void LCD_Send(unsigned char byte, unsigned char rs)
{
	unsigned char nibble[2] = { byte >> 4, byte & 0x0f };
	int n;

	for(n = 0; n < 2; n++)
	{
		I2C_LCD_Write((nibble[n] << 4) | rs | 0x08 | 0x04);   // EN=1
		TIM2_Delay(1);
		I2C_LCD_Write((nibble[n] << 4) | rs | 0x08);          // EN=0
		TIM2_Delay(1);
	}
}

void LCD_Init(void)
{
	I2C_LCD_Init(100000);
	TIM2_Delay(50);

	LCD_Send(0x33, 0);
	LCD_Send(0x32, 0);
	LCD_Send(0x28, 0);   // 4bit, 2line
	LCD_Send(0x0C, 0);   // Display ON
	LCD_Send(0x06, 0);   // Entry mode
	LCD_Send(0x01, 0);   // Clear
	TIM2_Delay(2);
}


void LCD_Print(char *str)
{
	while(*str)
	{
		LCD_Send(*str++, 1);
	} 
}