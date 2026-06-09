#ifndef INIT_H
#define INIT_H

#include <stdint.h>

void USER_SystemClock_Config(void);
void USER_RCC_Init(void);
void USER_GPIO_Init(void);
void USER_PWM_Init(void);
void USER_EXTI_Init(void);
void USER_ADC_Init(void);

#endif
