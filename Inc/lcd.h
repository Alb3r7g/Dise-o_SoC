#ifndef INC_LCD_H_
#define INC_LCD_H_

#include <stdint.h>
#include "main.h"

// Aquí definimos los pines de control y datos del LCD mapeados al Puerto C
#define LCD_RS_PIN_HIGH       ( 0x1UL <<  6U ) // Pin RS en nivel alto (PC6)
#define LCD_RW_PIN_HIGH       ( 0x1UL <<  7U ) // Pin RW en nivel alto (PC7)
#define LCD_EN_PIN_HIGH       ( 0x1UL <<  8U ) // Pin EN en nivel alto (PC8)

#define LCD_RS_PIN_LOW        ( 0x1UL << 22U ) // Pin RS en nivel bajo (PC6)
#define LCD_RW_PIN_LOW 	      ( 0x1UL << 23U ) // Pin RW en nivel bajo (PC7)
#define LCD_EN_PIN_LOW 	      ( 0x1UL << 24U ) // Pin EN en nivel bajo (PC8)

#define LCD_D4_PIN_HIGH       ( 0x1UL <<  9U ) // Pin de datos D4 en nivel alto (PC9)
#define LCD_D5_PIN_HIGH       ( 0x1UL << 10U ) // Pin de datos D5 en nivel alto (PC10)
#define LCD_D6_PIN_HIGH       ( 0x1UL << 11U ) // Pin de datos D6 en nivel alto (PC11)
#define LCD_D7_PIN_HIGH       ( 0x1UL << 12U ) // Pin de datos D7 en nivel alto (PC12)

#define LCD_D4_PIN_LOW        ( 0x1UL << 25U ) // Pin de datos D4 en nivel bajo (PC9)
#define LCD_D5_PIN_LOW 	      ( 0x1UL << 26U ) // Pin de datos D5 en nivel bajo (PC10)
#define LCD_D6_PIN_LOW 	      ( 0x1UL << 27U ) // Pin de datos D6 en nivel bajo (PC11)
#define LCD_D7_PIN_LOW 	      ( 0x1UL << 28U ) // Pin de datos D7 en nivel bajo (PC12)

// Aquí definimos las macros de comandos estándar para la pantalla LCD
#define LCD_Clear( )          LCD_Write_Cmd( 0x01U ) // Borra toda la pantalla
#define LCD_Display_ON( )     LCD_Write_Cmd( 0x0EU ) // Enciende el LCD
#define LCD_Display_OFF( )    LCD_Write_Cmd( 0x08U ) // Apaga el LCD
#define LCD_Cursor_Home( )    LCD_Write_Cmd( 0x02U ) // Retorna el cursor a la posición inicial
#define LCD_Cursor_Blink( )   LCD_Write_Cmd( 0x0FU ) // Hace parpadear el cursor
#define LCD_Cursor_ON( )      LCD_Write_Cmd( 0x0EU ) // Activa la visibilidad del cursor
#define LCD_Cursor_OFF( )     LCD_Write_Cmd( 0x0CU ) // Desactiva la visibilidad del cursor

// Aquí declaramos las funciones para el control del LCD y manejo de retardos
void LCD_Write_Byte(uint8_t val);
void LCD_Write_Cmd(uint8_t val);
void LCD_Put_Char(uint8_t c);
void LCD_Init(void);
void LCD_Set_Cursor(uint8_t line, uint8_t column);
void LCD_Put_Str(char * str);
void LCD_Put_Num(int32_t num);
char LCD_Busy(void);
void LCD_Pulse_EN(void);

void delay_us(uint32_t us);
void delay_ms(uint32_t ms);

#endif /* INC_LCD_H_ */