#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

void USER_USART2_Init(void);
void USER_USART2_SendChar(char c);
void USER_USART2_SendString(char *str);
void USER_IntToStr(uint32_t num, char *str);
uint16_t USER_ADC_Read(void);
void USER_Delay_ms(uint32_t ms);

#endif
