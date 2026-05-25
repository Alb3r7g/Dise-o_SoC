#include "motor_test.h"

void USER_RCC_Init(void);
void USER_GPIO_Init(void);

void USER_ADC_Init(void);
void USER_ADC_Calibration(void);
uint16_t USER_ADC_Read(void);

void USER_PWM_Init(void);

void USER_Motor_Forward(void);
void USER_Motor_Stop(void);

void USER_Delay(void);

int main(void) {
  uint16_t adcValue;
  uint8_t motorState = 0;

  USER_RCC_Init();
  USER_GPIO_Init();
  USER_PWM_Init();
  USER_ADC_Init();
  USER_ADC_Calibration();

  ADC1->CR2 |= ADC_CR2_ADON;

  while (1) {
    /* ================= BUTTON ================= */

    if (!(GPIOA->IDR & (0x1UL << 4U))) {
      USER_Delay();

      if (!(GPIOA->IDR & (0x1UL << 4U))) {
        motorState ^= 1;

        while (!(GPIOA->IDR & (0x1UL << 4U)))
          ;
      }
    }

    /* ================= MOTORS ON ================= */
    if (motorState) {
      USER_Motor_Forward();

      ADC1->CR2 |= ADC_CR2_SWSTART;

      adcValue = USER_ADC_Read();

      /* 0-4095 -> 0-1000 */
      adcValue = adcValue / 4;

      /* dead zone */
      if (adcValue < 200) {
        adcValue = 0;
      }

      /* PWM all motors */
      TIM3->CCR1 = adcValue;
      TIM3->CCR2 = adcValue;
      TIM3->CCR3 = adcValue;
      TIM3->CCR4 = adcValue;
    }
    /* ================= MOTORS OFF ================= */
    else {
      USER_Motor_Stop();

      TIM3->CCR1 = 0;
      TIM3->CCR2 = 0;
      TIM3->CCR3 = 0;
      TIM3->CCR4 = 0;
    }
  }
}

/* ================================================================ */

void USER_RCC_Init(void) {
  /* GPIOA + GPIOB + ADC1 */
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_ADC1EN;

  /* TIM3 CLOCK */
  RCC->APB1ENR |= (0x1UL << 1U);

  /* ADC PRESCALER */
  RCC->CFGR &= ~(0x3UL << 14U);
  RCC->CFGR |= (0x2UL << 14U);
}

/* ================================================================ */

void USER_GPIO_Init(void) {
  /* ================= PA0 ANALOG ================= */
  GPIOA->CRL &= ~(0xFUL << 0U);

  /* ================= PA4 BUTTON ================= */
  GPIOA->CRL &= ~(0xFUL << 16U);
  GPIOA->CRL |= (0x8UL << 16U);
  GPIOA->ODR |= (0x1UL << 4U);

  /* --- MOTOR 3: CAMBIADO A PB12 Y PB13 PARA EVITAR COLISION CON JTAG --- */
  /* ================= PB12 OUTPUT (Motor 3 IN1) ================= */
  GPIOB->CRH &= ~(0xFUL << 16U);
  GPIOB->CRH |= (0x1UL << 16U);

  /* ================= PB13 OUTPUT (Motor 3 IN2) ================= */
  GPIOB->CRH &= ~(0xFUL << 20U);
  GPIOB->CRH |= (0x1UL << 20U);

  /* ================= PB5 OUTPUT (Motor 1) ================= */
  GPIOB->CRL &= ~(0xFUL << 20U);
  GPIOB->CRL |= (0x1UL << 20U);

  /* ================= PB6 OUTPUT (Motor 1) ================= */
  GPIOB->CRL &= ~(0xFUL << 24U);
  GPIOB->CRL |= (0x1UL << 24U);

  /* ================= PB7 OUTPUT (Motor 2) ================= */
  GPIOB->CRL &= ~(0xFUL << 28U);
  GPIOB->CRL |= (0x1UL << 28U);

  /* ================= PB8 OUTPUT (Motor 2) ================= */
  GPIOB->CRH &= ~(0xFUL << 0U);
  GPIOB->CRH |= (0x1UL << 0U);

  /* ================= PB10 OUTPUT (Motor 4) ================= */
  GPIOB->CRH &= ~(0xFUL << 8U);
  GPIOB->CRH |= (0x1UL << 8U);

  /* ================= PB11 OUTPUT (Motor 4) ================= */
  GPIOB->CRH &= ~(0xFUL << 12U);
  GPIOB->CRH |= (0x1UL << 12U);
}

/* ================================================================ */

void USER_ADC_Init(void) {
  ADC1->CR1 &= ~ADC_CR1_DUALMOD;
  ADC1->CR2 &= ~ADC_CR2_ALIGN;
  ADC1->CR2 |= ADC_CR2_CONT;
  ADC1->SMPR2 &= ~ADC_SMPR2_SMP0;
  ADC1->SQR1 &= ~ADC_SQR1_L;
  ADC1->SQR3 &= ~ADC_SQR3_SQ1;
  ADC1->CR2 |= ADC_CR2_ADON;

  for (volatile int i = 0; i < 10000; i++)
    ;
}

/* ================================================================ */

void USER_ADC_Calibration(void) {
  ADC1->CR2 |= ADC_CR2_CAL;
  while (ADC1->CR2 & ADC_CR2_CAL)
    ;
}

/* ================================================================ */

uint16_t USER_ADC_Read(void) {
  while (!(ADC1->SR & ADC_SR_EOC))
    ;
  ADC1->SR &= ~ADC_SR_EOC;
  return (uint16_t)ADC1->DR;
}

/* ================================================================ */

void USER_PWM_Init(void) {
  /* ================= PA6 TIM3_CH1 ================= */
  GPIOA->CRL &= ~(0xFUL << 24U);
  GPIOA->CRL |= (0xBUL << 24U);

  /* ================= PA7 TIM3_CH2 ================= */
  GPIOA->CRL &= ~(0xFUL << 28U);
  GPIOA->CRL |= (0xBUL << 28U);

  /* ================= PB0 TIM3_CH3 ================= */
  GPIOB->CRL &= ~(0xFUL << 0U);
  GPIOB->CRL |= (0xBUL << 0U);

  /* ================= PB1 TIM3_CH4 ================= */
  GPIOB->CRL &= ~(0xFUL << 4U);
  GPIOB->CRL |= (0xBUL << 4U);

  /* ================= PWM FREQUENCY ================= */
  TIM3->PSC = 8 - 1;
  TIM3->ARR = 1000;

  /* ================= PWM MODE CH1 ================= */
  TIM3->CCMR1 &= ~(0x7UL << 4U);
  TIM3->CCMR1 |= (0x6UL << 4U);

  /* ================= PWM MODE CH2 ================= */
  TIM3->CCMR1 &= ~(0x7UL << 12U);
  TIM3->CCMR1 |= (0x6UL << 12U);

  /* ================= PWM MODE CH3 ================= */
  TIM3->CCMR2 &= ~(0x7UL << 4U);
  TIM3->CCMR2 |= (0x6UL << 4U);

  /* ================= PWM MODE CH4 ================= */
  TIM3->CCMR2 &= ~(0x7UL << 12U);
  TIM3->CCMR2 |= (0x6UL << 12U);

  /* ================= PRELOAD ================= */
  TIM3->CCMR1 |= (0x1UL << 3U);
  TIM3->CCMR1 |= (0x1UL << 11U);
  TIM3->CCMR2 |= (0x1UL << 3U);
  TIM3->CCMR2 |= (0x1UL << 11U);

  /* ================= ENABLE CHANNELS ================= */
  TIM3->CCER |= (0x1UL << 0U);
  TIM3->CCER |= (0x1UL << 4U);
  TIM3->CCER |= (0x1UL << 8U);
  TIM3->CCER |= (0x1UL << 12U);

  /* ================= INITIAL DUTY ================= */
  TIM3->CCR1 = 0;
  TIM3->CCR2 = 0;
  TIM3->CCR3 = 0;
  TIM3->CCR4 = 0;

  /* ================= AUTO RELOAD ================= */
  TIM3->CR1 |= (0x1UL << 7U);
  TIM3->EGR |= (0x1UL << 0U);

  /* ================= START TIMER ================= */
  TIM3->CR1 |= (0x1UL << 0U);
}

/* ================================================================ */

void USER_Motor_Forward(void) {
  /* ================= MOTOR 1 ================= */
  GPIOB->BSRR = (0x1UL << 5U);
  GPIOB->BRR = (0x1UL << 6U);

  /* ================= MOTOR 2 ================= */
  GPIOB->BSRR = (0x1UL << 7U);
  GPIOB->BRR = (0x1UL << 8U);

  /* ================= MOTOR 3 (Modificado PB12 y PB13) ================= */
  GPIOB->BSRR = (0x1UL << 12U);
  GPIOB->BRR = (0x1UL << 13U);

  /* ================= MOTOR 4 ================= */
  GPIOB->BSRR = (0x1UL << 10U);
  GPIOB->BRR = (0x1UL << 11U);
}

/* ================================================================ */

void USER_Motor_Stop(void) {
  // Limpiamos todos los pines de dirección para detener los motores
  GPIOB->BRR = (0x1UL << 5U) | (0x1UL << 6U) | (0x1UL << 7U) | (0x1UL << 8U) |
               (0x1UL << 10U) | (0x1UL << 11U) | (0x1UL << 12U) |
               (0x1UL << 13U);
}

/* ================================================================ */

void USER_Delay(void) {
  for (volatile int i = 0; i < 100000; i++)
    ;
}
