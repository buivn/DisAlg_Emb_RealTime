/*
 * LCD_Tasks.c
 *
 * Created: 25.10.2013 12:17:56
 *  Author: BuiAdmin
 */ 

//_____ I N C L U D E S ________________________________________________________
#include "USART.h"
#include "LCD_Tasks.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "global.h"
#include "LCD17x8ASCII.h"
#include "TTCAN.h"

char Motor_Power;
unsigned char switcher1=TRUE, switcher2=FALSE, switch3=FALSE, switch4=TRUE;

void Data_Reception(void* pvParameters)
{
	// Local variables
	int buffer1=0, buffer2, buffer3;
	unsigned char Check_Slot = 0, i=0;
	// variables to store the local time in string
	unsigned char Array1[7], Array2[7];
	// variables to get the local time
	portTickType xLastWakeTime1;
	for (;;)
	{
		// Wait for a CAN message?
		if (xQueueReceive(Wake_RxData, &Check_Slot, portMAX_DELAY))
		{		
			PORTF |= 0b10000000;
			// if receiving a Angles message
			if (Check_Slot == RX_ANGLE)
			{
				// receive data from the AngleQueue
				buffer1 = (Angle.data[0] << 8) + Angle.data[1];
				buffer2 = (Angle.data[2] << 8) + Angle.data[3];

				// Convert the angle to degree
				DPen_Angle = (float) (buffer1*9/50.0);
				DArm_Angle = (float) (buffer2*9/50.0);
				
				if (i > 2)
				{
					i = 0;
					// print the data on PC via Hterm
					xLastWakeTime1 = xTaskGetTickCount();
					// convert local time value to a string (6 elements)
					itoa(xLastWakeTime1, Array1, 10);	
					// convert Pendulum angles a string (6 elements)
					//itoa(buffer1, Array2, 10);
					// convert Arm angle to a string
					itoa(buffer2, Array2, 10);
					Array1[6] = 0;
					Array2[6] = 0;
					// print the local time
					Print(Array1); PutChar(' ');
					// print the Pendulum position
					Print(Array2); 
					// make a newline
					PutChar(10);
				}
				i ++;
			}
			// if receiving Pusb_uC Message
			if (Check_Slot == RX_PUSB)
			{
				if (Pusb_uC.data[2] == 1)	// Button 3: Change input
				{
					switcher1 = FALSE;
				}
				if (Pusb_uC.data[3] == 1) 	// Button 6: finish changing input
				{
					switcher1 = TRUE;
				}
				if (Pusb_uC.data[0] == 1)	// Button 1: Start application
				{
					switcher2 = TRUE;
				}
				if (Pusb_uC.data[1] == 1) 	// Button 2: stop application
				{
					switcher2 = FALSE;
				}
				buffer3 = (int) ((Pusb_uC.data[5] << 8 ) + Pusb_uC.data[6]);
				Motor_Power = (char) (buffer3/7.99);
				if (Motor_Power < 0) Motor_Power = -Motor_Power;
				if ((buffer1 > -910) && (buffer1 < 910)) Motor_Power = 25;
				if (!switcher2) Motor_Power = 0;
			}
			PORTF &= 0b01111111;
		}	// end of xQueueReceive...

	}	// end of for loop
}	// end of task Convert_Angle

//----------------------------------------------------------
// This task show all the information on the LCD
//----------------------------------------------------------
void LCD_Display( void * pvParameters)
{
	int  p=0, e=0;
	unsigned char sign, l=0;
	char t,h,z, k, f, n;
	float m=0, buffer1, buffer2;
	for (;;) // Start of infinity loop
	{	
		if (switcher1)
		{
				Csr();
				Dpstr(0,0, "Welcome to IRP");
				// Show the basic message
				Dpstr(1,0,"State: ");	// Show the word "State:"		
				// show the state
				if ((!switcher2) && (!switch4)) Dpstr(1, 7, "Stop");
				if ((!switcher2) && (switch4)) Dpstr(1, 7, "Power On");
				if ((switcher2)  && (!switch3)) Dpstr(1,7, "Swing Up");
				if ((switcher2)  && (switch3)) Dpstr(1,7, "Stable");
				Dpstr(2,0,"P_Ang:");	// Show the word "P_Ang:"
				Dpstr(3,0,"A_Ang:");	// Show the word "A_Ang:"
				// show two angle values
				if ((DPen_Angle < -163)||(DPen_Angle > 163)) switch3 = TRUE;
				else switch3 = FALSE;
				if (DPen_Angle == 0) switch4 = TRUE;
				else switch4 = FALSE;			
				// Pendulum Angle on LCD
				if (DPen_Angle < 0)	{
					sign = '-';
					buffer1 = -DPen_Angle;
				}
				else  {
					sign = '+';
					buffer1 = DPen_Angle;
				}
				// Separate into two parts
				p = (int) buffer1;
				m = buffer1 - (float) p;
				// integer part
				t = (p / 100) +'0'; p = p - ((p / 100) * 100);
				h = (p / 10) +'0';  p = p - ((p / 10) * 10);
				z = (p % 10) +'0';
				// decimal/ fractional part
				m = m*100;
				e = (int) (m);
				f = (e / 10) + '0'; e = e - ((e / 10) * 10);
				n = (e % 10) + '0';
				// Show Integer part
				Dpc(2, 6, sign); Dpc(2, 7, t); Dpc(2, 8, h); Dpc(2, 9, z);
				// Show decimal part
				Dpc(2, 10, '.'); Dpc(2, 11, f); Dpc(2, 12, n);		
				// Arm Angle on LCD
				if (DArm_Angle < 0) {
					sign = '-';
					buffer2 = -DArm_Angle;
				}
				else  {
					sign = '+';
					buffer2 = DArm_Angle;
				}			
				// Separate into two parts
				p = (int) buffer2;
				m = buffer2 - (float) p;
				// integer part
				t = (p / 100) +'0'; p = p - ((p / 100) * 100);
				h = (p / 10) +'0';  p = p - ((p / 10) * 10);
				z = (p % 10) +'0';			
				// decimal/ fractional part
				m = m *100;
				e = (int) (m);
				f = (e / 10) + '0'; e = e-((e/10)*10);
				n = (e % 10) + '0';				
				// Show Integer part
				Dpc(3, 6, sign); Dpc(3, 7, t); Dpc(3, 8, h); Dpc(3, 9, z);
				// Show decimal part
				Dpc(3, 10, '.'); Dpc(3, 11, f); Dpc(3, 12, n);	
				Dpstr(4,0,"PWM  : ");//Show the word "PWM:"
				// Show Motor Power value on LCD
				if (Motor_Power > 9)h = (Motor_Power/10) +'0'; 
				else h = ' ';
				Motor_Power = Motor_Power - ((Motor_Power/10)*10);
				e = (Motor_Power % 10) +'0';
				Dpc (4, 7, h); Dpc (4, 8, e); Dpc (4, 9, '%');
				Dpstr(5,0,"Ref  : ");	// Show the word "Ref:"
				// show the Nominal Input value
				k = Pusb_uC.data[4];
				if (k < 0) {
					sign = '-';
					k = -k;
				}
				else  sign = '+';
				z = (k / 10) +'0';	 k = k - ((k/10) * 10);
				e = (k % 10) +'0';
				Dpc (5, 6, sign);
				Dpc (5, 7, z);
				Dpc (5, 8, e);
				Dpstr(5, 10, "deg.");		
				// show the Start/Stop Button
				if(switcher2)	Dpstr(6, 0, "B.2  :Stop");
				else	Dpstr(6, 0, "B.1  :Start");
				// show the Input Button
				Dpstr(7,0, "B.3  :Change Ref");		// show the sentence B3 to change input	
		}
		else	// of if(switch1)..
		{
				Csr();
				Dpstr(0,0, "State:Change Ref");
				// Show the word "A_Ang:"
				Dpstr(1,0,"A_Ang:");
				// Arm Angle on LCD
				if (DArm_Angle < 0) {
					sign = '-';
					buffer2 = -DArm_Angle;
				}
				else {
					sign = '+';
					buffer2 = DArm_Angle;
				}
				// Separate into two parts
				p = (int) buffer2;
				m = buffer2 - (float) p;
				// integer part
				t = (p / 100) +'0'; p = p - ((p / 100) * 100);
				h = (p / 10) +'0';  p = p - ((p / 10) * 10);
				z = (p % 10) +'0';
				// decimal/ fractional part
				m = m *100;
				e = (int) (m);
				f = (e / 10) + '0'; e = e-((e/10)*10);
				n = (e % 10) + '0';	
				// Show Integer part
				Dpc(1, 6, sign); Dpc(1, 7, t); Dpc(1, 8, h); Dpc(1, 9, z);
				// Show decimal part
				Dpc(1, 10, '.'); Dpc(1, 11, f); Dpc(1, 12, n);
				Dpstr(2,0,"Ref: ");				// Show the word "Ref :"
				Dpstr(3,0,"B.4:Increase Ref.");		// Show the word "B.4:Increase Ref"
				Dpstr(4,0,"B.5:Decrease Ref.");		// Show the word "B.5: Decrease Ref :"
				Dpstr(5,0,"B.6:Finish");
				// shown the Ref. input
				k = Pusb_uC.data[4];
				if (k < 0) 	{
					sign = '-';
					k = -k;
				}
				else  sign = '+';
				z = (k / 10) +'0';	 k = k - ((k/10) * 10);
				e = (k % 10) +'0';
				Dpc (2, 5, sign);
				Dpc (2, 6, z);
				Dpc (2, 7, e);
				Dpstr(2, 9, "deg.");
				Dpstr(6,0,"B.7:Oscillation");
				Dpstr(7,0,"B.8:Stop Oscill.");			
		}
		// check the LCD
		l++;
		if (l>3) l = 0;
		switch (l % 4)
		{
			case 0: Dpc (7, 17, '/' ); break;
			case 1: Dpc (7, 17, '-'); break;
			case 2: Dpc (7, 17, '\\'); break;
			case 3: Dpc (7, 17, '|'); break;
		} // end switch
		vTaskDelay (250);
	} // end of infinity loop
} // end of task ShowOn_LCD

//----------------------------------------------------------
void  uC_Check( void * pvParameters)
{ 
	unsigned char  z = 0;
	// infinity loop
	for (;;)
	{
		vTaskDelay (300); // Wait 100ms
		z++;
		switch (z % 4)
		{
			case 0: Dpc (7, 17, '/' ); break;
			case 1: Dpc (7, 17, '-'); break;
			case 2: Dpc (7, 17, '\\'); break;
			case 3: Dpc (7, 17, '|'); break;
		} // end switch
	} // end of infinity loop
} //end of task Check_uC
//----------------------------------------------------------

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