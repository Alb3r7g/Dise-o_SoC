#ifndef MOTOR_TEST_H_
#define MOTOR_TEST_H_

#include <stdint.h>

/* ================= RCC ================= */

typedef struct{
	volatile uint32_t CR;
	volatile uint32_t CFGR;
	volatile uint32_t CIR;
	volatile uint32_t APB2RSTR;
	volatile uint32_t APB1RSTR;
	volatile uint32_t AHBENR;
	volatile uint32_t APB2ENR;
	volatile uint32_t APB1ENR;
	volatile uint32_t BDCR;
	volatile uint32_t CSR;
} RCC_TypeDef;

/* ================= GPIO ================= */

typedef struct{
	volatile uint32_t CRL;
	volatile uint32_t CRH;
	volatile uint32_t IDR;
	volatile uint32_t ODR;
	volatile uint32_t BSRR;
	volatile uint32_t BRR;
	volatile uint32_t LCKR;
} GPIO_TypeDef;

/* ================= ADC ================= */

typedef struct{
	volatile uint32_t SR;
	volatile uint32_t CR1;
	volatile uint32_t CR2;
	volatile uint32_t SMPR1;
	volatile uint32_t SMPR2;
	volatile uint32_t JOFR1;
	volatile uint32_t JOFR2;
	volatile uint32_t JOFR3;
	volatile uint32_t JOFR4;
	volatile uint32_t HTR;
	volatile uint32_t LTR;
	volatile uint32_t SQR1;
	volatile uint32_t SQR2;
	volatile uint32_t SQR3;
	volatile uint32_t JSQR;
	volatile uint32_t JDR1;
	volatile uint32_t JDR2;
	volatile uint32_t JDR3;
	volatile uint32_t JDR4;
	volatile uint32_t DR;
} ADC_TypeDef;

/* ================= TIMER ================= */

typedef struct{
	volatile uint32_t CR1;
	volatile uint32_t CR2;
	volatile uint32_t SMCR;
	volatile uint32_t DIER;
	volatile uint32_t SR;
	volatile uint32_t EGR;
	volatile uint32_t CCMR1;
	volatile uint32_t CCMR2;
	volatile uint32_t CCER;
	volatile uint32_t CNT;
	volatile uint32_t PSC;
	volatile uint32_t ARR;
	volatile uint32_t RESERVED1;
	volatile uint32_t CCR1;
	volatile uint32_t CCR2;
	volatile uint32_t CCR3;
	volatile uint32_t CCR4;
	volatile uint32_t RESERVED2;
	volatile uint32_t DCR;
	volatile uint32_t DMAR;
} TIM_TypeDef;

/* ================= BASE ADDRESSES ================= */

#define RCC_BASE        0x40021000UL

#define GPIOA_BASE      0x40010800UL
#define GPIOB_BASE      0x40010C00UL

#define ADC1_BASE       0x40012400UL

#define TIM2_BASE       0x40000000UL
#define TIM3_BASE       0x40000400UL
#define TIM4_BASE       0x40000800UL

/* ================= POINTERS ================= */

#define RCC             ((RCC_TypeDef*) RCC_BASE)

#define GPIOA           ((GPIO_TypeDef*) GPIOA_BASE)
#define GPIOB           ((GPIO_TypeDef*) GPIOB_BASE)

#define ADC1            ((ADC_TypeDef*) ADC1_BASE)

#define TIM2    		((TIM_TypeDef*) TIM2_BASE)
#define TIM3            ((TIM_TypeDef*) TIM3_BASE)
#define TIM4    		((TIM_TypeDef*) TIM4_BASE)

/* ================= RCC ================= */

#define RCC_APB2ENR_IOPAEN     (0x1UL << 2U)
#define RCC_APB2ENR_IOPBEN     (0x1UL << 3U)
#define RCC_APB2ENR_ADC1EN     (0x1UL << 9U)

/* ================= ADC ================= */

#define ADC_CR2_ADON           (0x1UL << 0U)
#define ADC_CR2_CONT           (0x1UL << 1U)
#define ADC_CR2_CAL            (0x1UL << 2U)
#define ADC_CR2_ALIGN          (0x1UL << 11U)
#define ADC_CR2_SWSTART        (0x1UL << 22U)

#define ADC_SR_EOC             (0x1UL << 1U)

#define ADC_CR1_DUALMOD        (0xFUL << 16U)

#define ADC_SMPR2_SMP0         (0x7UL << 0U)

#define ADC_SQR1_L             (0xFUL << 20U)

#define ADC_SQR3_SQ1           (0x1FUL << 0U)

/* ================= GPIO ================= */

#define GPIO_CRL_MODE0         (0x3UL << 0U)
#define GPIO_CRL_CNF0          (0x3UL << 2U)

#endif
