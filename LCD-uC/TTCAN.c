/*
 * TTCAN.c
 *
 * Created: 25.10.2013 12:13:47
 *  Author: BuiAdmin
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "TTCAN.h"
#include "LCD_Tasks.h"
#include "LCD17x8ASCII.h"
#include "global.h"

// Class variables
unsigned char Cycle_Time = 0;
unsigned int TXtime, RXtime, Ref_time;

// Trigger matrix for TTCAN
extern const unsigned char Trigger[TIMEMARKS][TRIGGER_INDEX] PROGMEM;

//Set-up matrix for CAN Controller (see Data-sheet for detail)
const unsigned char baudrate[6][3] PROGMEM =
{
	{0x12,0x0C,0x37},{0x0E,0x0C,0x37},{0x08,0x0C,0x37},
	{0x06,0x0C,0x37},{0x02,0x0C,0x37},{0x00,0x0C,0x36}
};

//--------------------------------------------------------------------
//Function to CAN-Initialization
//--------------------------------------------------------------------
void CAN_Init(unsigned char bitrate)
{
	unsigned char mob;

	CANGCON |= 0b00000001;		//Reset enabled for the CAN-Controllers
	CANGCON = 0x00;				//Reset the General Control Registers		
	CANGIT  = 0x00;				// Register for interrupt flag
	CANGIE  = 0x00;				// Register for Enable interrupt
	CANSIT2 = 0x00;				//Register for Status of interrupt by MOb
	CANSIT1 = 0x00;
	CANEN1  = 0x00;				// Register for check MOb availability
	CANEN2  = 0x00;				// 1 = in use. 0 = empty (can receive data)
	CANIE1  = 0x00;				// Register for Enable MOb Interrupt
	CANIE2  = 0x00;
	CANTCON = 0x00;				// Prescaler of CAN timer
	for (mob = 0; mob < 15; mob++)		//All MObs is set Null
	{
		CANPAGE = (mob << 4);			// Go to the next MOb
		CANIDT1 = 0x00;					// Reset CAN Identifier Tag Registers
		CANIDT2 = 0x00;					// used to set kind of message (remote, data)
		CANIDT3 = 0x00;
		CANIDT4 = 0x00;
		CANIDM1 = 0x00;					// Reset CAN Identifier Mask Registers
		CANIDM2 = 0x00;					// used to comparison true forced (filter)
		CANIDM3 = 0x00;
		CANIDM4 = 0x00;
		CANSTMOB = 0x00;				// Reset MOb-Status
		CANCDMOB = 0x00;				// Deactivate MOb
	}
		
	DDRD |= 0b10100000;			//Pin D5 output (TXCAN)
	DDRD &= 0b10111111;			//Pin D6 input (RXCAN)
	//Auto-Increment index bit3 = 0, when FIFO
	CANPAGE &= 0b11110111;	
	
	//CAN Bit Timing Register
	CANBT1 = pgm_read_byte(&baudrate[bitrate][0]);	// Set Baud Rate Prescaler
	CANBT2 = pgm_read_byte(&baudrate[bitrate][1]);	// set Re-Synchronization Jump Width and propagation Time Segment
	CANBT3 = pgm_read_byte(&baudrate[bitrate][2]);	// Set Phase Segment 2 and Segment 1, and Sample Point
	
	CANGIE  |= 0b10111000;	    	//ENIT-, ENRX-, ENTX-, ENERR-Interrupt activate
	CANGCON |= 0b00100010;			//TTC mode activate
	while (!(CANGSTA & 0x04));		//wait until CAN-Controller ready
	sei();
}

//--------------------------------------------------------------------
// This function return the MOb which has the running ISR
//--------------------------------------------------------------------
unsigned char Get_Operating_MOb(unsigned int Register)
{
	unsigned char mob_number;
	if (Register == 0) return 0xFF;
	
	// check from the first to the last MOb
	for (mob_number = 0; (Register & 0x01) == 0; ++mob_number) Register >>=1;
	if (mob_number > 14) return (0xFF);
	else return mob_number;
}

//--------------------------------------------------------------------
//This function sets configuration to receive message on the R_MOB Mob
//--------------------------------------------------------------------
void Can_RX()
{
	CANPAGE = (R_MOB << 4);		//Select a MOb to store the coming frame
	
	#if R_MOB>7  	   			//Enable the respective Interrupt for the MOb
	CANIE1 |= (1<<(R_MOB-8));
	#else
	CANIE2 |= (1<<R_MOB);
	#endif
	
	//Set Identifier Tag (allow to receive all frame)
	CANIDT1 = (unsigned char) (ID>>3);
	CANIDT2 = (unsigned char) ((ID<<5) & 0xE0);

	//Set received filter (no filtering)
	CANIDM1  = (unsigned char) (MSK>>3);
	CANIDM2  = (unsigned char) ((MSK<<5) & 0xE0);

	CANCDMOB &= 0b10111111;		// set bit6 = 0
	CANCDMOB |= 0b10000000;		// Enable reception
}

//--------------------------------------------------------------------
//This task sets configuration to transmit a frame by the S_MOB MOb
//--------------------------------------------------------------------
void Can_TX(struct CAN_message msg)
{
	unsigned char i;
	CANPAGE = (S_MOB << 4);			//select MOb storing Data to be sent

	#if S_MOB>7   					//Enable respective Interrupt for the MOb
	CANIE1 |= (1<<(S_MOB-8));
	#else
	CANIE2 |= (1<<S_MOB);
	#endif
	CANCDMOB &= 0b11100000;			//set CAN V2.0A and reset DLC = 0
	CANCDMOB |=	msg.length;			//set the length of Data field

	// Get Identifier of the up-sending frame
	CANIDT1 = (unsigned char) (msg.id>>3);
	CANIDT2 = (unsigned char) ((msg.id<<5) & 0xE0);

	//Write Data on CANMSG (Pointer with Auto-Increment)
	for (i=0; i<msg.length; i++) CANMSG = msg.data[i];

	while (CANGSTA & 0x10);			//Wait until CAN Transmitter available
	
	CANCDMOB &= 0b01111111;			// set bit7 = 0
	CANCDMOB |= 0b01000000;			// Enable transmission
}

void Error()  				// Error handling routine
{
	PutChar('E');
	CANSTMOB &= 0b11100000;	//Delete error flag
}
//--------------------------------------------------------------------
//This task sets a scheduler to transmit and receive frame
//--------------------------------------------------------------------
void TTCAN_Scheduler(void *pvParameters)
{
	struct CAN_message ref_message;
	static unsigned char Cycle_Number=0;
	unsigned char time_mark, base_mark, repeat, type;
	portTickType xLastWakeTime;
	unsigned char queue;
	
	xLastWakeTime = xTaskGetTickCount();	// integer type
	while(1)
	{
		PORTF |= 0b00100000;
		// Create RM to adjust the local time to global time
		if (Cycle_Time == 0 && TM == 1)						//At time mark zero and is Time master
		{
			ref_message.id=0x00;						//RM's ID
			ref_message.length=2;
			ref_message.data[0]=(char) (TXtime >> 8);	//Send the last transmitted time
			ref_message.data[1]=(char) TXtime;
			Can_TX(ref_message);
		}
		// read respective row in trigger matrix		
		time_mark = pgm_read_byte (&Trigger[Cycle_Time][0]);
		base_mark = pgm_read_byte (&Trigger[Cycle_Time][1]);
		repeat = pgm_read_byte (&Trigger[Cycle_Time][2]);
		type = pgm_read_byte (&Trigger[Cycle_Time][3]);

		if (Cycle_Time == time_mark)						//Reached the time mark?
		{
			//Check whether the frame provided in this cycle?
			if (((Cycle_Number - base_mark) % repeat == 0 && Cycle_Number > base_mark) || Cycle_Number == base_mark)
			{
				switch (type)
				{
					case 'R':						//receive message
					case 'r':
						queue = pgm_read_byte (&Trigger[Cycle_Time][5]);
						if (queue != 0) xQueueSend(Wake_RxData, &queue, 0);
						break;
					case 'S':						//send message
					case 's':
					case 'A':
					case 'a':
					default: break;
				}
			}
		}

		if (Cycle_Time == BASIC_CYCLE-1)			//If the end of the basic cycle time reaches to zero
		{
			Cycle_Time = 0;
			if (Cycle_Number < MAX_CYCLE-1) Cycle_Number++;
			else Cycle_Number=0; //Another cycle or the end of the system matrix achieved?
		}
		else Cycle_Time++;
		PORTF &= 0b11011111;
		vTaskDelayUntil(&xLastWakeTime, 1);	// wait 0.5ms
	}
}

//--------------------------------------------------------------------
//This function initialize configuration of all CAN Controller
//--------------------------------------------------------------------
void TTCAN_Init()
{
	CAN_Init(BAUDRATE);
	Can_RX();

	if (xTaskCreate(TTCAN_Scheduler, (signed char *) "TTCAN Schedule", 256, NULL, \
	tskIDLE_PRIORITY + 3, NULL) == pdPASS) Print(" Scheduler ");
}

//--------------------------------------------------------------------
//ISR of CAN Controller
//--------------------------------------------------------------------
ISR(CANIT_vect)
{
	struct CAN_message buffer;
	unsigned char save_canpage, mob, i;
	static unsigned char sync = FALSE;
	static unsigned int last_ref,last_rx;
	static unsigned long int locdiff = 999*BASIC_CYCLE;

	save_canpage = CANPAGE;				//save CANPAGE
	mob = Get_Operating_MOb(CANSIT);	//Get the MOb which its Interrupt is running?
	if(mob==0xFF) return;

	CANPAGE = (mob<<4);				//Go to the MOb
	buffer.id = (unsigned int) (CANIDT1<<8);	// Determine ID
	buffer.id |= (CANIDT2 & 0xE0);
	buffer.id >>= 5;

	if (CANSTMOB & 0b01000000)					//Sending is successful?
	{
		if (buffer.id == 0x00) TXtime = CANTTC; // Get transmitted time of RM
		CANSTMOB &= 0b10111111;					//Delete Transmit-Flag
		CANCDMOB &= 0b10111111;					//disable Sending
	}

	if (CANSTMOB & 0b00100000)					//Receive message successfully?
	{
		buffer.length = (CANCDMOB & 0x0F);		//Determine amount of Databyte
		for (i=0;i<8;i++) buffer.data[i]=0;		//clear buffer before receiving new data

		for (i=0;i<buffer.length;i++) buffer.data[i]=CANMSG; //Receive Data from the memory

		if (buffer.id == 0x00 && TM != 1)		//receive RM and node is not Timemaster
		{
			TCNT1 = CANTIM - CANTTC;			//configure local clock
			RXtime = CANTTC;					//Secure/save received time (SOF) of RM
			Cycle_Time = 1;	
			Ref_time = (buffer.data[0] << 8) + buffer.data[1];
			if (sync)							//Already received a RM?
			{
				locdiff = (RXtime - last_rx);		//Determine local cycle duration
				//Correct compared value of FeeRTOS-Timers for Drift correction
				OCR1A = (unsigned int) (COMPARE*locdiff / (Ref_time - last_ref));
			}
			last_ref = Ref_time;	// Save Ref_time and RX time to calculate the next cycle duration
			last_rx  = RXtime;
			sync = TRUE;			// Flag for 'Reference-Message is received'
		}

		// Angle message?
		if (buffer.id == 0x0001) Angle = buffer;		
		// Pusb_uC message?
		if (buffer.id == 0x0002) Pusb_uC = buffer;

		CANSTMOB &= 0b11011111;					//Delete Receive-Flag
		CANCDMOB|=0b10000000;					//enable again Reception
	}
	if (CANSTMOB & 0b00011111) Error();			//Failure? -> invoke Routine to troubleshooting
	
	CANPAGE = save_canpage;					// restore CANPAGE
}