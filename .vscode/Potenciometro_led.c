#include "main.h"
#include "EngTrModel.h"
#include "lcd.h"
#include <stdint.h>

/* ================================================================ */
/* ====================== VARIABLES ================================ */
/* ================================================================ */

volatile uint32_t encoderPulses = 0;

/* ================================================================ */
/* ====================== PROTOTYPES =============================== */
/* ================================================================ */

void USER_RCC_Init(void);

void USER_GPIO_Init(void);

void USER_PWM_Init(void);

void USER_USART2_Init(void);

void USER_EXTI_Init(void);

void USER_Motor_Forward(void);

void USER_Motor_Stop(void);

void USER_USART2_SendChar(char c);

void USER_USART2_SendString(char *str);

void USER_ADC_Init(void);

uint16_t USER_ADC_Read(void);

void USER_IntToStr(uint32_t num, char *str);

void USER_Delay_ms(uint32_t ms);

/* ================================================================ */
/* ============================ MAIN =============================== */
/* ================================================================ */

int main(void)
{
    uint32_t rpm;

    uint32_t pulses;

    uint16_t adcValue;

    uint16_t adcControl = 0;

    uint32_t contador = 0;

    uint32_t contadorModelo = 0;

    uint32_t setpointRPM;

    int32_t error;

    int32_t controlPWM;

    float Kp = 2.0f;

    /* ================= INIT ================= */

    USER_RCC_Init();

    USER_GPIO_Init();

    USER_PWM_Init();

    USER_USART2_Init();

    USER_ADC_Init();

    USER_EXTI_Init();

    EngTrModel_initialize();
    
    LCD_Init();

    LCD_Clear();

    uint32_t rpmModelo = 0;

    /* ================= MOTOR ON ================= */

    USER_Motor_Forward();

    adcValue = USER_ADC_Read();

    adcControl = adcValue;

    while(1)
    {  
    adcValue = USER_ADC_Read();

    if(!(GPIOA->IDR & (1UL << 4U)))
    {
        if(adcControl > 50)
        {
            adcControl -= 50;
        }
        else
        {
            adcControl = 0;
        }
    }
    else
    {
        adcControl = adcValue;
    }

    // ADC -> PWM /

    USER_Delay_ms(10);

    contador += 10;
    contadorModelo += 10;

    if(contadorModelo >= 40)
    {
        contadorModelo = 0;

        EngTrModel_U.Throttle =
            ((double)adcControl / 4095.0) * 100.0;

        if(!(GPIOA->IDR & (1UL << 4U)))
        {
            EngTrModel_U.BrakeTorque = 15000.0;
        }
        else
        {
            EngTrModel_U.BrakeTorque = 0.0;
        }

        EngTrModel_step();

        if (EngTrModel_DW.DiscreteTimeIntegrator_DSTATE < 1.0)
        {
            EngTrModel_DW.DiscreteTimeIntegrator_DSTATE = 1.0;
        }

        if (EngTrModel_DW.WheelSpeed_DSTATE < 0.0)
        {
            EngTrModel_DW.WheelSpeed_DSTATE = 0.0;
        }

        rpmModelo =
            (uint32_t)EngTrModel_Y.EngineSpeed;
    }
    
    if(contador >= 500)
    {
        contador = 0;

        pulses = encoderPulses;

        encoderPulses = 0;

        rpm = pulses * 6;

        // RPM deseadas desde potenciometro /

        setpointRPM = rpmModelo / 16;

        if (setpointRPM < 5)
        {
            setpointRPM = 0;
        }

        // error /

        error = (int32_t)setpointRPM - (int32_t)rpm;

       
        /*if(setpointRPM == 0)
        {
            controlPWM = 0;
        }
        else
        {
            controlPWM = 500 + (int32_t)(Kp * error);
        }*/

        controlPWM = 500 + (int32_t)(Kp * error);

        if(controlPWM < 0)
        {
            controlPWM = 0;
        }

        if(controlPWM > 1000)
        {
            controlPWM = 1000;
        }

        // aplicar PWM /

        TIM3->CCR1 = (uint32_t)controlPWM;
        //TIM3->CCR1 = 700;

        LCD_Set_Cursor(1,1);
        LCD_Put_Str("R:");
        LCD_Put_Num(rpm);

        LCD_Set_Cursor(1,9);
        LCD_Put_Str("P:");
        LCD_Put_Num(controlPWM);

        LCD_Set_Cursor(2,1);
        LCD_Put_Str("M:");
        LCD_Put_Num(rpmModelo);

        LCD_Set_Cursor(2,9);
        LCD_Put_Str("E:");
        LCD_Put_Num(error);

        /*LCD_Set_Cursor(2,9);
        LCD_Put_Str("G:");
        LCD_Put_Num((uint32_t)EngTrModel_Y.Gear);*/
        
    }
}
}

/* ================================================================ */
/* ====================== RCC INIT ================================ */
/* ================================================================ */

void USER_RCC_Init(void)
{
    /* GPIOA + GPIOB + GPIOC + AFIO */

    RCC->APB2ENR |= (1UL << 2U);

    RCC->APB2ENR |= (1UL << 3U);

    RCC->APB2ENR |= (1UL << 4U);

    RCC->APB2ENR |= (1UL << 0U);

    /* TIM3 */

    RCC->APB1ENR |= (1UL << 1U);

    /* USART2 */

    RCC->APB1ENR |= (1UL << 17U);

    /* ADC1 */

    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
}

/* ================================================================ */
/* ====================== GPIO INIT =============================== */
/* ================================================================ */

void USER_GPIO_Init(void)
{
    /* ================================================= */
    /* ================= PA6 PWM ======================= */
    /* ================================================= */

    GPIOA->CRL &= ~(0xFUL << 24U);

    GPIOA->CRL |=  (0xBUL << 24U);

    /* ================================================= */
    /* ================= PA2 USART2 TX ================= */
    /* ================================================= */

    GPIOA->CRL &= ~(0xFUL << 8U);

    GPIOA->CRL |=  (0xBUL << 8U);

    /* ================================================= */
    /* ================= PA8 ENCODER =================== */
    /* ================================================= */

    GPIOA->CRH &= ~(0xFUL << 0U);

    GPIOA->CRH |=  (0x8UL << 0U);

    /* pull-up */

    GPIOA->ODR |= (1UL << 8U);

    /* ================================================= */
    /* ================= PB5 OUTPUT ==================== */
    /* ================================================= */

    GPIOB->CRL &= ~(0xFUL << 20U);

    GPIOB->CRL |=  (0x1UL << 20U);

    /* ================================================= */
    /* ================= PB6 OUTPUT ==================== */
    /* ================================================= */

    GPIOB->CRL &= ~(0xFUL << 24U);

    GPIOB->CRL |=  (0x1UL << 24U);

    /* PA0 -> ADC */

    GPIOA->CRL &= ~(0xFUL << 0U);

    /* PA4 -> boton freno */

    GPIOA->CRL &= ~(0xFUL << 16U);
    GPIOA->CRL |=  (0x8UL << 16U);

    /* pull-up */

    GPIOA->ODR |= (1UL << 4U);
}

/* ================================================================ */
/* ====================== PWM INIT ================================ */
/* ================================================================ */

void USER_PWM_Init(void)
{
    /* prescaler */

    TIM3->PSC = 8 - 1;

    /* periodo */

    TIM3->ARR = 1000;

    /* PWM mode CH1 */

    TIM3->CCMR1 &= ~(0x7UL << 4U);

    TIM3->CCMR1 |=  (0x6UL << 4U);

    /* preload */

    TIM3->CCMR1 |= (0x1UL << 3U);

    /* enable CH1 */

    TIM3->CCER |= (0x1UL << 0U);

    /* duty inicial */

    TIM3->CCR1 = 0;

    /* auto reload */

    TIM3->CR1 |= (0x1UL << 7U);

    TIM3->EGR |= (0x1UL << 0U);

    /* start timer */

    TIM3->CR1 |= (0x1UL << 0U);
}

/* ================================================================ */
/* ====================== USART2 INIT ============================= */
/* ================================================================ */

void USER_USART2_Init(void)
{
    /*
       9600 baud @ 8MHz
    */

    USART2->BRR = 0x341;

    /* transmitter enable */

    USART2->CR1 |= (1UL << 3U);

    /* USART enable */

    USART2->CR1 |= (1UL << 13U);
}

/* ================================================================ */
/* ====================== USART SEND CHAR ========================= */
/* ================================================================ */

void USER_USART2_SendChar(char c)
{
    while(!(USART2->SR & (1UL << 7U)));

    USART2->DR = c;
}

/* ================================================================ */
/* ====================== USART SEND STRING ======================= */
/* ================================================================ */

void USER_USART2_SendString(char *str)
{
    while(*str)
    {
        USER_USART2_SendChar(*str++);

    }
}

/* ================================================================ */
/* ====================== EXTI INIT =============================== */
/* ================================================================ */

void USER_EXTI_Init(void)
{
    /*
       PA8 -> EXTI8
    */

    AFIO->EXTICR3 &= ~(0xFUL << 0U);

    /* unmask */

    EXTI->IMR |= (1UL << 8U);

    /* rising edge */

    EXTI->RTSR |= (1UL << 8U);

    /* NVIC enable */

    NVIC_ISER0 |= (1UL << 23U);
}

/* ================================================================ */
/* ====================== INTERRUPT =============================== */
/* ================================================================ */

void EXTI9_5_IRQHandler(void)
{
    if(EXTI->PR & (1UL << 8U))
    {
        encoderPulses++;

        /* clear pending */

        EXTI->PR |= (1UL << 8U);
    }
}

/* ================================================================ */
/* ====================== MOTOR FORWARD =========================== */
/* ================================================================ */

void USER_Motor_Forward(void)
{
    GPIOB->BSRR = (1UL << 5U);

    GPIOB->BRR  = (1UL << 6U);
}

/* ================================================================ */
/* ====================== MOTOR STOP ============================== */
/* ================================================================ */

void USER_Motor_Stop(void)
{
    GPIOB->BRR = (1UL << 5U);

    GPIOB->BRR = (1UL << 6U);
}

/* ================================================================ */
/* ====================== INT TO STRING =========================== */
/* ================================================================ */

void USER_IntToStr(uint32_t num, char *str)
{
    int i = 0;

    int j;

    char temp;

    if(num == 0)
    {
        str[0] = '0';

        str[1] = '\0';

        return;
    }

    while(num > 0)
    {
        str[i++] = (num % 10) + '0';

        num /= 10;
    }

    str[i] = '\0';

    for(j = 0; j < i / 2; j++)
    {
        temp = str[j];

        str[j] = str[i - j - 1];

        str[i - j - 1] = temp;
    }
}

/* ================================================================ */
/* ====================== ADC INIT ================================ */
/* ================================================================ */

void USER_ADC_Init(void)
{
    ADC1->CR1 = 0;

    ADC1->CR2 = 0;

    ADC1->SQR1 = 0;
    ADC1->SQR3 = 0;

    ADC1->SMPR2 |= (7UL << 0U);

    /* modo continuo */
    ADC1->CR2 |= ADC_CR2_CONT;

    /* encender ADC */
    ADC1->CR2 |= ADC_CR2_ADON;

    for(volatile uint32_t i=0;i<50000;i++);

    /* calibrar */
    ADC1->CR2 |= ADC_CR2_CAL;

    while(ADC1->CR2 & ADC_CR2_CAL);

    /* segundo ADON */
    ADC1->CR2 |= ADC_CR2_ADON;

    /* iniciar conversion continua */
    ADC1->CR2 |= ADC_CR2_SWSTART;
}

/* ================================================================ */
/* ====================== ADC READ ================================ */
/* ================================================================ */

uint16_t USER_ADC_Read(void)
{
    

    while(!(ADC1->SR & ADC_SR_EOC));

    ADC1->SR &= ~ADC_SR_EOC;

    return (uint16_t)ADC1->DR;
}

/* ================================================================ */
/* ====================== DELAY ================================== */
/* ================================================================ */

void USER_Delay_ms(uint32_t ms)
{
    for(uint32_t i = 0; i < ms; i++)
    {
        for(uint32_t j = 0; j < 1000; j++);
    }
}