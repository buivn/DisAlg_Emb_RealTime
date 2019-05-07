/*
 * LCD_uC.c
 *
 * Created: 24.10.2013 11:57:17
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
#include "LCD17x8ASCII.h"
#include "USART.h"
#include "TTCAN.h"
#include "LCD_Tasks.h"

//Controller: LCD-uC
//Time plan (timemark, basemark, repeat, type, id, Task1, Task2)
// This trigger matrix will run in 3ms and repeat
const unsigned char Trigger[TIMEMARKS][TRIGGER_INDEX] PROGMEM =
{
	{0, 0, 1, 'a', 0x00, NONE,		NONE},	//
	{1, 0, 1, 'a', 0x00, NONE,		NONE},	//
	{2, 0, 1, 'r', 0x11, RX_ANGLE,	NONE},	// Receive Angles
	{3, 0, 1, 'a', 0x00, NONE, 		NONE},	//
	{4, 0, 1, 'r', 0x12, RX_PUSB, 	NONE},	// Receive Pusb_uC message
	{5, 0, 1, 'a', 0x00, NONE, 		NONE}
};

//_____ F U N C T I O N S ______________________________________________________

//----------------------------------------------------------
void vApplicationStackOverflowHook (xTaskHandle *pxTask, const portCHAR *pcTaskName)
{	
	Csr ();  // Clear LCD
	Dpstr (0, 0, "Stack overflow:");
	Dpstr(1, 0, pcTaskName);  	
	cli();  // No longer interrupts
	while (1);  // Infinity loop
} 
//------------------------------------------------------------------------------
/*! \brief main
 *	 
 *   Main function
 *   Init HW, create tasks + queus, start scheduler
 */
int main(void)
{
	unsigned char ok;
	DDRD = 0;	PORTD = 0; //reset RX and TX CAN
	DDRF = 0xff; PORTF = 0; 
	// DOGS-LCD init
	DOGS_LCD_init();
	Csr();
	// Init CAN HW
	TTCAN_Init();
	// Init USART
	USART_Init((unsigned int)16); // F_CPU=16MHz/U2Xn=1/BAUD=115200
	
	//Create the Tasks and Queues 
	ok = 1;
	// priority 2
	ok &= xTaskCreate(Data_Reception, (const signed portCHAR *) "Receive_Data", \
	configMINIMAL_STACK_SIZE + 45, NULL, tskIDLE_PRIORITY + 2, &TH_Receive_Data);
	// priority 1
	ok &= xTaskCreate(LCD_Display, (const signed portCHAR *) "ShowOn_LCD", \
	configMINIMAL_STACK_SIZE + 34, NULL, tskIDLE_PRIORITY + 1, &TH_ShowOn_LCD);
	//ok &= xTaskCreate (uC_Check, (const signed portCHAR *) "Check_uC", \
	configMINIMAL_STACK_SIZE + 10, NULL, tskIDLE_PRIORITY + 1, &TH_Check_uC);	
	
	// Queue to wake up RxData task
	Wake_RxData = xQueueCreate(1, sizeof(unsigned char));
	if (Wake_RxData == 0) ok &= 1;
	// if all the tasks and queues were created, Start scheduler
	if (ok == 1) vTaskStartScheduler();
}