#include "main.h"
#include "lcd.h"
#include "uart.h"
#include "EngTrModel.h"
#include <stdio.h>

volatile uint8_t flag_40ms = 0;

int main(void) {
    uint16_t dataADC;

    // Inicializamos los periféricos y el reloj del sistema
    USER_SystemClock_Config();
    USER_GPIO_Init();
    USER_Motor_Init();
    USER_PWM_Init();
    USER_ADC_Init();
    USER_ADC_Calibration();
    USER_USART1_Init();
    USER_TIM2_Init();
    
    LCD_Init();
    LCD_Clear();
    LCD_Set_Cursor(1, 1);
    LCD_Put_Str("Tractor Reto");

    EngTrModel_initialize();

    // Fijamos la direccion de los motores hacia adelante
    USER_Motor_Forward();

    // Iniciamos la conversión del ADC
    ADC1->CR2 |= ADC_CR2_ADON;

    // Transmitimos un mensaje de prueba inicial por UART
    const char *test_msg = "TEST_UART1_OK\r\n";
    for (int i = 0; test_msg[i] != '\0'; i++) {
        while (!(USART1->SR & USART_SR_TXE));
        USART1->DR = test_msg[i];
    }

    static uint8_t lcd_counter = 0;

    while (1) {
        // Ejecutamos un ciclo de control cada 40 ms
        if (flag_40ms) {
            flag_40ms = 0;

            // Leemos el acelerador mediante el ADC de forma síncrona con el modelo
            ADC1->CR2 |= (0x1UL << 22U); // SWSTART
            dataADC = USER_ADC_Read();
            EngTrModel_U.Throttle = ((double)dataADC / 4095.0) * 100.0;

            // Leemos el freno en el pin PC13 configurado con resistencia Pull-Up
            if (!(GPIOC->IDR & GPIO_IDR_IDR13)) {
                EngTrModel_U.BrakeTorque = 15000.0; // Aplicamos un frenado fuerte
            } else {
                EngTrModel_U.BrakeTorque = 0.0;
            }

            // Avanzamos un paso en el modelo de transmisión automática
            EngTrModel_step();

            
            // Esto evita que el motor caiga por debajo de 1 RPM y que el vehículo retroceda
            if (EngTrModel_DW.DiscreteTimeIntegrator_DSTATE < 1.0) {
                EngTrModel_DW.DiscreteTimeIntegrator_DSTATE = 1.0;
            }
            if (EngTrModel_DW.WheelSpeed_DSTATE < 0.0) {
                EngTrModel_DW.WheelSpeed_DSTATE = 0.0;
            }

            // Topamos la velocidad máxima del vehículo a 120 mph
            if (EngTrModel_Y.VehicleSpeed > 120.0) {
                EngTrModel_Y.VehicleSpeed = 120.0;
            }

            // Escalamos la velocidad para modular el PWM de los LEDs
            double v_speed = EngTrModel_Y.VehicleSpeed;
            if (v_speed < 0.0) v_speed = 0.0;

            // Mapeamos la velocidad de 0-120 al rango de resolución del PWM (0-1000)
            uint32_t ccr_val = (uint32_t)(v_speed * (1000.0 / 120.0));
            if (ccr_val > 1000) ccr_val = 1000;

            // Aplicamos la señal PWM a los 4 canales configurados
            TIM3->CCR1 = ccr_val;
            TIM3->CCR2 = ccr_val;
            TIM3->CCR3 = ccr_val;
            TIM3->CCR4 = ccr_val;

            // Actualizamos los datos en el display LCD cada 200 ms 
            lcd_counter++;
            if (lcd_counter >= 5) {
                lcd_counter = 0;
                char lcd_buf[17];

                // Línea 1 A:<Acelerador> V:<VelocidadVehiculo> G:<Marcha>
                LCD_Set_Cursor(1, 1);
                sprintf(lcd_buf, "A:%-3d V:%-3d G:%-1d", 
                        (int)EngTrModel_U.Throttle, 
                        (int)EngTrModel_Y.VehicleSpeed, 
                        (int)EngTrModel_Y.Gear);
                LCD_Put_Str(lcd_buf);

                // Línea 2 RPM:<VelocidadMotor>
                LCD_Set_Cursor(2, 1);
                sprintf(lcd_buf, "RPM:%-4d        ", (int)EngTrModel_Y.EngineSpeed);
                LCD_Put_Str(lcd_buf);
            }

            // Transmitimos los datos por UART1 hacia la ESP32 (cada 40 ms)
            // Utilizamos enteros fijos para evitar errores de compilación con flotantes
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
        TIM2->SR &= ~TIM_SR_UIF; // Limpiamos la bandera de interrupción
        flag_40ms = 1;
    }
}

// Inicializamos el Timer 2 para generar una interrupción cada 40 ms
void USER_TIM2_Init(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    // Configuramos el prescaler asumiendo una frecuencia de 64 MHz
    TIM2->PSC = 64000 - 1;
    
    // Configuramos el auto-reload para alcanzar los 40 ms
    TIM2->ARR = 40 - 1;

    // Activamos la interrupción por actualización
    TIM2->DIER |= TIM_DIER_UIE;

    // Habilitamos la interrupción en el controlador NVIC
    NVIC_EnableIRQ(TIM2_IRQn);

    // Arrancamos el temporizador
    TIM2->CR1 |= TIM_CR1_CEN;
}

// Funciones de Configuración Base
void USER_SystemClock_Config(void) {
    // Configuramos el PLL para multiplicar el reloj a 64 MHz
    RCC->CR |= (0x1UL << 16U);
    while (!(RCC->CR & (0x1UL << 17U))); 
    RCC->CFGR &= ~((0xFUL << 18U) | (0x1UL << 16U));
    RCC->CFGR |= (0x6UL << 18U) | (0x1UL << 16U); 
    RCC->CR |= (0x1UL << 24U);
    while (!(RCC->CR & (0x1UL << 25U))); 
    FLASH->ACR &= ~(0x7UL << 0U);
    FLASH->ACR |= (0x2UL << 0U);
    RCC->CFGR &= ~(0x3UL << 0U);
    RCC->CFGR |= (0x2UL << 0U);
    while ((RCC->CFGR & (0x3UL << 2U)) != (0x2UL << 2U));
    
    
    RCC->CFGR &= ~(0x7UL << 8U);
    RCC->CFGR |=  (0x4UL << 8U); 
}

void USER_GPIO_Init(void)
{
    // Habilitamos el reloj para GPIOC 
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN | RCC_APB2ENR_IOPAEN;

    // Configuramos el pin PC13 como entrada con resistencia Pull-Up
    GPIOC->CRH &= ~(GPIO_CRH_MODE13 | GPIO_CRH_CNF13);
    GPIOC->CRH |= GPIO_CRH_CNF13_1; 
    GPIOC->ODR |= GPIO_ODR_ODR13;   

    // Configuramos el pin PA0 como entrada analógica
    GPIOA->CRL &= ~(GPIO_CRL_MODE0 | GPIO_CRL_CNF0);
}

void USER_PWM_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
   
    GPIOA->CRL &= ~(0xFUL << 24U | 0xFUL << 28U);
    GPIOA->CRL |=  (0xBUL << 24U | 0xBUL << 28U);

    GPIOB->CRL &= ~(0xFUL << 0U | 0xFUL << 4U);
    GPIOB->CRL |=  (0xBUL << 0U | 0xBUL << 4U);

    TIM3->PSC = 8 - 1;

    TIM3->ARR = 1000;

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

// ============================================================
// MOTOR CONTROL (L298N) - Pines de direccion
// ============================================================

void USER_Motor_Init(void)
{
    // Habilitamos reloj GPIOB (por si no esta habilitado aun)
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

    // PB5 OUTPUT (Motor 1 IN1)
    GPIOB->CRL &= ~(0xFUL << 20U);
    GPIOB->CRL |=  (0x1UL << 20U);

    // PB6 OUTPUT (Motor 1 IN2)
    GPIOB->CRL &= ~(0xFUL << 24U);
    GPIOB->CRL |=  (0x1UL << 24U);

    // PB7 OUTPUT (Motor 2 IN1)
    GPIOB->CRL &= ~(0xFUL << 28U);
    GPIOB->CRL |=  (0x1UL << 28U);

    // PB8 OUTPUT (Motor 2 IN2)
    GPIOB->CRH &= ~(0xFUL << 0U);
    GPIOB->CRH |=  (0x1UL << 0U);

    // PB10 OUTPUT (Motor 4 IN1)
    GPIOB->CRH &= ~(0xFUL << 8U);
    GPIOB->CRH |=  (0x1UL << 8U);

    // PB11 OUTPUT (Motor 4 IN2)
    GPIOB->CRH &= ~(0xFUL << 12U);
    GPIOB->CRH |=  (0x1UL << 12U);

    // PB12 OUTPUT (Motor 3 IN1 - corregido JTAG)
    GPIOB->CRH &= ~(0xFUL << 16U);
    GPIOB->CRH |=  (0x1UL << 16U);

    // PB13 OUTPUT (Motor 3 IN2 - corregido JTAG)
    GPIOB->CRH &= ~(0xFUL << 20U);
    GPIOB->CRH |=  (0x1UL << 20U);
}

void USER_Motor_Forward(void)
{
    GPIOB->BSRR = (0x1UL << 5U);   GPIOB->BRR = (0x1UL << 6U);   // Motor 1
    GPIOB->BSRR = (0x1UL << 7U);   GPIOB->BRR = (0x1UL << 8U);   // Motor 2
    GPIOB->BSRR = (0x1UL << 12U);  GPIOB->BRR = (0x1UL << 13U);  // Motor 3
    GPIOB->BSRR = (0x1UL << 10U);  GPIOB->BRR = (0x1UL << 11U);  // Motor 4
}

void USER_Motor_Stop(void)
{
    GPIOB->BRR = (0x1UL << 5U)  | (0x1UL << 6U)  |
                 (0x1UL << 7U)  | (0x1UL << 8U)  |
                 (0x1UL << 10U) | (0x1UL << 11U) |
                 (0x1UL << 12U) | (0x1UL << 13U);
}
