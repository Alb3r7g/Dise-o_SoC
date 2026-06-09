#include "Motores.h"
#include "main.h"

// Aquí configuramos el avance de los motores en sentido de marcha adelante

void USER_Motor_Forward(void) {
  // Aquí activamos el canal directo y desactivamos el canal inverso para el motor 1
  GPIOB->BSRR = (1UL << 6U);
  GPIOB->BRR = (1UL << 5U);
}

void USER_Motor2_Forward(void) {
  // Aquí activamos el canal directo y desactivamos el canal inverso para el motor 2
  GPIOB->BSRR = (1UL << 8U);
  GPIOB->BRR = (1UL << 7U);
}

void USER_Motor3_Forward(void) {
  // Aquí activamos el canal directo y desactivamos el canal inverso para el motor 3
  GPIOB->BSRR = (1UL << 13U);
  GPIOB->BRR = (1UL << 12U);
}

void USER_Motor4_Forward(void) {
  // Aquí activamos el canal directo y desactivamos el canal inverso para el motor 4
  GPIOB->BSRR = (1UL << 10U);
  GPIOB->BRR = (1UL << 11U);
}

// Aquí detenemos la marcha de todos los motores

void USER_Motor_Stop(void) {
  // Aquí desactivamos todos los pines de control para detener los motores
  GPIOB->BRR = (1UL << 5U);
  GPIOB->BRR = (1UL << 6U);
  GPIOB->BRR = (1UL << 7U);
  GPIOB->BRR = (1UL << 8U);
  GPIOB->BRR = (1UL << 10U);
  GPIOB->BRR = (1UL << 11U);
  GPIOB->BRR = (1UL << 12U);
  GPIOB->BRR = (1UL << 13U);
}