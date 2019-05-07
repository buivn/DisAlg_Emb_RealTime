/*
 * PUSB_uC.c
 *
 * Created: 24.10.2013 11:48:57
 *  Author: BuiAdmin
 */ 


//_____ I N C L U D E S ________________________________________________________

// Compiler includes
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
// FreeRTOS includes
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
// Modul includes
#include "global.h"
#include "USART.h"
#include "TTCAN.h"
#include "PUSB_Tasks.h"

//Controller: PUSB-uC
//Time plan (timemark, basemark, repeat, type, id, Task1, Task2)
// This trigger matrix will run in 3ms and repeat
const unsigned char Trigger[TIMEMARKS][TRIGGER_INDEX] PROGMEM =
{
	{0, 0, 1, 'a', 0x00, NONE,		NONE},	//
	{1, 0, 1, 'a', 0x00, NONE, 		NONE},	//
	{2, 0, 1, 'r', 0x11, RX_ANGLE, 	NONE},	// Receive Angles
	{3, 0, 1, 's', 0x12, NONE, 		NONE},	// Send PUSB_uC
	{4, 0, 1, 'a', 0x00, NONE,		NONE},	//
	{5, 0, 1, 'a', 0x00, NONE,		NONE}
};

//_____ F U N C T I O N S ______________________________________________________

//----------------------------------------------------------
void vApplicationStackOverflowHook (xTaskHandle *pxTask, const portCHAR *pcTaskName)
{	unsigned char  i;
	char message [] = "Stack Overflow in";
	// Send Taskname to UART
	for (i = 0; message [i] != 0; i++)
		USART_Transmit (message [i]);
	for (i = 0; *(pcTaskName + i) != 0; i++)
		USART_Transmit (*(pcTaskName + i));
	cli();  // No longer interrupts
	while (1);  // Infinity loop
} 
//------------------------------------------------------------------------------
/*! \brief main
 *	 
 *   Main function
 *   Init HW, create tasks + queues, start scheduler
 */
int main(void)
{ 
	// Declaration of local vars
	unsigned char  ok;
	DDRA = 0;	PORTA = 0;
	DDRB = 0;	PORTB = 0;
	DDRF = 0;	PORTF = 0;
	DDRD = 0;	PORTD = 0;
	DDRC = 0xff;PORTC = 0;	// reset the Test Port
	// initialize the frames	
	Pusb_uC0.id = 0x0002;
	Pusb_uC0.length = 8;
	Pusb_uC1.id = 0x0002;
	Pusb_uC1.length = 8;
	// Init CAN HW
	TTCAN_Init();
	// Init USART HW
	USART_Init((unsigned int)51); // F_CPU=16MHz/U2Xn=1/BAUD=38400
	
	// Create tasks
	ok = 1;
	// Priority 2
	ok &= xTaskCreate (PWM_Calculation, (const signed portCHAR *) "C.ControlledData", \
		configMINIMAL_STACK_SIZE + 92, NULL, tskIDLE_PRIORITY + 2, &TH_C_PWM);
	// Priority 1
	ok &= xTaskCreate (PushedButton_Detection, (const signed portCHAR *) "D.PButton", \
		configMINIMAL_STACK_SIZE + 26, NULL, tskIDLE_PRIORITY + 1, &TH_PushedButton);
	
	// Queues to interchange data on this micro controller
	NomInputQueue = xQueueCreate(1, sizeof(char));
	Choose_Frame_Queue = xQueueCreate(1, sizeof(unsigned char));
	
	if (NomInputQueue == 0) ok &= 1;
	if (Choose_Frame_Queue == 0) ok &= 1;
	
	vSemaphoreCreateBinary(WakeTaskSe);		
	// if every task and functions were created, start Scheduler
	if (ok == 1)  vTaskStartScheduler(); 
	
	// Error code for end of scheduling
}

//_____ I N T E R U P T S ______________________________________________________