/*
 * SEN_Tasks.h
 *
 * Created: 05.11.2013 14:55:17
 *  Author: BuiAdmin
 */ 


#ifndef SEN_TASKS_H_
#define SEN_TASKS_H_

void Data_Preparation(void * pvParameters);

void uC_Check(void * pvParameters);

void PrintOnPC();

extern void Print(char buffer[]);

extern void PutChar(char data);

extern char *itoa(int __val, char *__s, int __radix);

#endif /* SEN_TASKS_H_ */