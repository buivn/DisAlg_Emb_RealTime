/*
 * MOTO_Tasks.h
 *
 * Created: 29.10.2013 15:33:52
 *  Author: BuiAdmin
 */ 


#ifndef MOTO_TASKS_H_
#define MOTO_TASKS_H_


void Data_Reception(void *pvParameters);

void Upright_Controller(void * pvParameters);

void SwingUp_Controller(void * pvParameters);

void PWM_Test(void * pvParameters);

//----------------------------------------------------------
// This function outputs one Character via UART
//----------------------------------------------------------
extern void PutChar(char data);

//----------------------------------------------------------
// This function outputs one String via UART
//----------------------------------------------------------
extern void Print(char buffer[]);

#endif /* MOTO_TASKS_H_ */