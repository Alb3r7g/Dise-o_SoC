#include "uart.h"
#include "main.h"

// USART1 en PA9 (TX) y PA10 (RX)
// USART2 (PA2/PA3) está conectado al ST-Link en la Nucleo → conflicto de bus
// USART1 es libre y no tiene interferencia del ST-Link

void USER_USART2_Init(void) {
    // Habilitar relojes: GPIOA, AFIO y USART1 (está en APB2)
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN | RCC_APB2ENR_USART1EN;

    // Configure PA9 como Alternate Function Push-Pull (TX) (max 50MHz: MODE=11, CNF=10)
    // PA9 está en CRH bits [7:4] (pin 9 → (pin-8)*4 = 4)
    GPIOA->CRH &= ~(GPIO_CRH_MODE9 | GPIO_CRH_CNF9);
    GPIOA->CRH |= (GPIO_CRH_MODE9_1 | GPIO_CRH_MODE9_0 | GPIO_CRH_CNF9_1);

    // Configure PA10 como Input Floating (RX) (MODE=00, CNF=01)
    GPIOA->CRH &= ~(GPIO_CRH_MODE10 | GPIO_CRH_CNF10);
    GPIOA->CRH |= GPIO_CRH_CNF10_0;

    // Set baud rate a 115200
    // USART1 está en APB2 = 64 MHz (misma frecuencia que SYSCLK, sin prescaler)
    // USARTDIV = 64000000/(16*115200) = 34.722
    // Mantisa=34(0x22), Fracción=0.722*16=11.5≈12(0xC) -> BRR = 0x022C
    USART1->BRR = 0x022C;

    // Habilitar USART1, Transmisor y Receptor
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
