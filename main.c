#include "stm32f10x.h"
#include <stdio.h>

void GPIO_config(void);
void set_GPIO_input(void);
void set_GPIO_output(void);
int read_GPIO(void);
void DHT11_Start(void);
int DHT11_CheckResponse(void);
uint8_t DHT11_ReadByte(void);
void delay_us(uint32_t microseconds);
void SystemClock_Config(void);
void USART1_Config(void);
void USART1_SendString(char *str);

int main() {
	GPIO_config();
	SystemClock_Config();
	USART1_Config();
	uint8_t RH_integral, RH_decimal, T_integral, T_decimal, checksum;
  char buffer[100];
	while(1) {
    DHT11_Start();
    if (DHT11_CheckResponse()) {
      RH_integral = DHT11_ReadByte();
      RH_decimal = DHT11_ReadByte();
      T_integral = DHT11_ReadByte();
      T_decimal = DHT11_ReadByte();
      checksum = DHT11_ReadByte();
			if (checksum == ((RH_integral + RH_decimal + T_integral + T_decimal) & 0xFF)) {
                sprintf(buffer, "Humidity: %d.%d%%, Temperature: %d.%dC\r\n",
                        RH_integral, RH_decimal, T_integral, T_decimal);
                USART1_SendString(buffer); 
      }
	  }
		for (volatile int i = 0; i < 1000000; i++);
  }
}

void GPIO_config(void) {
	// Enable clock for GPIOA, bit2(IOPAEN)
	RCC->APB2ENR |= (1<<2); 
	// Clear MODE0 and CNF0
	GPIOA->CRL &= ~((0x3 << 0) | (0x3 << 2));
	// Output mode 10, Max speed 2Mhz
	GPIOA->CRL |= (0x2<<0);
	// Output open drain 01
	GPIOA->CRL |= (0x1<<2);
	
}

void set_GPIO_input(void) {
	// Clear MODE0 and CNF0
	GPIOA->CRL &= ~((0x3<<0)|(0x3<<2)); 
	// Floating input
	GPIOA->CRL |= (0x1<<2);
}

void set_GPIO_output(void) {
	// Clear CNF0 bits
	GPIOA->CRL &= ~(0x3 << 2); 
	// MODE0 = 10, CNF0 = 01 (Output open-drain)
	GPIOA->CRL |= (0x2 << 0) | (0x1 << 2);  
}

int read_GPIO(void) {
   return (GPIOA->IDR & (1 << 0)) ? 1 : 0;
}

void DHT11_Start(void) {
   set_GPIO_output();
   // Set PA0 low
   GPIOA->BSRR = (1 << 16); 
   delay_us(18000);    
   // Set PA0 high	
   GPIOA->BSRR = 1;          
   delay_us(20);           
   set_GPIO_input();
}


int DHT11_CheckResponse(void) {
   int response = 0;
   delay_us(40);
   if (!read_GPIO()) {
      delay_us(80);
      if (read_GPIO()) response = 1;
      delay_us(50);
   }
   return response;
}

uint8_t DHT11_ReadByte(void) {
   uint8_t i, data = 0;
   for (i = 0; i < 8; i++) {
     while (!read_GPIO());
       delay_us(40);
       if (!read_GPIO()) data &= ~(1 << (7 - i));  
       else {
          data |= (1 << (7 - i));  
          while (read_GPIO()); 
       }
   }
   return data;
}
void delay_us(uint32_t microseconds) {
   SysTick->LOAD = (SystemCoreClock / 1000000) * microseconds - 1; 
   SysTick->VAL = 0; 
	 // Enable SysTick
   SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk; 
   while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0);
	 // disable SysTick
	 SysTick->CTRL = 0; 
}

void USART1_Config(void) {
	 RCC->APB2ENR |= 1 << 14; 
   // GPIOA PA9 for USART1 TX as alternate function push-pull
	 // Enable GPIOA clock, bit 2 is IOPAEN
   RCC->APB2ENR |= 1 << 2;  
   GPIOA->CRH &= ~((0x0F) << (4 * (9 - 8))); 
   GPIOA->CRH |= (0x0B << (4 * (9 - 8)));  
   // Set baud rate 115200
   USART1->BRR = 72000000 /115200; 
   USART1->CR1 |= (1 << 13) | (1 << 3) | (1 << 2);
}

void USART1_SendString(char *str) {
    while (*str) {
        while (!(USART1->SR & USART_SR_TXE)); 
        USART1->DR = (uint8_t)(*str++); 
    }
}

void SystemClock_Config(void) {
	 RCC->CR |= RCC_CR_HSEON; 
   while (!(RCC->CR & RCC_CR_HSERDY)); 
   RCC->CFGR |= RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9;
   RCC->CR |= RCC_CR_PLLON; 
   while (!(RCC->CR & RCC_CR_PLLRDY)); 
   RCC->CFGR |= RCC_CFGR_SW_PLL;
   while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
   SystemCoreClockUpdate();
}
