/*
 * MOTO_Tasks.c
 *
 * Created: 29.10.2013 15:34:17
 *  Author: BuiAdmin
 */ 
#include "MOTO_Tasks.h"
#include "USART.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "global.h"
#include "TTCAN.h"

//_____ D E F I N I T I O N S - F U N C T I O N S _______________________________

// Global vars
unsigned char Start_Switch = FALSE;

//----------------------------------------------------------
// Receive the data on two CAN message: the angles and PWM 
// and start the task with memory 10, priority 2
//----------------------------------------------------------
void Data_Reception(void * pvParameters)
{
	unsigned char Check_Slot = 0;
	unsigned char  k = 3;
	for (;;)
	{	
		 //Receive angle message? 
		if(xQueueReceive(Wake_RxData, &Check_Slot, portMAX_DELAY))
		{		
			// if receive angle message
			if (Check_Slot == RX_ANGLE) 
			{
				// receive data from the Angles message
				Pen_Angle = (Angle.data[0] << 8) + Angle.data[1];
				Arm_Angle = (Angle.data[2] << 8) + Angle.data[3];	
				Pen_Up = (Angle.data[4] << 8) + Angle.data[5];		
				// safety condition
				if ((Arm_Angle > 600)||(Arm_Angle < -600))	
				{
					OCR3B = 0;
					xQueueSend(DirectionQueue, &k, 0);
					Start_Switch = FALSE;				
				}
			}
			// if receive Pusb_uC message
			if (Check_Slot == RX_PUSB) 
			{
				// if started button is pushed
				if (Pusb_uC.data[0] == 1) Start_Switch = TRUE;
				// if stopped button is pushed
				if (Pusb_uC.data[1] == 1)
				{
					OCR3B = 0;
					xQueueSend(DirectionQueue, &k, 0);
					Start_Switch = FALSE;	// set the start condition off
				}
				// trigger the Upright and SwingUp Controller
				if ((Pen_Up > -90) && (Pen_Up < 90)) xSemaphoreGive(Wake_Upright);
				else xSemaphoreGive(Wake_SwingUp);
			}			
		}	//end of if (xQueue..)
	}	// end of infinity loop
}

//----------------------------------------------------------
// Keep Pendulum in Upright position, memory 10, priority 1
//----------------------------------------------------------
void Upright_Controller(void * pvParameters)
{ 
	char k = 3;	// control the direction of DC-motor
	int SBData;
	for (;;) // Start of infinity loop
	{		
		// wait for a semaphore from Data_Reception() task
		if(xSemaphoreTake(Wake_Upright, portMAX_DELAY))	
		{	
			if (Start_Switch)	// if start button is pushed		
			{			
				SBData	= (Pusb_uC.data[5] << 8) + Pusb_uC.data[6];																		
				if (SBData > 0)
				{
					k = 1;
					xQueueSend(DirectionQueue, &k, 0);
				}
				if (SBData < 0)
				{
					k = 2;
					xQueueSend(DirectionQueue, &k, 0);
				}
				if (SBData == 0)
				{
					k = 3;
					xQueueSend(DirectionQueue, &k, 0);
				}
				if (SBData < 0) SBData = - SBData;
				if (SBData > 795) SBData = 795;
				OCR3B = (unsigned int) (SBData);	// Change OCR3B, PWM
			}	// end of Start_Switch condition
		} // end of if (xSemaphore ...)
	} // end infinity loop
}

//----------------------------------------------------------
// The task: Swing Up the Pendulum, memory: 20kb, priority 1
//----------------------------------------------------------
void SwingUp_Controller(void * pvParameters)
{
	// Local vars
	unsigned char k = 3;
	unsigned char state= 0;
	for (;;) // Start of infinity loop
	{
		// wait a semaphore from Data_Reception() task
		if(xSemaphoreTake(Wake_SwingUp, portMAX_DELAY))	
		{
			if (Start_Switch)	// if start button is pushed
			{	
				OCR3B = 250;	// set OCR3B to swing up
				switch (state)
				{
					case 0: vTaskDelay(4000);
							state = 1;
							break;
					case 1:
						k = 1; xQueueSend(DirectionQueue, &k, 0);
						if (Arm_Angle > 32) state = 2;
						break;
					case 2:
						k = 3; xQueueSend(DirectionQueue, &k, 0);
						if (Pen_Angle > 0) state = 3;
						break;
					case 3:
						k = 2; xQueueSend(DirectionQueue, &k, 0);
						if (Arm_Angle < 0)  state = 4;
						break;
					case 4:
						k = 3; xQueueSend(DirectionQueue, &k, 0);
						if (Pen_Angle < 0)  state = 1;
						break;
					default:
						k = 3; xQueueSend(DirectionQueue, &k, 0); // Stop
						break;
				} // end of switch	
			}	//end of Start_Switch condition
		}	// end of if (semaphore)
	}	// end of for() loop
}


/*//----------------------------------------------------------
// Check uC function
//----------------------------------------------------------
void PWM_Test(void * pvParameters)
{ 
	//Local vars
	unsigned int lauf = 0, lauf1=0;
	unsigned char k = 2;
	OCR3A = 799;
	for (;;) // Start of infinity loop
	{
		lauf1++;
		lauf = lauf + 10;		
		OCR3B = lauf;
		if (lauf >= 790) lauf = 0;
		if (lauf1 > 400)
		{
			lauf1 = 0;
			if (k == 1) k = 2;
			else k = 1;
		}
		xQueueSend(DirectionQueue, &k, 0);
		vTaskDelay(10); 
	} // end infinity loop
}
*/
//----------------------------------------------------------
// This function outputs one Character via UART
//----------------------------------------------------------
void PutChar(char data)
{
	while (!(UCSR0A&0x20));
	UDR0=data;
}

//----------------------------------------------------------
// This function outputs one String via UART
//----------------------------------------------------------
void Print(char buffer[])
{
	unsigned char i;

	for (i=0; buffer[i] != 0; i++) PutChar(buffer[i]);
}
//----------------------------------------------------------