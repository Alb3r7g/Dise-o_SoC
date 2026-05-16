#ifndef INC_LCD_H_
#define INC_LCD_H_

#include <stdint.h>
#include "main.h"

// --- NUEVO: Definiciones de pines para el LCD (Puerto C) ---
#define LCD_RS_PIN_HIGH       ( 0x1UL <<  6U )//	Set pin RS_LCD (PC6)
#define LCD_RW_PIN_HIGH       ( 0x1UL <<  7U )//	Set pin RW_LCD (PC7)
#define LCD_EN_PIN_HIGH       ( 0x1UL <<  8U )//	Set pin EN_LCD (PC8)

#define LCD_RS_PIN_LOW        ( 0x1UL << 22U )//	Reset pin RS_LCD (PC6)
#define LCD_RW_PIN_LOW 	      ( 0x1UL << 23U )//	Reset pin RW_LCD (PC7)
#define LCD_EN_PIN_LOW 	      ( 0x1UL << 24U )//	Reset pin EN_LCD (PC8)

#define LCD_D4_PIN_HIGH       ( 0x1UL <<  9U )//	Set pin DATA4_LCD (PC9)
#define LCD_D5_PIN_HIGH       ( 0x1UL << 10U )//	Set pin DATA5_LCD (PC10)
#define LCD_D6_PIN_HIGH       ( 0x1UL << 11U )//	Set pin DATA6_LCD (PC11)
#define LCD_D7_PIN_HIGH       ( 0x1UL << 12U )//	Set pin DATA7_LCD (PC12)

#define LCD_D4_PIN_LOW        ( 0x1UL << 25U )//	Reset pin DATA4_LCD (PC9)
#define LCD_D5_PIN_LOW 	      ( 0x1UL << 26U )//	Reset pin DATA5_LCD (PC10)
#define LCD_D6_PIN_LOW 	      ( 0x1UL << 27U )//	Reset pin DATA6_LCD (PC11)
#define LCD_D7_PIN_LOW 	      ( 0x1UL << 28U )//	Reset pin DATA7_LCD (PC12)

// --- NUEVO: Comandos del LCD ---
#define LCD_Clear( )          LCD_Write_Cmd( 0x01U )//	Borra la pantalla
#define LCD_Display_ON( )     LCD_Write_Cmd( 0x0EU )//	Pantalla LCD activa
#define LCD_Display_OFF( )    LCD_Write_Cmd( 0x08U )//	Pantalla LCD inactiva
#define LCD_Cursor_Home( )    LCD_Write_Cmd( 0x02U )//	Establecer el cursor a 'Home'
#define LCD_Cursor_Blink( )   LCD_Write_Cmd( 0x0FU )//	Cursor intermitente
#define LCD_Cursor_ON( )      LCD_Write_Cmd( 0x0EU )//	Cursor visible activo
#define LCD_Cursor_OFF( )     LCD_Write_Cmd( 0x0CU )//	Cursor inactivo

// --- NUEVO: Prototipos de funciones ---
void LCD_Write_Byte(uint8_t val);
void LCD_Write_Cmd(uint8_t val);
void LCD_Put_Char(uint8_t c);
void LCD_Init(void);
void LCD_Set_Cursor(uint8_t line, uint8_t column);
void LCD_Put_Str(char * str);
void LCD_Put_Num(int16_t num);
char LCD_Busy(void);
void LCD_Pulse_EN(void);

// --- NUEVO: Prototipos para Delays Universales ---
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);

#endif /* INC_LCD_H_ */
