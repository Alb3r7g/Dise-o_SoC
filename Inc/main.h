#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>

/* --- ESTRUCTURAS DE REGISTROS (DEFINICIÓN MANUAL BARE METAL) --- */

typedef struct {
    volatile uint32_t ACR;
    volatile uint32_t KEYR;
    volatile uint32_t OPTKEYR;
    volatile uint32_t SR;
    volatile uint32_t CR;
    volatile uint32_t AR;
    volatile uint32_t reserved;
    volatile uint32_t OBR;
    volatile uint32_t WRPR;
} FLASH_TypeDef;

typedef struct {
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

typedef struct {
    volatile uint32_t CRL;
    volatile uint32_t CRH;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t BRR;
    volatile uint32_t LCKR;
} GPIO_TypeDef;

typedef struct {
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

typedef struct {
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

typedef struct {
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t GTPR;
} USART_TypeDef;

typedef struct {
  volatile uint32_t ISER[8U];               
  uint32_t RESERVED0[24U];
  volatile uint32_t ICER[8U];               
  uint32_t RSERVED1[24U];
  volatile uint32_t ISPR[8U];               
  uint32_t RESERVED2[24U];
  volatile uint32_t ICPR[8U];               
  uint32_t RESERVED3[24U];
  volatile uint32_t IABR[8U];               
  uint32_t RESERVED4[56U];
  volatile uint8_t  IP[240U];               
  uint32_t RESERVED5[644U];
  volatile uint32_t STIR;                   
} NVIC_Type;

/* --- DIRECCIONES BASE --- */

#define FLASH_BASE     0x40022000UL
#define RCC_BASE       0x40021000UL
#define GPIOA_BASE     0x40010800UL
#define GPIOB_BASE     0x40010C00UL
#define GPIOC_BASE     0x40011000UL
#define ADC1_BASE      0x40012400UL
#define TIM2_BASE      0x40000000UL
#define TIM3_BASE      0x40000400UL
#define USART2_BASE    0x40004400UL
#define NVIC_BASE      0xE000E100UL

/* --- PUNTEROS A PERIFÉRICOS --- */

#define FLASH          ((FLASH_TypeDef*) FLASH_BASE)
#define RCC            ((RCC_TypeDef*) RCC_BASE)
#define GPIOA          ((GPIO_TypeDef*) GPIOA_BASE)
#define GPIOB          ((GPIO_TypeDef*) GPIOB_BASE)
#define GPIOC          ((GPIO_TypeDef*) GPIOC_BASE)
#define ADC1           ((ADC_TypeDef*) ADC1_BASE)
#define TIM2           ((TIM_TypeDef*) TIM2_BASE)
#define TIM3           ((TIM_TypeDef*) TIM3_BASE)
#define USART2         ((USART_TypeDef*) USART2_BASE)
#define NVIC           ((NVIC_Type*) NVIC_BASE)

/* --- MÁSCARAS Y BITS (RCC) --- */

#define RCC_APB2ENR_IOPAEN     (0x1UL << 2U)
#define RCC_APB2ENR_IOPBEN     (0x1UL << 3U)
#define RCC_APB2ENR_IOPCEN     (0x1UL << 4U)
#define RCC_APB2ENR_ADC1EN     (0x1UL << 9U)
#define RCC_APB2ENR_AFIOEN     (0x1UL << 0U)
#define RCC_APB1ENR_TIM2EN     (0x1UL << 0U)
#define RCC_APB1ENR_TIM3EN     (0x1UL << 1U)
#define RCC_APB1ENR_USART2EN   (0x1UL << 17U)
#define RCC_CFGR_ADCPRE        (0x3UL << 14U)

/* --- MÁSCARAS Y BITS (GPIO) --- */

#define GPIO_CRL_MODE0         (0x3UL << 0U)
#define GPIO_CRL_CNF0          (0x3UL << 2U)
#define GPIO_CRL_MODE1_1       (0x1UL << 5U)
#define GPIO_CRL_MODE4_1       (0x1UL << 17U)
#define GPIO_CRL_MODE5_1       (0x1UL << 21U)
#define GPIO_CRL_MODE6_1       (0x1UL << 25U)

#define GPIO_CRL_MODE2         (0x3UL << 8U)
#define GPIO_CRL_CNF2          (0x3UL << 10U)
#define GPIO_CRL_MODE2_0       (0x1UL << 8U)
#define GPIO_CRL_MODE2_1       (0x1UL << 9U)
#define GPIO_CRL_CNF2_1        (0x1UL << 11U)
#define GPIO_CRL_MODE3         (0x3UL << 12U)
#define GPIO_CRL_CNF3          (0x3UL << 14U)
#define GPIO_CRL_CNF3_0        (0x1UL << 14U)

#define GPIO_CRH_MODE13        (0x3UL << 20U)
#define GPIO_CRH_CNF13         (0x3UL << 22U)
#define GPIO_CRH_CNF13_1       (0x1UL << 23U)
#define GPIO_ODR_ODR13         (0x1UL << 13U)
#define GPIO_IDR_IDR13         (0x1UL << 13U)

/* --- MÁSCARAS Y BITS (ADC) --- */

#define ADC_CR2_ADON           (0x1UL << 0U)
#define ADC_CR2_CONT           (0x1UL << 1U)
#define ADC_CR2_CAL            (0x1UL << 2U)
#define ADC_CR2_ALIGN          (0x1UL << 11U)
#define ADC_CR1_DUALMOD        (0xFUL << 16U)
#define ADC_SR_EOC             (0x1UL << 1U)
#define ADC_SMPR2_SMP0         (0x7UL << 0U)
#define ADC_SQR1_L             (0xFUL << 20U)
#define ADC_SQR3_SQ1           (0x1FUL << 0U)

/* --- MÁSCARAS Y BITS (TIMER) --- */

#define TIM_CR1_CEN            (0x1UL << 0U)
#define TIM_CR1_ARPE           (0x1UL << 7U)
#define TIM_DIER_UIE           (0x1UL << 0U)
#define TIM_SR_UIF             (0x1UL << 0U)

/* --- MÁSCARAS Y BITS (USART) --- */

#define USART_CR1_UE           (0x1UL << 13U)
#define USART_CR1_TE           (0x1UL << 3U)
#define USART_CR1_RE           (0x1UL << 2U)
#define USART_SR_TXE           (0x1UL << 7U)
#define USART_SR_RXNE          (0x1UL << 5U)

/* --- NVIC --- */
#define TIM2_IRQn              28
#define NVIC_EnableIRQ(IRQn)   NVIC->ISER[(uint32_t)(IRQn) >> 5U] = (uint32_t)(1UL << ((uint32_t)(IRQn) & 0x1FU))

/* --- PROTOTIPOS --- */

void USER_SystemClock_Config(void);
void USER_GPIO_Init(void);
void USER_ADC_Init(void);
void USER_ADC_Calibration(void);
uint16_t USER_ADC_Read(void);
void USER_PWM_Init(void);
void USER_TIM2_Init(void);

#endif /* MAIN_H */
