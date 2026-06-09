#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>

// Aquí definimos la estructura para el control del periférico RCC (Reloj)
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

// Aquí definimos la estructura para el control de los puertos GPIO
typedef struct{
	volatile uint32_t CRL;
	volatile uint32_t CRH;
	volatile uint32_t IDR;
	volatile uint32_t ODR;
	volatile uint32_t BSRR;
	volatile uint32_t BRR;
	volatile uint32_t LCKR;
} GPIO_TypeDef;

// Aquí definimos la estructura para los temporizadores (TIM)
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

// Aquí definimos la estructura para los periféricos USART (Comunicación serial)
typedef struct{
	volatile uint32_t SR;
	volatile uint32_t DR;
	volatile uint32_t BRR;
	volatile uint32_t CR1;
	volatile uint32_t CR2;
	volatile uint32_t CR3;
	volatile uint32_t GTPR;
} USART_TypeDef;

// Aquí definimos la estructura para el control de interrupciones externas (EXTI)
typedef struct{
	volatile uint32_t IMR;
	volatile uint32_t EMR;
	volatile uint32_t RTSR;
	volatile uint32_t FTSR;
	volatile uint32_t SWIER;
	volatile uint32_t PR;
} EXTI_TypeDef;

// Aquí definimos la estructura para el mapeo de funciones alternas (AFIO)
typedef struct{
	volatile uint32_t EVCR;
	volatile uint32_t MAPR;
	volatile uint32_t EXTICR1;
	volatile uint32_t EXTICR2;
	volatile uint32_t EXTICR3;
	volatile uint32_t EXTICR4;
	volatile uint32_t MAPR2;
} AFIO_TypeDef;

// Aquí definimos la estructura para el convertidor analógico a digital (ADC)
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

// Aquí definimos la estructura para el controlador de memoria Flash
typedef struct{
	volatile uint32_t ACR;
	volatile uint32_t KEYR;
	volatile uint32_t OPTKEYR;
	volatile uint32_t SR;
	volatile uint32_t CR;
	volatile uint32_t AR;
	volatile uint32_t RESERVED;
	volatile uint32_t OBR;
	volatile uint32_t WRPR;
} FLASH_TypeDef;

// Aquí definimos las direcciones base de memoria para los periféricos utilizados
#define RCC_BASE        0x40021000UL
#define GPIOA_BASE      0x40010800UL
#define GPIOB_BASE      0x40010C00UL
#define GPIOC_BASE      0x40011000UL
#define TIM2_BASE       0x40000000UL
#define TIM3_BASE       0x40000400UL
#define USART1_BASE     0x40013800UL
#define USART2_BASE     0x40004400UL
#define EXTI_BASE       0x40010400UL
#define AFIO_BASE       0x40010000UL
#define ADC1_BASE       0x40012400UL
#define FLASH_BASE_R    0x40022000UL

// Aquí asociamos los punteros de control a las direcciones base de los periféricos
#define RCC             ((RCC_TypeDef*) RCC_BASE)
#define GPIOA           ((GPIO_TypeDef*) GPIOA_BASE)
#define GPIOB           ((GPIO_TypeDef*) GPIOB_BASE)
#define GPIOC 			((GPIO_TypeDef*) GPIOC_BASE)
#define TIM2            ((TIM_TypeDef*) TIM2_BASE)
#define TIM3            ((TIM_TypeDef*) TIM3_BASE)
#define USART1          ((USART_TypeDef*) USART1_BASE)
#define USART2          ((USART_TypeDef*) USART2_BASE)
#define EXTI            ((EXTI_TypeDef*) EXTI_BASE)
#define AFIO            ((AFIO_TypeDef*) AFIO_BASE)
#define ADC1            ((ADC_TypeDef*) ADC1_BASE)
#define FLASH_R         ((FLASH_TypeDef*) FLASH_BASE_R)

// Aquí definimos las macros para el acceso directo a los registros del NVIC
#define NVIC_ISER0      (*(volatile uint32_t*)0xE000E100)
#define NVIC_ISER1      (*(volatile uint32_t*)0xE000E104)

// Aquí definimos los bits de control para la configuración del RCC
#define RCC_APB2ENR_AFIOEN      (0x1UL << 0U)
#define RCC_APB2ENR_IOPAEN      (0x1UL << 2U)
#define RCC_APB2ENR_IOPBEN      (0x1UL << 3U)
#define RCC_APB2ENR_IOPCEN      (0x1UL << 4U)
#define RCC_APB2ENR_ADC1EN      (0x1UL << 9U)

#define RCC_APB1ENR_TIM3EN      (0x1UL << 1U)
#define RCC_APB1ENR_USART2EN    (0x1UL << 17U)

#define RCC_APB2ENR_USART1EN    (0x1UL << 14U)

// Aquí definimos los bits de configuración y estado para el ADC
#define ADC_CR2_ADON            (0x1UL << 0U)
#define ADC_CR2_SWSTART         (0x1UL << 22U)
#define ADC_SR_EOC              (0x1UL << 1U)
#define ADC_CR2_CONT      (1UL << 1U)
#define ADC_CR2_CAL       (1UL << 2U)

#endif