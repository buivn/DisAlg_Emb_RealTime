/*
 * MOTO_uC.c
 *
 * Created: 24.10.2013 11:52:51
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
#include "MOTO_Tasks.h"
#include "TTCAN.h"

//Controller: MOTO-uC
//Time plan (timemark, basemark, repeat, type, id, Task1, Task2)
// This trigger matrix will run in 3ms and repeat
const unsigned char Trigger[TIMEMARKS][TRIGGER_INDEX] PROGMEM =
{
	{0, 0, 1, 'a', 0x00, NONE,		NONE},	//
	{1, 0, 1, 'a', 0x00, NONE,		NONE},	//
	{2, 0, 1, 'r', 0x11, RX_ANGLE,	NONE},	// receive Angles
	{3, 0, 1, 'a', 0x00, NONE, 		NONE},	//
	{4, 0, 1, 'r', 0x12, RX_PUSB, 	NONE},	// Receive Pusb_uC message
	{5, 0, 1, 'a', 0x00, NONE,		NONE}
};

unsigned char Direction=0;
//_____ F U N C T I O N S ______________________________________________________

//----------------------------------------------------------
void vApplicationStackOverflowHook (xTaskHandle *pxTask, const portCHAR *pcTaskName)
{	unsigned char  i;
	// Send Taskname to UART
	USART_Transmit ('S');
	USART_Transmit ('O');
	USART_Transmit (':');
	USART_Transmit (' ');
	for (i = 0; *(pcTaskName + i) != 0; i++)
		USART_Transmit (*(pcTaskName + i));
	cli();  // No longer interrupts
	PORTC = 0; // Motor driver off
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
	DDRD = 0;	PORTD = 0;
	DDRC = 0b01110000;	
	PORTC = 0;			// Power off at L6205 (Motor Port)
	DDRF = 0xff; PORTF = 0;
	unsigned char  ok;
	
	// Init Timer 3 for DCMotor PWM generation
	TCCR3A = 0;					// CTC mode
	TCCR3B = 0b00001001;		// Prescaler: Clkio/1 (last 3 bits)
	TCCR3C = 0;
	TIMSK3 = 0b00000110;		// Enable ISR on compA and compB
	
	TCNT3 = 0;
	OCR3B = 0;								
	OCR3A = 799;		// Time point to set PWM
	// Init CAN HW
	TTCAN_Init();
	// Init USART HW
	USART_Init ((unsigned int) 51); // F_CPU=16MHz/U2Xn=1/BAUD=38400
	
	ok = 1;
	// Create tasks
	//Priority 2
	ok &= xTaskCreate(Data_Reception, (const signed portCHAR *) "Receive_Data", \
	configMINIMAL_STACK_SIZE + 10, NULL, tskIDLE_PRIORITY + 2, &TH_Receive_Data);
	//Priority 1
	ok &= xTaskCreate(Upright_Controller, (const signed portCHAR *) "Upright Controller", \
		configMINIMAL_STACK_SIZE + 10, NULL, tskIDLE_PRIORITY + 1, &TH_Upright);
	ok &= xTaskCreate(SwingUp_Controller, (const signed portCHAR *) "SwingUp", \
	configMINIMAL_STACK_SIZE + 10, NULL, tskIDLE_PRIORITY + 1, &TH_SwingUp);	 	
	
	// Create the Queues
	// Queue to control Direction of DC motor
	DirectionQueue = xQueueCreate(1, sizeof(unsigned char));
	// Queue to wake up the Rx_Data task
	Wake_RxData = xQueueCreate(1, sizeof(unsigned char));
	
	// two Semaphore to wake up Upright controller, SwingUp_Controller task
	vSemaphoreCreateBinary(Wake_Upright);
	vSemaphoreCreateBinary(Wake_SwingUp);
	
	if (DirectionQueue == 0) ok &= 1;
	if (Wake_RxData == 0) ok &= 1;
	
	// if all tasks were created, start the Scheduler
	if (ok == 1) vTaskStartScheduler();
}

//_____ I N T E R U P T S ______________________________________________________

ISR(TIMER3_COMPB_vect)
{
	PORTC &= 0b11001111;	
}

ISR(TIMER3_COMPA_vect)
{
	xQueueReceiveFromISR(DirectionQueue, &Direction, 0);
	
	if (Direction == 3) PORTC &= 0b11001111;	
	if (Direction == 1) 
	{
		PORTC &= 0b11001111;
		PORTC |= 0b01010000;
	}
	if (Direction == 2) 
	{
		PORTC &= 0b11001111;
		PORTC |= 0b01100000;
	}
}