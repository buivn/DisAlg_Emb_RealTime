/*
 * SEN_Tasks.c
 *
 * Created: 05.11.2013 14:56:02
 *  Author: BuiAdmin
 */ 

//_____ I N C L U D E S ________________________________________________________
#include <string.h>
#include "USART.h"
#include "TTCAN.h"
#include "SEN_Tasks.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "global.h"
#include "semphr.h"

//_____ F U N C T I O N S - D E F I N I T I O N S _________________

//----------------------------------------------------------
// Get Data from Sensor, save on the Queue, priority 2, memory 30kb
//----------------------------------------------------------
void Data_Preparation(void * pvParameters)
{
	int Buffer1, Buffer2, Buffer3;
	DDRF = 0xff;
	PORTF = 0;
	for (;;) // Start of infinity loop
	{
		if (xSemaphoreTake(WakeTaskSe, portMAX_DELAY))
		{
			PORTF |= 0b10000000;
			
			// Save pendulum angle
			if ((Pendulum_axis >= 2000)||(Pendulum_axis <= -2000))
			{
				Pendulum_axis = Pendulum_axis % 2000;	
			}
			Buffer1 = Pendulum_axis;
			// offset 180 degree the coordinate
			if (Pendulum_axis < 0) Buffer3 = Buffer1 + 1000;
			else Buffer3 = Buffer1 - 1000;
			
			Angle.data[0] = (unsigned char)(Buffer1 >> 8);
			Angle.data[1] = (unsigned char)(Buffer1 & 0x00ff);
		
			// Save arm angle
			if ((Arm_axis >= 2000)||(Arm_axis <= -2000))
			{
				Arm_axis = Arm_axis % 2000;	
			}
			Buffer2 = Arm_axis;
			Angle.data[2] = (unsigned char)(Buffer2 >> 8);
			Angle.data[3] = (unsigned char)(Buffer2 & 0x00ff);	
		
			// Save Pendulum up
			Angle.data[4] = (unsigned char)(Buffer3 >> 8);
			Angle.data[5] = (unsigned char)(Buffer3 & 0x00ff);
			PORTF &= 0b01111111;
		}
	} // end infinity loop
} // end of task Get_Sensor_Data

//----------------------------------------------------------
// Blink with LEDs on the sensor I/O ports
//----------------------------------------------------------
void  uC_Check(void *pvParameters)
{
	// Init TPort HW
	DDRF = 0xff; // PF is output
	PORTF = 0; // 
	unsigned char lauf=0;
	for (;;) // Start of infinity loop
	{ 
		PORTF = 0;
		PORTF |= (1 << lauf);
		lauf ++;
		if (lauf > 7) lauf = 0;
		vTaskDelay(100);
	} // end infinity loop
} // end of task Test_TPort

//----------------------------------------------------------
// This function outputs one Character via USART
//----------------------------------------------------------
void PutChar(char data)
{
	while (!(UCSR0A & 0x20));
	UDR0=data;
}

//----------------------------------------------------------
// This function outputs one String via USART
//----------------------------------------------------------
void Print(char buffer[])
{
	unsigned char i;

	for (i=0; buffer[i] != 0; i++) PutChar(buffer[i]);
}

//----------------------------------------------------------
// This function outputs Pendulum angle via USART to PC
//----------------------------------------------------------
void PrintOnPC()
{
	portTickType xLastWakeTime;
	unsigned char i;
	//char *pointer;
	// variables to store the local time in string
	char buffer1[7], buffer2[7], buffer3[7], buffer4[7], buffer5[7];	
	
	for (i=0;i< 5; i++)
	{
		buffer1[i] = 1;
		buffer2[i] = 1;
		buffer3[i] = 1;
		buffer4[i] = 1;
		buffer5[i] = 1;
	}
	for (;;)
	{
		// Get the local time
		xLastWakeTime = xTaskGetTickCount();	
		// convert local time value to a string (6 elements)
		//pointer = (char*)itoa(xLastWakeTime, buffer1, 10);
		itoa(xLastWakeTime, buffer1, 10);
		// convert angles and angular velocities to a string (6 elements)
		itoa(Pendulum_axis, buffer2, 10);
		itoa(Arm_axis, buffer3, 10);
		itoa(Pen_vel, buffer4, 10);
		itoa(Arm_vel, buffer5, 10);		
		
		// the last char of string is zero	
		buffer1[6] = 0;
		buffer2[6] = 0;
		buffer3[6] = 0;
		buffer4[6] = 0;
		buffer5[6] = 0;
		// print the local time
		Print(buffer1);				
		Print(": ");	
		
		// print the Pendulum angle
		Print(buffer2);
		Print("; ");
		
		// print the Arm angle
		Print(buffer3);
		Print("; ");
		
		// print the Pendulum angle velocity
		Print(buffer4);
		Print("; ");
		
		// print the Arm angle velocity
		Print(buffer5);
		Print("; ");		
		// make a newline
		PutChar(10);
		//PutChar(13);
		
		// delay 3ms to get the new angular velocity
		vTaskDelay(3);
	}
}
