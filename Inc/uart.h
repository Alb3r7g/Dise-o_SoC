#ifndef UART_H
#define UART_H

#include <stdint.h>

void USER_USART1_Init(void);
void USER_USART1_SendChar(char c);
void USER_USART1_SendString(const char *str);
int USER_USART1_ReceiveChar(void);

#endif /* UART_H */
