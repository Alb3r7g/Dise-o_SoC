#include "uart.h"
#include "main.h"

// Aquí configuramos la inicialización del periférico USART1

void USER_USART1_Init(void)
{
    // Aquí habilitamos los relojes de GPIOA, AFIO y USART1
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN | RCC_APB2ENR_USART1EN;

    // Aquí configuramos el pin PA9 en modo de salida alterna push-pull para TX
    GPIOA->CRH &= ~(0xFUL << 4U);
    GPIOA->CRH |=  (0xBUL << 4U);

    // Aquí configuramos el pin PA10 en modo de entrada flotante para RX
    GPIOA->CRH &= ~(0xFUL << 8U);
    GPIOA->CRH |=  (0x4UL << 8U);

    // Aquí establecemos los baudios a 9600 con un reloj del sistema de 64 MHz
    USART1->BRR = 0x1A0B;

    // Aquí habilitamos el USART1, la transmisión, la recepción y la interrupción por RXNE
    USART1->CR1 = (1UL << 13U) | (1UL << 5U) | (1UL << 3U) | (1UL << 2U);

    // Aquí habilitamos la interrupción del USART1 en el controlador NVIC
    NVIC_ISER1 |= (1UL << 5U);
}

// Aquí transmitimos un caracter individual a través de USART1

void USER_USART1_SendChar(char c)
{
    while(!(USART1->SR & (1UL << 7U)));
    USART1->DR = (uint8_t)c;
}

// Aquí transmitimos una cadena completa de caracteres a través de USART1

void USER_USART1_SendString(const char *str)
{
    while(*str)
    {
        USER_USART1_SendChar(*str++);
    }
}

// Aquí recibimos un caracter desde USART1 si está disponible

int USER_USART1_ReceiveChar(void)
{
    // Aquí verificamos si hay un dato disponible en el registro de recepción
    if(USART1->SR & (1UL << 5U))
    {
        return (int)(USART1->DR & 0xFF);
    }
    return -1; // Retorna -1 si no hay datos
}
