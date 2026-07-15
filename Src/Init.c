#include "Init.h"
#include "main.h"

// Variable global requerida por FreeRTOSConfig.h para definir la frecuencia de
// CPU
uint32_t SystemCoreClock = 8000000U;

// Aquí configuramos el reloj del sistema para operar a 64 MHz usando el PLL
void USER_SystemClock_Config(void) {
  // Aquí configuramos 2 ciclos de espera en la memoria Flash para frecuencias
  // mayores a 48 MHz
  FLASH_R->ACR &= ~(0x7UL << 0U);
  FLASH_R->ACR |= (0x2UL << 0U);

  // Aquí seleccionamos HSI dividido por 2 como entrada del PLL y configuramos
  // los divisores APB1 y APB2
  RCC->CFGR &= ~(0x1UL << 16U);
  RCC->CFGR &= ~(0x7UL << 11U);
  RCC->CFGR &= ~(0x7UL << 8U);
  RCC->CFGR |= (0x4UL << 8U);

  // Aquí establecemos el multiplicador del PLL a 16 para obtener 64 MHz a
  // partir de los 4 MHz de entrada
  RCC->CFGR &= ~(0xFUL << 18U);
  RCC->CFGR |= (0xFUL << 18U);

  // Aquí encendemos el PLL
  RCC->CR |= (0x1UL << 24U);

  // Aquí esperamos a que el PLL se estabilice por completo
  while (!(RCC->CR & (0x1UL << 25U)))
    ;

  // Aquí seleccionamos el PLL como la fuente del reloj del sistema
  RCC->CFGR &= ~(0x3UL << 0U);
  RCC->CFGR |= (0x2UL << 0U);

  // Aquí esperamos a que el sistema confirme el cambio de reloj al PLL
  while ((RCC->CFGR & 0xCUL) != 0x8UL)
    ;

  SystemCoreClock = 64000000U;
}

// Aquí habilitamos los relojes de los periféricos utilizados en el proyecto
void USER_RCC_Init(void) {
  // Habilitamos los relojes de GPIOA, GPIOB y AFIO
  RCC->APB2ENR |= (1UL << 2U);
  RCC->APB2ENR |= (1UL << 3U);
  RCC->APB2ENR |= (1UL << 0U);

  // Habilitamos el reloj del temporizador TIM3
  RCC->APB1ENR |= (1UL << 1U);

  // Habilitamos el reloj del periférico USART2
  RCC->APB1ENR |= (1UL << 17U);

  // Habilitamos el reloj del convertidor ADC1
  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

  // Aquí configuramos el divisor del reloj del ADC a 6 para no superar la
  // frecuencia máxima de 14 MHz
  RCC->CFGR &= ~(0x3UL << 14U);
  RCC->CFGR |= (0x2UL << 14U);
}

// Aquí configuramos las funciones y modos de los pines GPIO
void USER_GPIO_Init(void) {
  // Configuramos el pin PA6 como salida de función alterna para PWM (canal 1 de
  // TIM3)
  GPIOA->CRL &= ~(0xFUL << 24U);
  GPIOA->CRL |= (0xBUL << 24U);

  // Configuramos el pin PA7 como salida de función alterna para PWM (canal 2 de
  // TIM3)
  GPIOA->CRL &= ~(0xFUL << 28U);
  GPIOA->CRL |= (0xBUL << 28U);

  // Configuramos el pin PA2 en modo de salida alterna para TX de USART2
  GPIOA->CRL &= ~(0xFUL << 8U);
  GPIOA->CRL |= (0xBUL << 8U);

  // Configuramos el pin PA8 en modo de entrada con resistencia de pull-up para
  // el encoder 1
  GPIOA->CRH &= ~(0xFUL << 0U);
  GPIOA->CRH |= (0x8UL << 0U);
  GPIOA->ODR |= (1UL << 8U);

  // Configuramos el pin PB9 en modo de entrada con resistencia de pull-up para
  // el encoder 2
  GPIOB->CRH &= ~(0xFUL << 4U);
  GPIOB->CRH |= (0x8UL << 4U);
  GPIOB->ODR |= (1UL << 9U);

  // Configuramos los pines PB5, PB6, PB7 y PB8 como salidas digitales para el
  // control de sentido de los motores 1 y 2
  GPIOB->CRL &= ~(0xFUL << 20U);
  GPIOB->CRL |= (0x1UL << 20U);

  GPIOB->CRL &= ~(0xFUL << 24U);
  GPIOB->CRL |= (0x1UL << 24U);

  GPIOB->CRL &= ~(0xFUL << 28U);
  GPIOB->CRL |= (0x1UL << 28U);

  GPIOB->CRH &= ~(0xFUL << 0U);
  GPIOB->CRH |= (0x1UL << 0U);

  // Configuramos el pin PA0 en modo analógico para la lectura del potenciómetro
  GPIOA->CRL &= ~(0xFUL << 0U);

  // Configuramos el pin PA4 en modo de entrada con resistencia de pull-up para
  // el botón de freno
  GPIOA->CRL &= ~(0xFUL << 16U);
  GPIOA->CRL |= (0x8UL << 16U);
  GPIOA->ODR |= (1UL << 4U);

  // Configuramos los pines PB0 y PB1 como salidas de función alterna para PWM
  // (canales 3 y 4 de TIM3)
  GPIOB->CRL &= ~(0xFFUL << 0U);
  GPIOB->CRL |= (0xBBUL << 0U);

  // Configuramos los pines PB10, PB11, PB12 y PB13 como salidas digitales para
  // el control de sentido de los motores 3 y 4
  GPIOB->CRH &= ~(0xFFFFUL << 8U);
  GPIOB->CRH |= (0x1111UL << 8U);

  // Configuramos los pines PB14 y PB15 en modo de entrada con resistencia de
  // pull-up para los encoders de los motores 3 y 4
  GPIOB->CRH &= ~(0xFFUL << 24U);
  GPIOB->CRH |= (0x88UL << 24U);
  GPIOB->ODR |= (1UL << 14U) | (1UL << 15U);
}

// Aquí inicializamos el temporizador TIM3 en modo PWM para controlar la
// velocidad de los 4 motores
void USER_PWM_Init(void) {
  // Configuramos el prescaler para obtener una frecuencia de conteo de 1 MHz
  TIM3->PSC = 64 - 1;

  // Establecemos el valor de auto-reload en 1000 para definir el periodo del
  // PWM en 1 ms
  TIM3->ARR = 1000;

  // Configuramos el canal 1 de TIM3 en modo PWM 1 con soporte de precarga
  TIM3->CCMR1 &= ~(0x7UL << 4U);
  TIM3->CCMR1 |= (0x6UL << 4U);
  TIM3->CCMR1 |= (0x1UL << 3U);

  // Configuramos el canal 2 de TIM3 en modo PWM 1 con soporte de precarga
  TIM3->CCMR1 &= ~(0x7UL << 12U);
  TIM3->CCMR1 |= (0x6UL << 12U);
  TIM3->CCMR1 |= (0x1UL << 11U);

  // Habilitamos la salida física de los canales 1 y 2
  TIM3->CCER |= (0x1UL << 0U);
  TIM3->CCER |= (0x1UL << 4U);

  // Configuramos el canal 3 de TIM3 en modo PWM 1 con soporte de precarga
  TIM3->CCMR2 &= ~(0x7UL << 4U);
  TIM3->CCMR2 |= (0x6UL << 4U);
  TIM3->CCMR2 |= (0x1UL << 3U);

  // Configuramos el canal 4 de TIM3 en modo PWM 1 con soporte de precarga
  TIM3->CCMR2 &= ~(0x7UL << 12U);
  TIM3->CCMR2 |= (0x6UL << 12U);
  TIM3->CCMR2 |= (0x1UL << 11U);

  // Habilitamos la salida física de los canales 3 y 4
  TIM3->CCER |= (1UL << 8U);
  TIM3->CCER |= (1UL << 12U);

  // Inicializamos el duty cycle de los 4 canales en 0
  TIM3->CCR1 = 0;
  TIM3->CCR2 = 0;
  TIM3->CCR3 = 0;
  TIM3->CCR4 = 0;

  // Activamos la precarga del registro auto-reload y generamos un evento para
  // actualizar los registros
  TIM3->CR1 |= (0x1UL << 7U);
  TIM3->EGR |= (0x1UL << 0U);

  // Arrancamos el conteo del temporizador TIM3
  TIM3->CR1 |= (0x1UL << 0U);
}

// Aquí configuramos las interrupciones externas (EXTI) para las lecturas de los
// encoders
void USER_EXTI_Init(void) {
  // Limpiamos los bits de configuración para EXTI8 y EXTI9 en el registro
  // AFIO_EXTICR3
  AFIO->EXTICR3 &= ~(0xFFUL << 0U);

  // Mapeamos la línea de interrupción EXTI9 al pin PB9
  AFIO->EXTICR3 |= (0x1UL << 4U);

  // Mapeamos las líneas EXTI14 y EXTI15 a los pines PB14 y PB15 en el registro
  // AFIO_EXTICR4
  AFIO->EXTICR4 &= ~(0xFFUL << 8U);
  AFIO->EXTICR4 |= (0x11UL << 8U);

  // Habilitamos la máscara de interrupción para las líneas EXTI8, EXTI9, EXTI14
  // y EXTI15
  EXTI->IMR |= (1UL << 8U) | (1UL << 9U) | (1UL << 14U) | (1UL << 15U);

  // Configuramos el disparo de interrupción para detectar flancos de subida en
  // estas líneas
  EXTI->RTSR |= (1UL << 8U) | (1UL << 9U) | (1UL << 14U) | (1UL << 15U);

  // Habilitamos las interrupciones correspondientes en el controlador NVIC
  NVIC_ISER0 |= (1UL << 23U); // Habilita EXTI9 a EXTI5
  NVIC_ISER1 |= (1UL << 8U);  // Habilita EXTI15 a EXTI10
}

// Aquí configuramos el convertidor analógico a digital ADC1 en modo de
// conversión continua
void USER_ADC_Init(void) {
  ADC1->CR1 = 0;
  ADC1->CR2 = 0;
  ADC1->SQR1 = 0;
  ADC1->SQR3 = 0;

  // Establecemos el tiempo de muestreo para el canal del potenciómetro
  ADC1->SMPR2 |= (7UL << 0U);

  // Activamos el modo de conversión continua
  ADC1->CR2 |= ADC_CR2_CONT;

  // Encendemos el periférico ADC por primera vez
  ADC1->CR2 |= ADC_CR2_ADON;

  // Esperamos un momento para permitir la estabilización del circuito interno
  // del ADC
  for (volatile uint32_t i = 0; i < 50000; i++)
    ;

  // Iniciamos el proceso de calibración y esperamos a que finalice
  ADC1->CR2 |= ADC_CR2_CAL;
  while (ADC1->CR2 & ADC_CR2_CAL)
    ;

  // Realizamos el segundo encendido para activar la lógica de disparo del ADC
  ADC1->CR2 |= ADC_CR2_ADON;

  // Iniciamos la conversión analógica continua por software
  ADC1->CR2 |= ADC_CR2_SWSTART;
}
