#include "main.h"
#include "EngTrModel.h"
#include "lcd.h"
#include <stdint.h>
#include <stdio.h>
#include "Init.h"
#include "Motores.h"
#include "Utils.h"
#include "uart.h"

// Librerías de FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

// Estructuras de datos para la comunicación inter-tareas (IPC)

// Aquí definimos la estructura del comando remoto enviado desde la ESP32
typedef struct {
    uint8_t  remote_control;
    uint16_t remote_throttle;
    uint8_t  remote_brake;
} CommandData_t;

// Aquí definimos la estructura de salida del modelo físico
typedef struct {
    float    engineSpeed;
    float    vehicleSpeed;
    float    gear;
    float    throttle;
    uint32_t setpointRPM;
} ModelOutput_t;

// Aquí declaramos los manejadores (handles) de colas y grupos de eventos
static QueueHandle_t      xQueueModelOutput = NULL;  // Canal de datos del modelo hacia motores, TX y LCD
static QueueHandle_t      xQueueRawCmd      = NULL;  // Canal para comandos sin procesar desde la interrupción
static QueueHandle_t      xQueueValidCmd    = NULL;  // Canal para comandos validados hacia el modelo
static EventGroupHandle_t xEventGroup       = NULL;  // Grupo de eventos para sincronización

#define EVT_BIT_NEW_CMD   (1 << 0)

// Aquí definimos los contadores de pulsos de los encoders compartidos con las interrupciones
static volatile uint32_t s_encoderPulses[4] = {0, 0, 0, 0};

// Declaraciones de las funciones de tarea
static void Task_Model(void *pv);
static void Task_CommTx(void *pv);
static void Task_CommRx(void *pv);
static void Task_MotorControl(void *pv);
static void Task_LCD(void *pv);

// Punto de entrada principal del sistema
int main(void)
{
    // Aquí configuramos el reloj principal del sistema a 64 MHz
    USER_SystemClock_Config();

    // Aquí inicializamos los periféricos de hardware
    USER_RCC_Init();
    USER_GPIO_Init();
    USER_PWM_Init();
    USER_USART2_Init();
    USER_USART1_Init();
    USER_ADC_Init();
    USER_EXTI_Init();

    // Aquí inicializamos el modelo de transmisión automática
    EngTrModel_initialize();

    // Aquí inicializamos y limpiamos la pantalla LCD
    LCD_Init();
    LCD_Clear();

    // Aquí activamos la marcha adelante inicial de los 4 motores
    USER_Motor_Forward();
    USER_Motor2_Forward();
    USER_Motor3_Forward();
    USER_Motor4_Forward();

    USER_USART2_SendString("FreeRTOS Init...\r\n");

    // Aquí configuramos las prioridades de interrupción en el NVIC compatibles con FreeRTOS
    volatile uint8_t *nvic_ipr = (volatile uint8_t *)0xE000E400;
    nvic_ipr[28] = 0xF0; // Prioridad 15 para TIM2 (reloj de tick de FreeRTOS)
    nvic_ipr[37] = 0x50; // Prioridad 5 para USART1 (recepción de datos rápida)
    nvic_ipr[23] = 0x60; // Prioridad 6 para EXTI9_5
    nvic_ipr[40] = 0x60; // Prioridad 6 para EXTI15_10

    // Aquí creamos las colas y grupos de eventos para la comunicación
    xQueueModelOutput = xQueueCreate(1, sizeof(ModelOutput_t));
    xQueueRawCmd      = xQueueCreate(1, sizeof(CommandData_t));
    xQueueValidCmd    = xQueueCreate(1, sizeof(CommandData_t));
    xEventGroup       = xEventGroupCreate();

    // Aquí creamos las tareas con prioridades asignadas por Rate Monotonic Scheduling (RMS)
    xTaskCreate(Task_Model,        "Model",   384, NULL, 5, NULL);
    xTaskCreate(Task_CommRx,       "CommRx",  128, NULL, 4, NULL);
    xTaskCreate(Task_MotorControl, "MotCtl",  256, NULL, 3, NULL);
    xTaskCreate(Task_CommTx,       "CommTx",  256, NULL, 2, NULL);
    xTaskCreate(Task_LCD,          "LCD",     256, NULL, 1, NULL);

    // Aquí iniciamos el planificador de FreeRTOS
    vTaskStartScheduler();

    // Bucle infinito en caso de que falle el inicio por falta de memoria heap
    for (;;) {}
}

// Aquí implementamos la tarea que procesa el paso del modelo físico del vehículo
static void Task_Model(void *pv)
{
    (void)pv;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(40);

    CommandData_t cmd = {0, 0, 0};
    ModelOutput_t output;

    uint16_t adcValue   = USER_ADC_Read();
    uint16_t adcControl = adcValue;

    for (;;)
    {
        // Aquí verificamos si hay un comando remoto nuevo sin bloquear la ejecución
        EventBits_t bits = xEventGroupWaitBits(xEventGroup, EVT_BIT_NEW_CMD,
                                                pdTRUE, pdFALSE, 0);
        if (bits & EVT_BIT_NEW_CMD)
        {
            xQueueReceive(xQueueValidCmd, &cmd, 0);
        }

        // Aquí leemos el valor analógico del potenciómetro
        adcValue = USER_ADC_Read();

        // Aqui determinamos los valores de entrada para el acelerador y el freno del modelo
        if (!(GPIOA->IDR & (1UL << 4U)) || (cmd.remote_control && cmd.remote_brake))
        {
            // Freno activo: aplicamos el torque máximo de frenado y ponemos el acelerador a cero
            EngTrModel_U.BrakeTorque = 150000.0;
            EngTrModel_U.Throttle    = 0.0;
            adcControl = 0;
        }
        else
        {
            // Freno inactivo: seleccionamos la entrada del acelerador (potenciómetro o control remoto)
            EngTrModel_U.BrakeTorque = 0.0;

            if (cmd.remote_control)
            {
                adcControl = (cmd.remote_throttle * 4095) / 100;
            }
            else
            {
                adcControl = adcValue;
            }
            EngTrModel_U.Throttle = ((double)adcControl / 4095.0) * 100.0;
        }

        // Aquí ejecutamos un paso de cálculo en el modelo de transmisión
        EngTrModel_step();

        // Aquí limitamos los estados internos del modelo para prevenir inconsistencias físicas
        if (EngTrModel_DW.DiscreteTimeIntegrator_DSTATE < 1.0)
            EngTrModel_DW.DiscreteTimeIntegrator_DSTATE = 1.0;
        if (EngTrModel_DW.WheelSpeed_DSTATE < 0.0)
            EngTrModel_DW.WheelSpeed_DSTATE = 0.0;

        // Aquí estructuramos los datos calculados para compartirlos con otras tareas
        output.engineSpeed  = (float)EngTrModel_Y.EngineSpeed;
        output.vehicleSpeed = (float)EngTrModel_Y.VehicleSpeed;
        output.gear         = (float)EngTrModel_Y.Gear;
        output.throttle     = (float)EngTrModel_U.Throttle;
        output.setpointRPM  = (uint32_t)EngTrModel_Y.EngineSpeed / 12;
        if (output.setpointRPM < 5)
            output.setpointRPM = 0;

        // Aquí enviamos la información actualizando la cola con el dato más reciente
        xQueueOverwrite(xQueueModelOutput, &output);

        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

// Aquí implementamos la tarea que transmite periódicamente la telemetría a la ESP32
static void Task_CommTx(void *pv)
{
    (void)pv;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    // Definimos un intervalo de 500 ms para evitar saturar el canal de comunicación
    const TickType_t xPeriod = pdMS_TO_TICKS(500);

    ModelOutput_t output;
    char uart_buf[64];

    for (;;)
    {
        // Consultamos el dato de la cola sin consumirlo para permitir el acceso a otras tareas
        if (xQueuePeek(xQueueModelOutput, &output, 0) == pdTRUE)
        {
            int rpm_modelo = (int)output.engineSpeed;
            int vel_raw    = (int)(output.vehicleSpeed * 100.0f);
            int gear       = (int)output.gear;
            int vel_i      = vel_raw / 100;
            int vel_d      = vel_raw % 100;
            if (vel_d < 0) vel_d = -vel_d;

            sprintf(uart_buf, "%d,%d.%02d,%d\r\n", rpm_modelo, vel_i, vel_d, gear);
            USER_USART1_SendString(uart_buf);
        }

        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

// Aquí implementamos la tarea que recibe y valida los comandos de control remoto
static void Task_CommRx(void *pv)
{
    (void)pv;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(50);

    CommandData_t rawCmd;

    for (;;)
    {
        // Leemos la cola de comandos recibidos desde la interrupción sin bloquear
        if (xQueueReceive(xQueueRawCmd, &rawCmd, 0) == pdTRUE)
        {
            // Aquí validamos los rangos permitidos para el estado y el nivel de aceleración
            if (rawCmd.remote_throttle > 100)
                rawCmd.remote_throttle = 100;
            if (rawCmd.remote_control > 1)
                rawCmd.remote_control = 1;

            // Aquí guardamos el comando procesado en la cola del modelo
            xQueueOverwrite(xQueueValidCmd, &rawCmd);

            // Aquí activamos el bit de evento para notificar al modelo la presencia de un comando
            xEventGroupSetBits(xEventGroup, EVT_BIT_NEW_CMD);
        }

        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

// Aquí implementamos la tarea del lazo de control PID para la velocidad de los motores
static void Task_MotorControl(void *pv)
{
    (void)pv;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(50);

    ModelOutput_t output;

    uint32_t rpm[4];
    uint32_t pulses[4];
    int32_t  error[4];
    int32_t  controlPWM[4];
    int32_t  integral[4] = {0, 0, 0, 0};

    float Kp = 1.0f;
    float Ki = 0.2f;

    // Buffer de depuración deshabilitado para evitar advertencias de compilación

    for (;;)
    {
        // Aquí leemos y reiniciamos los contadores de encoders protegiendo la zona crítica
        taskENTER_CRITICAL();
        for (int i = 0; i < 4; i++)
        {
            pulses[i] = s_encoderPulses[i];
            s_encoderPulses[i] = 0;
        }
        taskEXIT_CRITICAL();

        // Aquí calculamos las RPM de los motores con base en los pulsos recibidos
        for (int i = 0; i < 4; i++)
        {
            rpm[i] = pulses[i] * 60;
            if (rpm[i] > 800) rpm[i] = 800;
        }

        // Aquí leemos la velocidad de referencia provista por el modelo físico
        uint32_t setpointRPM = 0;
        if (xQueuePeek(xQueueModelOutput, &output, 0) == pdTRUE)
        {
            setpointRPM = output.setpointRPM;
        }

        // Aquí calculamos el error de velocidad por cada rueda
        for (int i = 0; i < 4; i++)
        {
            error[i] = (int32_t)setpointRPM - (int32_t)rpm[i];
        }

        if (setpointRPM == 0)
        {
            for (int i = 0; i < 4; i++)
            {
                controlPWM[i] = 0;
                integral[i]   = 0;
            }
        }
        else
        {
            for (int i = 0; i < 4; i++)
            {
                integral[i] += error[i];
                if (integral[i] >  3000) integral[i] =  3000;
                if (integral[i] < -500)  integral[i] = -500;

                int32_t feedforward = 200 + (int32_t)setpointRPM;
                controlPWM[i] = feedforward
                              + (int32_t)(Kp * (float)error[i])
                              + (int32_t)(Ki * (float)integral[i]);
            }
        }

        // Aquí limitamos los valores de PWM al rango operativo de 0 a 1000
        for (int i = 0; i < 4; i++)
        {
            if (controlPWM[i] < 0)    controlPWM[i] = 0;
            if (controlPWM[i] > 1000) controlPWM[i] = 1000;
        }

        // Aquí actualizamos los registros de comparación del TIM3 con el ciclo calculado
        TIM3->CCR1 = (uint32_t)controlPWM[0];
        TIM3->CCR2 = (uint32_t)controlPWM[1];
        TIM3->CCR3 = (uint32_t)controlPWM[2];
        TIM3->CCR4 = (uint32_t)controlPWM[3];

        // Los mensajes de depuración por USART2 permanecen inactivos para evitar demoras en la ejecución
        /*
        USER_USART2_SendString("SP:");
        USER_IntToStr(setpointRPM, buffer);
        USER_USART2_SendString(buffer);

        USER_USART2_SendString(" R1:");
        USER_IntToStr(rpm[0], buffer);
        USER_USART2_SendString(buffer);

        USER_USART2_SendString(" R2:");
        USER_IntToStr(rpm[1], buffer);
        USER_USART2_SendString(buffer);

        USER_USART2_SendString(" R3:");
        USER_IntToStr(rpm[2], buffer);
        USER_USART2_SendString(buffer);

        USER_USART2_SendString(" R4:");
        USER_IntToStr(rpm[3], buffer);
        USER_USART2_SendString(buffer);

        USER_USART2_SendString(" W1:");
        USER_IntToStr((uint32_t)controlPWM[0], buffer);
        USER_USART2_SendString(buffer);

        USER_USART2_SendString(" W2:");
        USER_IntToStr((uint32_t)controlPWM[1], buffer);
        USER_USART2_SendString(buffer);

        USER_USART2_SendString(" W3:");
        USER_IntToStr((uint32_t)controlPWM[2], buffer);
        USER_USART2_SendString(buffer);

        USER_USART2_SendString(" W4:");
        USER_IntToStr((uint32_t)controlPWM[3], buffer);
        USER_USART2_SendString(buffer);

        USER_USART2_SendString("\r\n");
        */

        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

// Aquí implementamos la tarea encargada de actualizar la pantalla LCD
static void Task_LCD(void *pv)
{
    (void)pv;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(500);

    ModelOutput_t output;

    for (;;)
    {
        if (xQueuePeek(xQueueModelOutput, &output, 0) == pdTRUE)
        {
            // Escribimos el acelerador, la velocidad y la marcha en la primera línea
            LCD_Set_Cursor(1, 1);
            LCD_Put_Str("A:              ");
            LCD_Set_Cursor(1, 3);
            LCD_Put_Num((int32_t)output.throttle);
            LCD_Set_Cursor(1, 7);
            LCD_Put_Str("V:");
            LCD_Put_Num((int32_t)output.vehicleSpeed);
            LCD_Set_Cursor(1, 14);
            LCD_Put_Str("G:");
            LCD_Put_Num((int32_t)output.gear);

            // Escribimos las RPM del motor en la segunda línea
            LCD_Set_Cursor(2, 1);
            LCD_Put_Str("RPM:            ");
            LCD_Set_Cursor(2, 5);
            LCD_Put_Num((int32_t)output.engineSpeed);
        }

        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

// Manejo de las interrupciones del sistema (ISRs)

// Rutinas de servicio para las interrupciones de los encoders

void EXTI9_5_IRQHandler(void)
{
    // Aquí procesamos la interrupción externa de la línea 8
    if (EXTI->PR & (1UL << 8U))
    {
        for (volatile int i = 0; i < 160; i++); // Retardo simple para mitigar rebotes mecánicos a 64 MHz
        if (GPIOA->IDR & (1UL << 8U))
        {
            s_encoderPulses[0]++;
        }
        // Aquí limpiamos la bandera de interrupción en el registro EXTI escribiendo un uno lógico
        EXTI->PR = (1UL << 8U);
    }

    // Aquí procesamos la interrupción externa de la línea 9
    if (EXTI->PR & (1UL << 9U))
    {
        for (volatile int i = 0; i < 160; i++);
        if (GPIOB->IDR & (1UL << 9U))
        {
            s_encoderPulses[1]++;
        }
        EXTI->PR = (1UL << 9U);
    }
}

void EXTI15_10_IRQHandler(void)
{
    // Aquí procesamos la interrupción externa de la línea 14
    if (EXTI->PR & (1UL << 14U))
    {
        for (volatile int i = 0; i < 160; i++);
        if (GPIOB->IDR & (1UL << 14U))
        {
            s_encoderPulses[2]++;
        }
        EXTI->PR = (1UL << 14U);
    }

    // Aquí procesamos la interrupción externa de la línea 15
    if (EXTI->PR & (1UL << 15U))
    {
        for (volatile int i = 0; i < 160; i++);
        if (GPIOB->IDR & (1UL << 15U))
        {
            s_encoderPulses[3]++;
        }
        EXTI->PR = (1UL << 15U);
    }
}

// Rutina de servicio de la interrupción de recepción del USART1
void USART1_IRQHandler(void)
{
    if (USART1->SR & ((1UL << 5U) | (1UL << 3U)))
    {
        char rx_c = (char)(USART1->DR & 0xFF);

        static char rx_buf[32];
        static int  rx_idx = 0;

        if (rx_c == '\n' || rx_idx >= 31)
        {
            rx_buf[rx_idx] = '\0';

            // Aquí procesamos el formato esperado del comando de control
            if (rx_buf[0] == 'C' && rx_buf[1] == ':')
            {
                int idx = 2;
                int st  = 0;
                int acc = 0;

                while (rx_buf[idx] >= '0' && rx_buf[idx] <= '9')
                {
                    st = st * 10 + (rx_buf[idx] - '0');
                    idx++;
                }

                if (rx_buf[idx] == ',')
                {
                    idx++;
                    while (rx_buf[idx] >= '0' && rx_buf[idx] <= '9')
                    {
                        acc = acc * 10 + (rx_buf[idx] - '0');
                        idx++;
                    }

                    // Aquí colocamos el comando en la cola de recepción desde la interrupción
                    CommandData_t cmd;
                    cmd.remote_control  = (uint8_t)st;
                    cmd.remote_throttle = (uint16_t)acc;
                    cmd.remote_brake    = 0;

                    // Aqui parseamos el tercer campo opcional (freno remoto)
                    if (rx_buf[idx] == ',')
                    {
                        idx++;
                        int brk = 0;
                        while (rx_buf[idx] >= '0' && rx_buf[idx] <= '9')
                        {
                            brk = brk * 10 + (rx_buf[idx] - '0');
                            idx++;
                        }
                        cmd.remote_brake = (uint8_t)brk;
                    }
                    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                    xQueueOverwriteFromISR(xQueueRawCmd, &cmd,
                                           &xHigherPriorityTaskWoken);
                    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
                }
            }
            rx_idx = 0;
        }
        else if (rx_c != '\r')
        {
            rx_buf[rx_idx++] = rx_c;
        }
    }
}
