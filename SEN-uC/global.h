/*
 * Global.h
 *
 * Created: 29.11.2012 17:50:15
 *  Author: operator
 */ 

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

//_____ I N C L U D E S ________________________________________________________

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

//_____ D E F I N I T I O N S __________________________________________________

#define NONE		0
#define PR_DATA		3

//_____ H A N D L E S __________________________________________________________

// Queue handles
//xQueueHandle GetDataSignal;
// Tasks handles
xTaskHandle  TH_Get_Sensor_Data;
xTaskHandle  TH_Check_uC;

xSemaphoreHandle WakeTaskSe;


//_____ V A R I A B L E S ______________________________________________________

int  Pendulum_axis;		// 2000 per rotation
int  Arm_axis;			// 2000 per rotation

int Arm_vel;
int Pen_vel;
		
#endif /* _GLOBAL_H_ */