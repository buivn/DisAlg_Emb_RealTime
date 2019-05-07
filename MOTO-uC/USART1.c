/*
 * USART1.c
 *
 * Created: 25.08.2011 14:06:17
 *  Author: operator
 */ 
// Compiler includes
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <math.h>
// FreeRTOS includes
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
// Modul includes
#include "global.h"
#include "USART.h"
// Module initialized vars

//-------------------------------------------------------------

void USART_Init( unsigned int baud )
{	/* Set baud rate */
	UBRR0H = (unsigned char)(baud>>8);
	UBRR0L = (unsigned char)baud;
	/* Douple baud rate */
	UCSR0A = UCSR0A | (1<<U2X0);
	/* Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: 8data, 2stop bit */
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);
	}
//-------------------------------------------------------------

void USART_Transmit( unsigned char data )
{ /* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) )
	;
	/* Put data into buffer, sends the data */
	UDR0 = data;
}
//-------------------------------------------------------------
unsigned char USART_Receive( void )
{ /* Wait for data to be received */
	while ( !(UCSR0A & (1<<RXC0)) )
	;
	/* Get and return received data from buffer */
	return UDR0;
}
//-------------------------------------------------------------
