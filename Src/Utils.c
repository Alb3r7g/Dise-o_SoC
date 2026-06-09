#include "Utils.h"
#include "main.h"

// Aquí configuramos la inicialización del periférico USART2 para telemetría secundaria o debug

void USER_USART2_Init(void)
{
    // Aquí calculamos los baudios a 9600 con un reloj de periféricos APB1 a 32 MHz
    USART2->BRR = 0x0D05;

    // Aquí habilitamos el transmisor del USART2
    USART2->CR1 |= (1UL << 3U);

    // Aquí activamos el periférico USART2
    USART2->CR1 |= (1UL << 13U);
}

// Aquí transmitimos un caracter individual a través de USART2

void USER_USART2_SendChar(char c)
{
    // Esperamos a que el registro de transmisión esté vacío
    while(!(USART2->SR & (1UL << 7U)));

    // Escribimos el caracter en el registro de datos
    USART2->DR = c;
}

// Aquí transmitimos una cadena completa de caracteres a través de USART2

void USER_USART2_SendString(char *str)
{
    while(*str)
    {
        USER_USART2_SendChar(*str++);
    }
}

// Aquí convertimos un número entero a su representación en formato de texto

void USER_IntToStr(uint32_t num, char *str)
{
    int i = 0;
    int j;
    char temp;

    // Si el número es cero, retornamos de inmediato con '0'
    if(num == 0)
    {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    // Extraemos los dígitos de derecha a izquierda
    while(num > 0)
    {
        str[i++] = (num % 10) + '0';
        num /= 10;
    }

    str[i] = '\0';

    // Invertimos el orden del texto obtenido para representarlo correctamente
    for(j = 0; j < i / 2; j++)
    {
        temp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = temp;
    }
}

// Aquí realizamos la lectura del valor analógico desde el convertidor ADC1

uint16_t USER_ADC_Read(void)
{
    // Esperamos a que la conversión de datos termine
    while(!(ADC1->SR & ADC_SR_EOC));

    // Limpiamos la bandera de fin de conversión
    ADC1->SR &= ~ADC_SR_EOC;

    // Retornamos el valor digital leído
    return (uint16_t)ADC1->DR;
}

// Aquí implementamos un retardo por software en milisegundos para operaciones iniciales

void USER_Delay_ms(uint32_t ms)
{
    for(uint32_t i = 0; i < ms; i++)
    {
        for(volatile uint32_t j = 0; j < 8000; j++);
    }
}
