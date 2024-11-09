/*
 * mo.c
 *
 *  Created on: Mar 31, 2023
 *      Author: musta
 */

/* FreeRTOS Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Drivers Includes */
#include "uart.h"
#include "BasicIO.h"
#include "LCD.h"
#include "ADC.h"

/* Prototypes */
void system_init(void);

void ReadTemp_Task(void* pvParam);
void AC_Task(void* pvParam);
void Fan_Task(void* pvParam);

void ReadUart_Task(void* pvParam);
void DataParsing_Task(void* pvParam);
void Action(void* pvParam);

/* Object Declaration */
SemaphoreHandle_t AC_Semph = NULL;
SemaphoreHandle_t Fan_Semph = NULL;
QueueHandle_t  Queue_Handler = NULL;
QueueHandle_t  Data_Queue = NULL;

int main()
{
	/* Pr. Init */
	system_init();

	/* Create OS Objects */

	AC_Semph = xSemaphoreCreateBinary();
	Fan_Semph = xSemaphoreCreateBinary();

	Queue_Handler = xQueueCreate(5, 8);

	/* Create Tasks */
	xTaskCreate(ReadUart_Task, "ReadUart_Task", 120, NULL, 1, NULL);
	xTaskCreate(DataParsing_Task, "DataParsing_Task", 140, NULL, 2, NULL);
	xTaskCreate(ReadTemp_Task, "ReadTemp_Task", 120, NULL, 3, NULL);
	xTaskCreate(AC_Task, "AC_Task", 120, NULL, 4, NULL);
	xTaskCreate(Fan_Task, "Fan_Task", 120, NULL, 5, NULL);
	//	xTaskCreate(Action, "Action", 120, NULL, 6, NULL);

	//	/* Start OS */
	vTaskStartScheduler();

	/* Unreachable Code */
	while(1)
	{
		LCD_DispStr("Error");
	}
	return 0;
}

void system_init(void){
	Uart_Init(9600);
	ADC_Init();
	LCD_Init();
	Leds_AllInit();
	LCD_DispStrXY(1, 1, "System Started");
}

void ReadUart_Task(void* pvParam)
{
	u8 Local_Au8CharsArray[8] = {0};
	u8 Local_u8Char = 0;
	u8 Local_u8Counter = 0;
	while(1)
	{
		//		Uart_SendStr("ReadUart_Task\r\n");
		if(Uart_ReceiveByte_Unblock(&Local_u8Char) == TRUE)
		{
			//New Data is received
			if(Local_u8Char != 13)
			{
				Local_Au8CharsArray[Local_u8Counter] = Local_u8Char;
				Local_u8Counter++;
			}
			else
			{
				Local_Au8CharsArray[Local_u8Counter] = '\0';
				xQueueSend(Queue_Handler, Local_Au8CharsArray, portMAX_DELAY);
				Local_u8Counter = 0;
			}

		}
		vTaskDelay(20);
	}
}

void DataParsing_Task(void* pvParam)
{
	u8 Local_Au8CharsArray[8] = {0};
	u8 Local_u8DataArray[3] = {0};
	u8 Local_u8Counter = 0;
	u8 Local_u8Counter2 = 0;
	while(1)
	{
		Uart_SendStr("DataParsing_Task\r\n");
		if(xQueueReceive(Queue_Handler, Local_Au8CharsArray, portMAX_DELAY))
		{
			//A frame is successfully received.
			while(Local_Au8CharsArray[Local_u8Counter] != '#')
			{
				if(Local_Au8CharsArray[Local_u8Counter] != '*')
				{
					Local_u8DataArray[Local_u8Counter2] = Local_Au8CharsArray[Local_u8Counter];
					Local_u8Counter2++;
				}

				Local_u8Counter++;
			}

			if(Local_u8DataArray[0] == 'i')
			{
				switch(Local_u8DataArray[1])
				{
				case '1':
					if(Local_u8DataArray[2] == '1')
					{
						Led_On(LED3);
					}
					else if(Local_u8DataArray[2] == '0')
					{
						Led_Off(LED3);
					}
					break;
				case '2':
					if(Local_u8DataArray[2] == '1')
					{
						Led_On(LED4);
					}
					else if(Local_u8DataArray[2] == '0')
					{
						Led_Off(LED4);
					}
					break;
				case '3':
					if(Local_u8DataArray[2] == '1')
					{
						Led_On(LED5);
					}
					else if(Local_u8DataArray[2] == '0')
					{
						Led_Off(LED5);
					}
					break;
				};
			}
			else if(Local_u8DataArray[0] == 'a')
			{

			}



		}
		Local_u8Counter = 0;
		Local_u8Counter2 = 0;
	}
}

//void Action(void* pvParam)
//{
//	u8 Local_u8DataArray[3] = {0};
//
//	while(1)
//	{
//		Uart_SendStr("Action\r\n");

//		Uart_SendStr("Action\r\n");
//		if(xQueueReceive(Data_Queue, Local_u8DataArray, portMAX_DELAY))
//		{
//			if(Local_u8DataArray[0] == 'i')
//			{
//				switch(Local_u8DataArray[1])
//				{
//				case '1':
//					if(Local_u8DataArray[2] == '1')
//					{
//						Led_On(LED3);
//					}
//					else if(Local_u8DataArray[2] == '0')
//					{
//						Led_Off(LED3);
//					}
//					break;
//				case '2':
//					if(Local_u8DataArray[2] == '1')
//					{
//						Led_On(LED4);
//					}
//					else if(Local_u8DataArray[2] == '0')
//					{
//						Led_Off(LED4);
//					}
//					break;
//				case '3':
//					if(Local_u8DataArray[2] == '1')
//					{
//						Led_On(LED5);
//					}
//					else if(Local_u8DataArray[2] == '0')
//					{
//						Led_Off(LED5);
//					}
//					break;
//				};
//			}
//			else if(Local_u8DataArray[0] == 'a')
//			{
//
//			}
//		}
//	}
//}

void ReadTemp_Task(void * pvParam)
{
	u16 Local_u16DigitalRead = 0;
	u16 Local_u16AnalogRead = 0;
	while(1)
	{
		//		Uart_SendStr("ReadTemp_Task\r\n");
		Led_Off(LED1);
		Led_Off(LED2);

		Local_u16DigitalRead = ADC_Read(ADC_CH0);
		Local_u16AnalogRead = (Local_u16DigitalRead * 500) / 1024;
		if(Local_u16AnalogRead >= 35)
		{
			// Turn on the AC
			xSemaphoreGive(AC_Semph);
		}
		else if(Local_u16AnalogRead >= 30 && Local_u16AnalogRead <= 35)
		{
			//Turn on the Fan
			xSemaphoreGive(Fan_Semph);
		}
		else
		{
			// DoNothing
		}

		vTaskDelay(300);
	}
}
void AC_Task(void* pvParam)
{
	while(1)
	{
		//		Uart_SendStr("AC_Task\r\n");
		if(xSemaphoreTake(AC_Semph, portMAX_DELAY) == pdTRUE)
		{
			// Semaphore successfully taken.
			Led_On(LED2);
		}
	}
}

void Fan_Task(void* pvParam)
{
	while(1)
	{

		//		Uart_SendStr("Fan_Task\r\n");
		if(xSemaphoreTake(Fan_Semph, portMAX_DELAY) == pdTRUE)
		{
			// Semaphore successfully taken.
			Led_On(LED1);
		}
	}
}
