/*
 * TTCAN.h
 *
 * Created: 05.11.2013 14:56:15
 *  Author: BuiAdmin
 */ 

#ifndef TTCAN_H_
#define TTCAN_H_

#define BASIC_CYCLE		6		//Duration of Basis cycle (ms) (Time marks)
#define MAX_CYCLE		1		//number of Basis cycle (scope of System matrix)
#define TIMEMARKS		6		//number of time mark (= row number of trigger matrix)
#define TRIGGER_INDEX	7		//number of index in Trigger matrix
#define TM				0		// = 1: Time master. = 0 not Timer master


//set up Baud rate (_100KBPS, _125KBPS, _200KBPS,  _250KBPS, _500KBPS, _1MBPS)
//#define BAUDRATE _125KBPS
#define BAUDRATE _500KBPS
#define COMPARE		999		//compared value of FreeRTOS-Timers
#define R_MOB		1		//Choose the second MOb to receive the coming frame
#define S_MOB		0		//Choose the first MOb to transmit the sending frame
#define ID			0x00	//receipt ID for Masking
#define MSK			0x00	//receipt masking

//Baud rate
#define	_100KBPS	0
#define	_125KBPS	1
#define	_200KBPS	2
#define	_250KBPS	3
#define	_500KBPS	4
#define	_1MBPS		5

#define TRUE		1
#define FALSE		0

#ifndef BAUDRATE
#error Keine Baudrate festgelegt!
#endif

struct CAN_message
{
	unsigned int id;		//MSG ID 11 Bit
	unsigned char length;	//DLC Length
	unsigned char data[8];	//Data field 8 Byte
} Angle;

void TTCAN_Init();


#endif /* TTCAN_H_ */