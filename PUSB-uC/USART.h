/*
 * USART.h
 *
 * Created: 25.08.2011 14:45:52
 *  Author: operator
 */ 


#ifndef USART_H_
#define USART_H_

extern void  USART_Init ( unsigned int  baud );

extern void  USART_Transmit ( unsigned char  data );

extern unsigned char  USART_Receive ( void );

#endif /* USART_H_ */