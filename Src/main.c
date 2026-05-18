#include "main.h"
#include "lcd.h"
#include "uart.h"
#include "EngTrModel.h"
#include <stdio.h>

volatile uint8_t flag_40ms = 0;

int main(void) {
    uint16_t dataADC;

    // Inicializaciones
    USER_SystemClock_Config();
    USER_GPIO_Init();
    USER_PWM_Init();
    USER_ADC_Init();
    USER_ADC_Calibration();
    USER_USART2_Init();
    USER_TIM2_Init();
    
    LCD_Init();
    LCD_Clear();
    LCD_Set_Cursor(1, 1);
    LCD_Put_Str("Tractor Reto");

    EngTrModel_initialize();

    // Iniciar el ADC
    ADC1->CR2 |= ADC_CR2_ADON;

    // --- TEST DIRECTO DE UART ---
    const char *test_msg = "TEST_UART1_OK\r\n";
    for (int i = 0; test_msg[i] != '\0'; i++) {
        while (!(USART1->SR & USART_SR_TXE));
        USART1->DR = test_msg[i];
    }

    static uint8_t lcd_counter = 0;

    while (1) {
        // Ejecución cada 40 ms
        if (flag_40ms) {
            flag_40ms = 0;

            // --- Leer Sensores (síncrono con el modelo) ---
            ADC1->CR2 |= (0x1UL << 22U); // SWSTART
            dataADC = USER_ADC_Read();
            EngTrModel_U.Throttle = ((double)dataADC / 4095.0) * 100.0;

            // Leer Freno (PC13) - Pull-Up (Lógica negativa al presionar)
            if (!(GPIOC->IDR & GPIO_IDR_IDR13)) {
                EngTrModel_U.BrakeTorque = 15000.0; // Torque de frenado fuerte
            } else {
                EngTrModel_U.BrakeTorque = 0.0;
            }

            // --- Avanzar el paso del modelo ---
            EngTrModel_step();

            // --- Controlador de Ralentí (Idle) ---
            // Evita que el motor caiga por debajo de 800 RPM y que el vehículo retroceda
            if (EngTrModel_DW.DiscreteTimeIntegrator_DSTATE < 100.0) {
                EngTrModel_DW.DiscreteTimeIntegrator_DSTATE = 100.0;
            }
            if (EngTrModel_DW.WheelSpeed_DSTATE < 0.0) {
                EngTrModel_DW.WheelSpeed_DSTATE = 0.0;
            }

            // Topar la velocidad del vehículo a 120 mph
            if (EngTrModel_Y.VehicleSpeed > 120.0) {
                EngTrModel_Y.VehicleSpeed = 120.0;
            }

            // --- Escalar Velocidad para PWM de LEDs ---
            double v_speed = EngTrModel_Y.VehicleSpeed;
            if (v_speed < 0.0) v_speed = 0.0;

            // Velocidad entre 0-120 mph para rango 0-4095
            uint32_t ccr_val = (uint32_t)(v_speed * (4095.0 / 120.0));
            if (ccr_val > 4095) ccr_val = 4095;

            // Aplicar PWM a los 4 canales
            TIM3->CCR1 = ccr_val;
            TIM3->CCR2 = ccr_val;
            TIM3->CCR3 = ccr_val;
            TIM3->CCR4 = ccr_val;

            // --- Actualizar LCD cada 200 ms (cada 5 ticks de 40 ms) ---
            lcd_counter++;
            if (lcd_counter >= 5) {
                lcd_counter = 0;
                char lcd_buf[17];

                // Línea 1: A:<Throttle> V:<VehicleSpeed> G:<Gear>
                LCD_Set_Cursor(1, 1);
                sprintf(lcd_buf, "A:%-3d V:%-3d G:%-1d", 
                        (int)EngTrModel_U.Throttle, 
                        (int)EngTrModel_Y.VehicleSpeed, 
                        (int)EngTrModel_Y.Gear);
                LCD_Put_Str(lcd_buf);

                // Línea 2: RPM:<EngineSpeed>
                LCD_Set_Cursor(2, 1);
                sprintf(lcd_buf, "RPM:%-4d        ", (int)EngTrModel_Y.EngineSpeed);
                LCD_Put_Str(lcd_buf);
            }

            // --- Transmitir por UART1 al ESP32 (cada 40 ms) ---
            {
                char uart_buf[32];
                int rpm   = (int)EngTrModel_Y.EngineSpeed;
                int vel   = (int)(EngTrModel_Y.VehicleSpeed * 100.0);
                int gear  = (int)EngTrModel_Y.Gear;
                int vel_i = vel / 100;
                int vel_d = vel % 100;
                int n = sprintf(uart_buf, "%d,%d.%02d,%d\r\n", rpm, vel_i, vel_d, gear);
                for (int i = 0; i < n; i++) {
                    while (!(USART1->SR & USART_SR_TXE));
                    USART1->DR = (uint8_t)uart_buf[i];
                }
            }

        }
    }
}

// Manejador de la Interrupción del TIM2
void TIM2_IRQHandler(void) {
    if (TIM2->SR & TIM_SR_UIF) {
        TIM2->SR &= ~TIM_SR_UIF; // Limpiar bandera
        flag_40ms = 1;
    }
}

// Inicialización del Timer 2 para 40ms
void USER_TIM2_Init(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    // Prescaler: Frecuencia Timer = 64MHz. 64000 -> 1 ms por tick
    TIM2->PSC = 64000 - 1;
    // Auto-reload para 40 ms
    TIM2->ARR = 40 - 1;

    // Activar interrupción
    TIM2->DIER |= TIM_DIER_UIE;

    // Habilitar en NVIC
    NVIC_EnableIRQ(TIM2_IRQn);

    // Arrancar timer
    TIM2->CR1 |= TIM_CR1_CEN;
}

// Funciones de Configuración Base
void USER_SystemClock_Config(void) {
    RCC->CR |= (0x1UL << 16U);
    while (!(RCC->CR & (0x1UL << 17U))); 
    RCC->CFGR &= ~((0xFUL << 18U) | (0x1UL << 16U));
    RCC->CFGR |= (0x6UL << 18U) | (0x1UL << 16U); // PLLMUL x8 = 64MHz
    RCC->CR |= (0x1UL << 24U);
    while (!(RCC->CR & (0x1UL << 25U))); 
    FLASH->ACR &= ~(0x7UL << 0U);
    FLASH->ACR |= (0x2UL << 0U);
    RCC->CFGR &= ~(0x3UL << 0U);
    RCC->CFGR |= (0x2UL << 0U); // SW PLL
    while ((RCC->CFGR & (0x3UL << 2U)) != (0x2UL << 2U));
    
    // APB1 Prescaler /2 para no exceder 36MHz (quedará a 32MHz)
    // Nota: El reloj de los timers en APB1 se multiplica por 2 (queda en 64MHz)
    RCC->CFGR &= ~(0x7UL << 8U);
    RCC->CFGR |=  (0x4UL << 8U); 
}

void USER_GPIO_Init(void)
{
    // Habilitar reloj para GPIOC (PC13 Freno) y GPIOA (PA0 Potenciómetro)
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN | RCC_APB2ENR_IOPAEN;

    // PC13 como Input with Pull-up
    GPIOC->CRH &= ~(GPIO_CRH_MODE13 | GPIO_CRH_CNF13);
    GPIOC->CRH |= GPIO_CRH_CNF13_1; // Input with pull-up/down
    GPIOC->ODR |= GPIO_ODR_ODR13;   // Pull-up

    // PA0 como Analog Input
    GPIOA->CRL &= ~(GPIO_CRL_MODE0 | GPIO_CRL_CNF0);
}

void USER_PWM_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    /* PA6 TIM3_CH1, PA7 TIM3_CH2 */
    GPIOA->CRL &= ~(0xFUL << 24U | 0xFUL << 28U);
    GPIOA->CRL |=  (0xBUL << 24U | 0xBUL << 28U);

    /* PB0 TIM3_CH3, PB1 TIM3_CH4 */
    GPIOB->CRL &= ~(0xFUL << 0U | 0xFUL << 4U);
    GPIOB->CRL |=  (0xBUL << 0U | 0xBUL << 4U);

    /* Prescaler */
    TIM3->PSC = 8 - 1;
    /* PWM resolution (0 - 4095) */
    TIM3->ARR = 4095;

    /* PWM mode CH1-CH4 */
    TIM3->CCMR1 &= ~(0xFUL << 4U | 0xFUL << 12U);
    TIM3->CCMR1 |= (0x6UL << 4U | 0x6UL << 12U);
    TIM3->CCMR2 &= ~(0xFUL << 4U | 0xFUL << 12U);
    TIM3->CCMR2 |= (0x6UL << 4U | 0x6UL << 12U);

    TIM3->CCMR1 |= (0x1UL << 3U) | (0x1UL << 11U);
    TIM3->CCMR2 |= (0x1UL << 3U) | (0x1UL << 11U);

    TIM3->CCER |= (0x1UL << 0U) | (0x1UL << 4U) | (0x1UL << 8U) | (0x1UL << 12U);

    TIM3->CCR1 = 0; TIM3->CCR2 = 0; TIM3->CCR3 = 0; TIM3->CCR4 = 0;

    TIM3->CR1 |= (0x1UL << 7U);
    TIM3->EGR |= (0x1UL << 0U);
    TIM3->CR1 |= (0x1UL << 0U);
}

void USER_ADC_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    RCC->CFGR |= RCC_CFGR_ADCPRE; 

    ADC1->CR1 &= ~ADC_CR1_DUALMOD;
    ADC1->CR2 &= ~ADC_CR2_ALIGN;
    ADC1->CR2 |= ADC_CR2_CONT;
    ADC1->SMPR2 &= ~ADC_SMPR2_SMP0;
    ADC1->SQR1 &= ~ADC_SQR1_L;
    ADC1->SQR3 &= ~ADC_SQR3_SQ1;
    ADC1->CR2 |= ADC_CR2_ADON;
    for(volatile int i = 0; i < 10000; i++);
}

void USER_ADC_Calibration(void)
{
    ADC1->CR2 |= ADC_CR2_CAL;
    while(ADC1->CR2 & ADC_CR2_CAL);
}

uint16_t USER_ADC_Read(void)
{
    while(!(ADC1->SR & ADC_SR_EOC));
    ADC1->SR &= ~ADC_SR_EOC;
    return (uint16_t)ADC1->DR;
}
