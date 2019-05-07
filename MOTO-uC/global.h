/*
 * Global.h
 *
 * Created: 29.10.2013 15:34:17
 *  Author: BuiAdmin
 */ 


#ifndef _GLOBAL_H_
#define _GLOBAL_H_

//_____ I N C L U D E S ________________________________________________________

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

//_____ D E F I N I T I O N S __________________________________________________

#define NONE			0
#define RX_ANGLE		1
#define RX_PUSB			2

//_____ H A N D L E S __________________________________________________________

// Task handles
xTaskHandle TH_Upright;
xTaskHandle TH_Check_uC;
xTaskHandle TH_Receive_Data;
xTaskHandle TH_SwingUp;

// Queue handles
xQueueHandle DirectionQueue;		// Queue to control the direction of DC motor
xQueueHandle Wake_RxData;		// Queue to trigger task on time schedule

xSemaphoreHandle Wake_Upright;
xSemaphoreHandle Wake_SwingUp;

// Global Vars
int Pen_Angle, Arm_Angle;
int Pen_Up;

#endif /* _GLOBAL_H_ */