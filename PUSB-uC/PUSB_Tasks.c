/*
 * PUSB_Tasks.c
 *
 * Created: 04.11.2013 17:37:39
 *  Author: BuiAdmin
 */ 

//_____ I N C L U D E S ________________________________________________________
#include "USART.h"
#include "TTCAN.h"
#include "PUSB_Tasks.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "global.h"

char NominalInput;
//_____F U N C T I O N S - D E F I N I T I O N S ____________________

//----------------------------------------------------------
// This function calculates the Controlled Data
//----------------------------------------------------------
void  PWM_Calculation(void * pvParameters)
{ 
	// system parameter
	const float K1 = -3.92, K2 = 55.35, K3 = -5.86, K4 = 10.66, Ki = -1.41;	
	const float dt = 0.003;
	// Local vars
	float Arm_Up, Pen_Up, Arm_Vel, Pen_Vel, Last_ArmUp = 0, Last_PenUp = 0;
	float Feedback,F_Ref_Input = 0;// Moto_Stable, ;
	float Current_Error, Control_Error = 0, SBData;
	int buffer1, buffer2, Control_Value;
	unsigned int count=0;
	char C_Ref_Input, Oscillation = FALSE;		
	
	for (;;)	// Start of infinity loop
	{ 
		if (xSemaphoreTake(WakeTaskSe, portMAX_DELAY))
		{
			count++;	// increase count value to oscillate the pendulum
			// receive data from the Angles message - the up angles
			buffer1 = (Angle.data[4] << 8) + Angle.data[5];			
			buffer2 = (Angle.data[2] << 8) + Angle.data[3];
			// Convert the angle to radian
			Pen_Up = (float) (buffer1*3.142/1000);
			Arm_Up = (float) (buffer2*3.142/1000);
			// Velocities calculation
			Pen_Vel = (Pen_Up - Last_PenUp)/dt;
			Arm_Vel = (Arm_Up - Last_ArmUp)/dt;			
			// receiving new nominal Input?
			if (xQueueReceive(NomInputQueue, &C_Ref_Input, 0))
			{
				if (C_Ref_Input == 100)		// signal to start oscillation
				{
					F_Ref_Input = 0.4;
					count = 0;
					Oscillation = TRUE;
				}
				else if (C_Ref_Input == -100)	// signal to stop oscillation
				{
					F_Ref_Input = 0;
					Oscillation = FALSE;
				}
				else
				{	// Convert Nominal value to radian
					F_Ref_Input = ((float) (C_Ref_Input))*0.01745;	// signal to change reference input
					Oscillation = FALSE;
				}
			}		
			// code to control the oscillation
			if ((Oscillation == TRUE)&&(count > 3300))
			{
				if (F_Ref_Input > 0) F_Ref_Input = -0.4;
				else F_Ref_Input = 0.4;
				count = 0;
			}
			
			// Control Signal with nonzero ref. Input
			Current_Error = Arm_Up - F_Ref_Input;		
			
			//Feedback signal
			Feedback = K1*Arm_Up + K2*Pen_Up + K3*Arm_Vel + K4*Pen_Vel;						
			
			// if the Pendulum angle is in the upright range
			if ((buffer1 > -90) && (Pen_Up < 90))
				Control_Error = Control_Error + Ki*Current_Error;
			else Control_Error = 0;			
			
			// total voltage apply to DC-motor
			SBData = -Control_Error*dt - Feedback;				
			// convert the voltage value to the value of OCR3B
			//Moto_Stable = (float) (SBData/12.0);
			//Control_Value = (int) (Moto_Stable*799);			
			Control_Value = (int) (SBData*66.58);			
			// if there are no button pushed?
			Pusb_uC0.data[5] = (unsigned char) (Control_Value >> 8);
			Pusb_uC0.data[6] = (unsigned char) (Control_Value & 0x00ff);			
			// if any button is pushed?
			Pusb_uC1.data[5] = (unsigned char) (Control_Value >> 8);
			Pusb_uC1.data[6] = (unsigned char) (Control_Value & 0x00ff);
			// save data for the next calculation
			Last_PenUp = Pen_Up;
			Last_ArmUp = Arm_Up;	
		}
	} // end infinity loop
} // end of task Calculate_ControlledData

//----------------------------------------------------------
// Dectect_PushedButton detect pushed button then save the variation on PushButton
//----------------------------------------------------------
void  PushedButton_Detection(void * pvParameters)
{
	DDRF = 0b00001111;
	DDRA = 0b11111111;
	// Local vars
	unsigned char i,k, Oscillation_Switch;
	unsigned char P1=0, P2=0, P3=0, P4=0, P5=0, P6=0, P7=0, P8=0;
	unsigned char R1=0, R2=0, R3=0, R4=0, R5=0, R6=0, R7=0, R8 = 0;

	for (i=0; i < 5; i++) Pusb_uC0.data[i] = 0;		
	
	for (;;) // Start of infinity loop
	{	
		PORTA = 0;
		PORTF = 0xff;
		PORTF = 0b11111110; // S1 .. S3
		k = PINF; // read the PINF one time for the SYNC LATCH
		
		// Button 1: Start button
		if ((PINF & 0b00010000) == 0)  
		{
			P1++;
			if (P1 > Confidence_Level) P1 = Confidence_Level;	
		}
		else if (P1 == Confidence_Level)
		{
			R1++;
			if (R1 > Confidence_Level) R1 = Confidence_Level;
		}
		if ((P1 == Confidence_Level) && (R1 == Confidence_Level))
		{
			for (i=0; i < 5; i++) Pusb_uC1.data[i] = 0;	
			P1 = 0;
			R1 = 0;			
			// turn on the respective LED
			PORTA = 0b00000001;
			
			// Set the PushButton frame/message
			Pusb_uC1.data[0] = 1;		// send the start signal to the motor microcontroller
			
			if (NominalInput < -90) NominalInput = -90;
			if (NominalInput > 90) NominalInput = 90;
			Pusb_uC1.data[4] = NominalInput;	// send the reference input to the LCD Microcontroller
			k = 1;
			xQueueSend(NomInputQueue, &NominalInput, 0);	// send the reference input to PWM calculation
			xQueueSend(Choose_Frame_Queue, &k, 0);			// choose the changed frame to send to other microcontroller
		}
		
		// Button 2: Stop button	
		if ((PINF & 0b00100000) == 0)  
		{
			P2++;
			if (P2 > Confidence_Level) P2 = Confidence_Level;
		}
		else if (P2 == Confidence_Level)
		{
			R2++;
			if (R2 > Confidence_Level) R2 = Confidence_Level;
		}
		if ((P2 == Confidence_Level) && (R2 == Confidence_Level))
		{
			for (i=0; i < 5; i++) Pusb_uC1.data[i] = 0;	 // reset the data on the frame
			// Turn on the respective LED
			PORTA = 0b00000010;	
			// set the sending frame
			Pusb_uC1.data[1] = 1;		// send a signal to turn off the DC motor
			Pusb_uC1.data[4] = NominalInput;
			k = 1;			
			xQueueSend(Choose_Frame_Queue, &k, 0);
			P2 = 0;
			R2 = 0;
		}
		
		// Button 3: Change reference Input button
		if ((PINF & 0b01000000) == 0)	// if the third button is pressed?
		{
			P3++;
			if (P3 > Confidence_Level) P3 = Confidence_Level;
		}
		else if (P3 == Confidence_Level) // if the third button is released?
		{
			R3++;
			if (R3 > Confidence_Level) R3 = Confidence_Level;
		}
		if ((P3 == Confidence_Level) && (R3 == Confidence_Level))
		{
			for (i=0; i < 5; i++) Pusb_uC1.data[i] = 0;	 // reset the data frame
			PORTA = 0b00000100;
			
			Pusb_uC1.data[2] = 1;			// store the signal to show the change ref. Input screen
			Pusb_uC1.data[4] = NominalInput;	// store the ref. input
			k=1;			
			xQueueSend(Choose_Frame_Queue, &k, 0);	// choose the Pusb_uC1 to send to other microcontroller
			P3 = 0;
			R3 = 0;
		}
		
		PORTF = 0xff;
		PORTF = 0b11111101; // S4 .. S6
		k = PINF;			// read the PINF one time for the SYNC LATCH
		// Button 4: Increase button
		if ((PINF & 0b00010000) == 0)  
		{
			P4++;
			if (P4 > Confidence_Level) P4 = Confidence_Level;
		}
		else if (P4 == Confidence_Level) {
			R4++;
			if (R4 > Confidence_Level) R4 = Confidence_Level;
		}
		if ((P4 == Confidence_Level) && (R4 == Confidence_Level))
		{
			for (i=0; i < 5; i++) Pusb_uC1.data[i] = 0;		
			PORTA = 0b00001000;	// Turn on the respective LED
			// set the PushButton Message
			NominalInput = NominalInput+1;				// increase the reference input
			if (NominalInput > 90) NominalInput = 90;	// check working condition
			Pusb_uC1.data[2] = 1;				// store the signal to show the changed ref. Input screen
			Pusb_uC1.data[4] = NominalInput;	// store the ref. input
			k=1;			
			xQueueSend(Choose_Frame_Queue, &k, 0);
			P4 = 0; R4 = 0;		// reset the check confidence_level
		}
		// Button 5: Decrease button
		if ((PINF & 0b00100000) == 0)
		{
			P5++;
			if (P5 > Confidence_Level) P5 = Confidence_Level;
		}
		else if (P5 == Confidence_Level) {
			R5++;
			if (R5 > Confidence_Level) R5 = Confidence_Level;
		}
		if ((P5 == Confidence_Level)&&(R5==Confidence_Level))
		{	
			for (i=0; i < 5; i++) Pusb_uC1.data[i] = 0;	
			PORTA = 0b00010000;	// Turn on the respective LED
			// set the PushButton Message
			NominalInput = NominalInput - 1;		// reduce the ref. Input
			if (NominalInput < -90) NominalInput = -90;	// check working condition
			Pusb_uC1.data[2]=1;				// store the signal to show the changed ref. Input screen
			Pusb_uC1.data[4]= NominalInput;		// save the ref. Input
			k=1;			
			xQueueSend(Choose_Frame_Queue, &k, 0);
			P5 = 0;	R5 = 0;		// reset the check confidence_level
		}
		
		// Button 6: Finish button
		if ((PINF & 0b01000000) == 0)  
		{
			P6++;
			if (P6 > Confidence_Level) P6 = Confidence_Level;
		}
		else if (P6 == Confidence_Level) {
			R6++;
			if (R6 > Confidence_Level) R6 = Confidence_Level;
		}
		if ((P6 == Confidence_Level)&&(R6==Confidence_Level))
		{
			for (i=0; i < 5; i++) Pusb_uC1.data[i] = 0;
			PORTA = 0b00100000;
			Pusb_uC1.data[3] = 1;		// store the signal to show the first screen			
			Pusb_uC1.data[4] = NominalInput;		// save the ref. Input
			xQueueSend(NomInputQueue, &NominalInput, 0);	// send the new ref. Input for PWM calculation
			k=1;
			xQueueSend(Choose_Frame_Queue, &k, 0);	// choose Pusb_uC1 frame to send to other microcontroller
			P6 = 0;	R6 = 0;
		} 
		PORTF = 0xff;
		PORTF = 0b11111011; // check for button 7 and button 8
		k = PINF;			// read the PINF one time for the SYNC LATCH
		// Button 7: Oscillation button
		if ((PINF & 0b00010000) == 0)  {
			P7++;
			if (P7 > Confidence_Level) P7 = Confidence_Level;
		}
		else if (P7 == Confidence_Level) {
			R7++;
			if (R7 > Confidence_Level) R7 = Confidence_Level;
		}
		if ((P7 == Confidence_Level) && (R7 == Confidence_Level))
		{
			for (i=0; i < 5; i++) Pusb_uC1.data[i] = 0;	
			PORTA = 0b01000000;			// Turn on the respective LED
			Pusb_uC1.data[2] = 1;				// store the signal to show the second screen
			NominalInput = 0;
			Pusb_uC1.data[4] = NominalInput;	// store the ref. input
			Oscillation_Switch = 100;
			xQueueSend(NomInputQueue, &Oscillation_Switch, 0);	// send the new ref. Input for PWM calculation
			k=1;			
			xQueueSend(Choose_Frame_Queue, &k, 0);
			P7 = 0; R7 = 0;
		}
		
		// Button 8: Stop Oscillation button
		if ((PINF & 0b00100000) == 0) {
			P8++;
			if (P8 > Confidence_Level) P8 = Confidence_Level;
		}
		else if (P8 == Confidence_Level) {
			R8++;
			if (R8 > Confidence_Level) R8 = Confidence_Level;
		}
		if ((P8 == Confidence_Level) && (R8 == Confidence_Level))
		{
			for (i=0; i < 5; i++) Pusb_uC1.data[i] = 0;
			PORTA = 0b10000000;		// Turn on the respective LED
			Pusb_uC1.data[2] = 1;				// store the signal to show the second screen
			Pusb_uC1.data[4] = NominalInput;	// store the ref. input		
			Oscillation_Switch = -100;
			// send the new ref. Input for PWM calculation
			xQueueSend(NomInputQueue, &Oscillation_Switch, 0);	
			k=1;
			xQueueSend(Choose_Frame_Queue, &k, 0);
			P8 = 0; R8 = 0;
		}
		Pusb_uC0.data[4] = NominalInput;
		vTaskDelay(8);
	} // end infinity loop
} // end of task Detect_PushedButton

//----------------------------------------------------------
// Blink with the LEDs at the push button matrix
//----------------------------------------------------------
void Check_uC( void * pvParameters)
{ 
	// Init I/Os
	DDRC = 255;
	PORTC = 0;
	//Local vars
	unsigned char lauf = 0;
	for (;;) // Start of infinity loop
	{
		PORTC = 0x00;
		PORTC |= (1 << lauf);
		lauf++;
		if (lauf > 7) lauf = 0;
		vTaskDelay(300);
	} // end infinity loop
} // end of task Check_uC

//----------------------------------------------------------
// This function outputs one Character via UART
//----------------------------------------------------------
void PutChar(unsigned char data)
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