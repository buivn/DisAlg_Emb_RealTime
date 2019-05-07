/*
 * SEN_uC.c
 *
 * Created: 24.10.2013 11:40:26
 *  Author: BuiAdmin
 */ 


//_____ I N C L U D E S ________________________________________________________

// Compiler includes
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>
// FreeRTOS includes
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
// Modul includes
#include "global.h"
#include "USART.h"
#include "SEN_Tasks.h"
#include "TTCAN.h"
#include "semphr.h"


//Controller: SEN-uC
//Time plan (timemark, basemark, repeat, type, id, Task1, Task2)
// This trigger matrix will run in 3ms and repeat
const unsigned char Trigger[TIMEMARKS][TRIGGER_INDEX] PROGMEM =
{
	{0, 0, 1, 't', 0x00, NONE, PR_DATA},	// Prepare the Sensor Data
	{1, 0, 1, 's', 0x11, NONE, NONE},		// Send Angles
	{2, 0, 1, 'a', 0x00, NONE, NONE},
	{3, 0, 1, 'a', 0x00, NONE, NONE}, //
	{4, 0, 1, 'a', 0x00, NONE, NONE},	//
	{5, 0, 1, 'a', 0x00, NONE, NONE}
};

//_____ F U N C T I O N S ______________________________________________________

//----------------------------------------------------------
void vApplicationStackOverflowHook (xTaskHandle *pxTask, const portCHAR *pcTaskName)
{	
	unsigned char  i;
	char message [] = "Stack Overflow in Task:";
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
	// Init of global vars
	DDRD = 0;	PORTD = 0; // reset the TX and RX CAN
	DDRE = 0;	PORTE = 0;
	DDRF = 0xff;	PORTF = 0;	// reset the Test Port
	
	Pendulum_axis = 0;
	Arm_axis = 0;
	Angle.id = 0x0001;
	Angle.length = 6;
	// Init for external ints at port D 0..2
	EICRA = 0b00010001; // Int on both edges at PD0 and PD2
	EICRB = 0b00010001;
	EIMSK = 0b01010101; // Int0/PD0, Int2/PD2, Int4/PE4, Int6/PE6

	// Init CAN HW
	TTCAN_Init();
	// Init USART HW
	USART_Init ((unsigned int) 16); // F_CPU=16MHz/U2Xn=1/BAUD=115200
	
	// Create tasks
	ok = 1;
	// Priority 2
	ok &= xTaskCreate (Data_Preparation, (const signed portCHAR *) "Get_SensorData", \
		configMINIMAL_STACK_SIZE + 30, NULL, tskIDLE_PRIORITY + 2, &TH_Get_Sensor_Data);
	// Priority 1
	ok &= xTaskCreate (uC_Check, (const signed portCHAR *) "Check_uC", \
		configMINIMAL_STACK_SIZE + 20, NULL, tskIDLE_PRIORITY + 1, &TH_Check_uC);
	
	vSemaphoreCreateBinary(WakeTaskSe);	
	
	// If all the tasks were created, Start scheduler
	if (ok == 1) vTaskStartScheduler();
	// Error code for end of scheduling
}

//_____ I N T E R U P T S ______________________________________________________
ISR(INT0_vect)
{	// Pin A of quadrature encoder
	if (((PIND & 5) == 4) || ((PIND & 5) == 1))
		Pendulum_axis++;
	else  Pendulum_axis--;
}
//------------------------------------------------------------------------------
ISR(INT1_vect)
{	// Pin I of quadrature encoder
}
//------------------------------------------------------------------------------
ISR(INT2_vect)
{	// Pin B of quadrature encoder
	if ( ((PIND & 5) == 0) || ((PIND & 5) == 5) )
		Pendulum_axis++;
	else  Pendulum_axis--;
}
//------------------------------------------------------------------------------
ISR(INT4_vect)
{	// Pin A of quadrature encoder
	if ( ((PINE & 80) == 64) || ((PINE & 80) == 16) ) 
		Arm_axis++;
	else  Arm_axis--;
}
//------------------------------------------------------------------------------
ISR(INT5_vect)
{	// Pin I of quadrature encoder
}
//------------------------------------------------------------------------------
ISR(INT6_vect)
{	// Pin B of quadrature encoder
	if ( ((PINE & 80) == 0) || ((PINE & 80) == 80) )
		Arm_axis++;
	else  Arm_axis--;
}