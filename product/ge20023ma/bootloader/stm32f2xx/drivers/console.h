

#ifndef _CONSOLE_H
#define _CONSOLE_H

/*************************************************************************
 * Definition for COM port0, connected to USART1                     
 * TX pin: PA9, RX pin: PA10 
 *************************************************************************/
#define COM_PORT0						USART1
#define COM_PORT0_CLK					RCC_APB2Periph_USART1
#define COM_PORT0_TX_PIN				GPIO_Pin_9
#define COM_PORT0_TX_GPIO_PORT			GPIOA
#define COM_PORT0_TX_GPIO_CLK			RCC_AHB1Periph_GPIOA
#define COM_PORT0_TX_SOURCE				GPIO_PinSource9
#define COM_PORT0_TX_AF					GPIO_AF_USART1
#define COM_PORT0_RX_PIN				GPIO_Pin_10
#define COM_PORT0_RX_GPIO_PORT			GPIOA
#define COM_PORT0_RX_GPIO_CLK			RCC_AHB1Periph_GPIOA
#define COM_PORT0_RX_SOURCE				GPIO_PinSource10
#define COM_PORT0_RX_AF					GPIO_AF_USART1
#define COM_PORT0_IRQn 					USART1_IRQn

/*************************************************************************
 * Definition for COM port1, connected to USART2                     
 * TX pin: PD5, RX pin: PD6 
 *************************************************************************/
#define COM_PORT1                       USART2
#define COM_PORT1_CLK                   RCC_APB1Periph_USART2
#define COM_PORT1_TX_PIN                GPIO_Pin_5
#define COM_PORT1_TX_GPIO_PORT          GPIOD
#define COM_PORT1_TX_GPIO_CLK           RCC_AHB1Periph_GPIOD
#define COM_PORT1_TX_SOURCE             GPIO_PinSource5
#define COM_PORT1_TX_AF                 GPIO_AF_USART2
#define COM_PORT1_RX_PIN                GPIO_Pin_6
#define COM_PORT1_RX_GPIO_PORT          GPIOD
#define COM_PORT1_RX_GPIO_CLK           RCC_AHB1Periph_GPIOD
#define COM_PORT1_RX_SOURCE             GPIO_PinSource6
#define COM_PORT1_RX_AF                 GPIO_AF_USART2
#define COM_PORT1_IRQn                  USART2_IRQn

/*************************************************************************
 * Definition for COM port1, connected to USART3                     
 * TX pin: PD8, RX pin: PD9 
 *************************************************************************/
#define COM_PORT2                       USART3
#define COM_PORT2_CLK                   RCC_APB1Periph_USART3
#define COM_PORT2_TX_PIN                GPIO_Pin_8
#define COM_PORT2_TX_GPIO_PORT          GPIOD
#define COM_PORT2_TX_GPIO_CLK           RCC_AHB1Periph_GPIOD
#define COM_PORT2_TX_SOURCE             GPIO_PinSource8
#define COM_PORT2_TX_AF                 GPIO_AF_USART3
#define COM_PORT2_RX_PIN                GPIO_Pin_9
#define COM_PORT2_RX_GPIO_PORT          GPIOD
#define COM_PORT2_RX_GPIO_CLK           RCC_AHB1Periph_GPIOD
#define COM_PORT2_RX_SOURCE             GPIO_PinSource9
#define COM_PORT2_RX_AF                 GPIO_AF_USART3
#define COM_PORT2_IRQn                  USART3_IRQn

/*************************************************************************
 * Definition for COM port1, connected to USART5                     
 * TX pin: PD0, RX pin: PD2 
 *************************************************************************/
#define COM_PORT4                       UART5
#define COM_PORT4_CLK                   RCC_APB1Periph_UART5
#define COM_PORT4_TX_PIN                GPIO_Pin_12
#define COM_PORT4_TX_GPIO_PORT          GPIOC
#define COM_PORT4_TX_GPIO_CLK           RCC_AHB1Periph_GPIOC
#define COM_PORT4_TX_SOURCE             GPIO_PinSource12
#define COM_PORT4_TX_AF                 GPIO_AF_UART5
#define COM_PORT4_RX_PIN                GPIO_Pin_2
#define COM_PORT4_RX_GPIO_PORT          GPIOD
#define COM_PORT4_RX_GPIO_CLK           RCC_AHB1Periph_GPIOD
#define COM_PORT4_RX_SOURCE             GPIO_PinSource2
#define COM_PORT4_RX_AF                 GPIO_AF_UART5
#define COM_PORT4_IRQn                  UART5_IRQn

typedef enum { 
	COM_USART1,
	COM_USART2, 
	COM_USART3,
	COM_UART4,
	COM_UART5,
	COM_USART6	
} eComPort;

typedef enum { 
	RS232,
	RS485	
} eComMode;

void ConsolePutChar(char c);
void ConsolePutString(const char * s);
void ConsoleInit(eComPort Port, unsigned int BaudRate);
char ConsoleGetChar(void);
void ConsoleDisable(void);
void ConsoleEnable(void);
int ConsoleCheckRxChar(char c);

#endif

