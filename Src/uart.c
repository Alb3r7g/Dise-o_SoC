#include "uart.h"
#include "main.h"

void USER_USART2_Init(void) {
    // Enable GPIOA and USART2 clock
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    // Configure PA2 as Alternate Function Push-Pull (TX) (max 50MHz: MODE=11, CNF=10)
    GPIOA->CRL &= ~(GPIO_CRL_MODE2 | GPIO_CRL_CNF2);
    GPIOA->CRL |= (GPIO_CRL_MODE2_1 | GPIO_CRL_MODE2_0 | GPIO_CRL_CNF2_1);

    // Configure PA3 as Input Floating (RX) (MODE=00, CNF=01)
    GPIOA->CRL &= ~(GPIO_CRL_MODE3 | GPIO_CRL_CNF3);
    GPIOA->CRL |= GPIO_CRL_CNF3_0;

    // Set baud rate to 115200
    // Asumiendo que APB1 = 32 MHz (mitad de 64MHz)
    USART2->BRR = 32000000 / 115200;

    // Enable USART2, Transmitter, Receiver
    USART2->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

int __io_putchar(int ch) {
    while (!(USART2->SR & USART_SR_TXE));
    USART2->DR = (ch & 0xFF);
    return ch;
}

int __io_getchar(void) {
    while (!(USART2->SR & USART_SR_RXNE));
    return (int)(USART2->DR & 0xFF);
}
