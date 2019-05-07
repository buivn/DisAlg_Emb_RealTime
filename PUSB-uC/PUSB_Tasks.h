/*
 * PUSB_Tasks.h
 *
 * Created: 04.11.2013 17:37:07
 *  Author: BuiAdmin
 */ 


#ifndef PUSB_TASKS_H_
#define PUSB_TASKS_H_

#define Confidence_Level	2

void PWM_Calculation(void * pvParameters);

void PushedButton_Detection(void * pvParameters);

void Check_uC(void * pvParameters);

#endif /* PUSB_TASKS_H_ */