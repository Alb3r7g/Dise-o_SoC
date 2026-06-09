#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

// Aquí configuramos el temporizador TIM2 para generar las interrupciones del tick de FreeRTOS

void vPortSetupTimerInterrupt(void)
{
    // Aquí habilitamos el reloj del periférico TIM2
    RCC->APB1ENR |= (1UL << 0U);

    // Aquí configuramos el prescaler, el auto-reload y los registros del TIM2 para operar a 1 kHz
    TIM2->PSC  = 64U - 1U;
    TIM2->ARR  = 1000U - 1U;
    TIM2->DIER |= (1UL << 0U);   // Activamos la interrupción por actualización
    TIM2->EGR  |= (1UL << 0U);   // Generamos un evento de actualización
    TIM2->SR   &= ~(1UL << 0U);  // Limpiamos la bandera UIF
    TIM2->CR1  |= (1UL << 0U);   // Encendemos el temporizador

    // Aquí habilitamos la interrupción de TIM2 en el NVIC
    NVIC_ISER0 |= (1UL << 28U);
}

// Aquí manejamos la interrupción periódica de TIM2 para avanzar el tick de FreeRTOS

extern void xPortSysTickHandler(void);

void TIM2_IRQHandler(void)
{
    // Aquí limpiamos la bandera de interrupción UIF del TIM2
    TIM2->SR &= ~(1UL << 0U);

    // Aquí avanzamos el tick de FreeRTOS si el planificador ya está iniciado
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        xPortSysTickHandler();
    }
}

// Aquí proveemos la memoria estática necesaria para la tarea inactiva (Idle Task)

static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t  xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t  **ppxIdleTaskStackBuffer,
                                   uint32_t      *pulIdleTaskStackSize)
{
    // Asignamos los buffers estáticos creados para la Idle Task
    *ppxIdleTaskTCBBuffer   = &xIdleTaskTCBBuffer;
    *ppxIdleTaskStackBuffer = &xIdleStack[0];
    *pulIdleTaskStackSize   = configMINIMAL_STACK_SIZE;
}

// Aquí definimos los ganchos de depuración en caso de fallos del sistema

// Este gancho se ejecuta si la asignación dinámica en la pila falla
void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();
    for (;;) {}
}

// Este gancho se ejecuta si alguna tarea desborda su stack asignado
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    taskDISABLE_INTERRUPTS();
    for (;;) {}
}
