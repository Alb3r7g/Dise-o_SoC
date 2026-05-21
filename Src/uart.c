#include "uart.h"
#include "main.h"

void USER_USART1_Init(void) {
    // Habilitamos los relojes para GPIOA, AFIO y USART1
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN | RCC_APB2ENR_USART1EN;

    // Configuramos el pin PA9 como salida alternativa Push-Pull para TX
    GPIOA->CRH &= ~(GPIO_CRH_MODE9 | GPIO_CRH_CNF9);
    GPIOA->CRH |= (GPIO_CRH_MODE9_1 | GPIO_CRH_MODE9_0 | GPIO_CRH_CNF9_1);

    // Configuramos el pin PA10 como entrada flotante para RX
    GPIOA->CRH &= ~(GPIO_CRH_MODE10 | GPIO_CRH_CNF10);
    GPIOA->CRH |= GPIO_CRH_CNF10_0;

    // Establecemos la velocidad de transmisión a 115200 baudios 
    USART1->BRR = 0x022C;

    // Habilitamos el periférico USART1, el transmisor y el receptor
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

int __io_putchar(int ch) {
    while (!(USART1->SR & USART_SR_TXE));
    USART1->DR = (ch & 0xFF);
    return ch;
}

int __io_getchar(void) {
    while (!(USART1->SR & USART_SR_RXNE));
    return (int)(USART1->DR & 0xFF);
}
