#include <stdint.h>
#include "main.h"
#include "lcd.h"


// Librerias de FreeRTOS para la proteccion de la comunicacion
#include "FreeRTOS.h"
#include "task.h"

// Aquí implementamos un retraso en microsegundos usando ciclos de reloj a 64 MHz
void delay_us(uint32_t us) {
    // Aumentamos el multiplicador a 150 para garantizar que el retardo nunca sea
    // menor al requerido, sin importar la alineacion en la memoria Flash y el prefetch
    volatile uint32_t count = us * 150; 
    while(count--);
}

// Aquí implementamos un retraso en milisegundos llamando repetidamente a delay_us
void delay_ms(uint32_t ms) {
    while(ms--) {
        delay_us(1000);
    }
}

// Aquí definimos los retardos requeridos para la pantalla LCD
void USER_TIM2_Delay_40ms(void) { delay_ms(45); } 
void USER_TIM_Delay_4_1ms(void) { delay_ms(5); }  
void USER_TIM_Delay_53us(void)  { delay_us(60); } 
void USER_TIM_Delay_100us(void) { delay_us(100); }
void USER_TIM_Delay_10us(void)  { delay_us(10); }
void USER_TIM_Delay_1ms(void)   { delay_ms(1); }

// Aquí definimos la matriz del mapa de caracteres personalizados
const int8_t UserFont[8][8] =
{
		{ 0x11, 0x0A, 0x04, 0x1B, 0x11, 0x11, 0x11, 0x0E },
		{ 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10 },
		{ 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18 },
		{ 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C },
		{ 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E },
		{ 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F },
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
};

// Aquí inicializamos la pantalla LCD en modo de 4 bits y cargamos la fuente personalizada
void LCD_Init(void){
	int8_t const *p;
    // Aquí habilitamos el reloj para el Puerto C
	RCC->APB2ENR	|=	 ( 0x1UL <<  4U );	
    // Aquí configuramos los pines PC6 a PC12 como salidas digitales de propósito general
	GPIOC->CRL		&=	~( 0x3UL << 30U ) & ~( 0x2UL << 28U ) & ~( 0x3UL << 26U ) & ~( 0x2UL << 24U );
	GPIOC->CRL 		|= 	 ( 0x1UL << 28U ) | ( 0x1UL << 24U );
	GPIOC->CRH		&=	~( 0x3UL << 18U ) & ~( 0x2UL << 16U ) & ~( 0x3UL << 14U ) & ~( 0x2UL << 12U )
					&	~( 0x3UL << 10U ) & ~( 0x2UL <<  8U ) & ~( 0x3UL <<  6U ) & ~( 0x2UL <<  4U )
					& 	~( 0x3UL <<  2U ) & ~( 0x2UL <<  0U );
	GPIOC->CRH		|= 	 ( 0x1UL << 16U ) | ( 0x1UL << 12U ) | ( 0x1UL <<  8U ) | ( 0x1UL <<  4U ) | ( 0x1UL <<  0U );

    // Aquí ejecutamos la secuencia de comandos para inicializar la pantalla en modo 4 bits
	GPIOC->BSRR	 =	 LCD_RS_PIN_LOW | LCD_RW_PIN_LOW | LCD_EN_PIN_LOW | LCD_D4_PIN_LOW | LCD_D5_PIN_LOW | LCD_D6_PIN_LOW | LCD_D7_PIN_LOW;
	USER_TIM2_Delay_40ms();
    
	GPIOC->BSRR	 =	 LCD_D4_PIN_HIGH | LCD_D5_PIN_HIGH;
	LCD_Pulse_EN();
	USER_TIM_Delay_4_1ms();

	GPIOC->BSRR	 =	 LCD_D4_PIN_HIGH | LCD_D5_PIN_HIGH;
	LCD_Pulse_EN();
	USER_TIM_Delay_53us();

	GPIOC->BSRR	 =	 LCD_D4_PIN_HIGH | LCD_D5_PIN_HIGH;
	LCD_Pulse_EN();
	delay_ms(2);

	GPIOC->BSRR	 =	 LCD_D4_PIN_LOW | LCD_D5_PIN_HIGH;
	LCD_Pulse_EN();
	delay_ms(2);

	LCD_Write_Cmd( 0x28U );
	LCD_Write_Cmd( 0x08U );
	LCD_Write_Cmd( 0x01U );
	LCD_Write_Cmd( 0x06U );
	LCD_Write_Cmd( 0x0FU );

    // Aquí cargamos los patrones personalizados en la CGRAM de la pantalla
	LCD_Write_Cmd( 0x40 );
	p = &UserFont[0][0];
	for( uint32_t i = 0; i < sizeof( UserFont ); i++, p++ ) LCD_Put_Char( *p );
	LCD_Write_Cmd( 0x80 );
}

// Aquí escribimos los 4 bits menos significativos del valor en las líneas de datos del LCD
void LCD_Out_Data4(uint8_t val){
	if( ( val & 0x01U ) == 0x01U ) GPIOC->BSRR = LCD_D4_PIN_HIGH; else GPIOC->BSRR = LCD_D4_PIN_LOW;
	if( ( val & 0x02U ) == 0x02U ) GPIOC->BSRR = LCD_D5_PIN_HIGH; else GPIOC->BSRR = LCD_D5_PIN_LOW;
	if( ( val & 0x04U ) == 0x04U ) GPIOC->BSRR = LCD_D6_PIN_HIGH; else GPIOC->BSRR = LCD_D6_PIN_LOW;
	if( ( val & 0x08U ) == 0x08U ) GPIOC->BSRR = LCD_D7_PIN_HIGH; else GPIOC->BSRR = LCD_D7_PIN_LOW;
}

// Aqui enviamos un byte completo dividiendolo en dos paquetes de 4 bits, protegiendo contra preemption
void LCD_Write_Byte(uint8_t val){
	// Deshabilitamos el cambio de contexto temporalmente para no corromper la comunicacion
	vTaskSuspendAll();

	LCD_Out_Data4( ( val >> 4 ) & 0x0FU );
	LCD_Pulse_EN( );
	LCD_Out_Data4( val & 0x0FU );
	LCD_Pulse_EN( );

	// Restauramos el cambio de contexto
	xTaskResumeAll();

	delay_us(100);
}

// Aquí enviamos un comando a la pantalla LCD
void LCD_Write_Cmd(uint8_t val){
	GPIOC->BSRR	=	LCD_RS_PIN_LOW;
	LCD_Write_Byte( val );
}

// Aquí enviamos un caracter para ser mostrado en la pantalla LCD
void LCD_Put_Char(uint8_t c){
	GPIOC->BSRR	=	LCD_RS_PIN_HIGH;
	LCD_Write_Byte( c );
}

// Aquí posicionamos el cursor en una fila y columna específicas de la pantalla
void LCD_Set_Cursor(uint8_t line, uint8_t column){
	uint8_t address;
	column--; line--;
	address = ( line * 0x40U ) + column;
	address = 0x80U + ( address & 0x7FU );
	LCD_Write_Cmd( address );
}

// Aquí escribimos una cadena de texto en la pantalla con un máximo de 16 caracteres
void LCD_Put_Str(char * str){
	for( int16_t i = 0; i < 16 && str[ i ] != 0; i++ ) LCD_Put_Char( str[ i ] );
}

// Aquí convertimos y escribimos un valor numérico entero en la pantalla
void LCD_Put_Num(int32_t num){
	if (num < 0) {
		LCD_Put_Char('-');
		num = -num;
	}
	int32_t p; int32_t f = 0; int8_t ch[ 5 ];
	for( int32_t i = 0; i < 5; i++ ){
		p = 1;
		for( int32_t j = 4 - i; j > 0; j-- ) p = p * 10;
		ch[ i ] = ( num / p );
		if( num >= p && !f ) f = 1;
		num = num - ch[ i ] * p;
		ch[ i ] = ch[ i ] + 48;
		if( f ) LCD_Put_Char( ch[ i ] );
        else if (i == 4) LCD_Put_Char('0');
	}
}

// Aquí leemos la bandera de ocupado del LCD para verificar si está listo para recibir comandos
char LCD_Busy(void){
	GPIOC->CRH	&=	~( 0x2UL << 18U ) & ~( 0x3UL << 16U );
	GPIOC->CRH	|=   ( 0x1UL << 18U );
	GPIOC->BSRR	 =	 LCD_RS_PIN_LOW | LCD_RW_PIN_HIGH | LCD_EN_PIN_HIGH;
	USER_TIM_Delay_100us();
	if(( GPIOC->IDR	& LCD_D7_PIN_HIGH )) {
		GPIOC->BSRR	= 	LCD_EN_PIN_LOW | LCD_RW_PIN_LOW;
		GPIOC->CRH	&=	~( 0x3UL << 18U ) & ~( 0x2UL << 16U );
		GPIOC->CRH	|=   ( 0x1UL << 16U );
		return 1;
	} else {
		GPIOC->BSRR	= 	LCD_EN_PIN_LOW | LCD_RW_PIN_LOW;
		GPIOC->CRH	&=	~( 0x3UL << 18U ) & ~( 0x2UL << 16U );
		GPIOC->CRH	|=   ( 0x1UL << 16U );
		return 0;
	}
}

// Aquí generamos el pulso de habilitación (Enable) en el pin correspondiente de la pantalla
void LCD_Pulse_EN(void){
	GPIOC->BSRR	=	LCD_EN_PIN_LOW;
	delay_us(10);
	GPIOC->BSRR	=	LCD_EN_PIN_HIGH;
	delay_us(10);
	GPIOC->BSRR	=	LCD_EN_PIN_LOW;
	delay_us(10);
}