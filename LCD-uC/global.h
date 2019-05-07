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
#define RX_ANGLE	1
#define RX_PUSB		2

//_____ H A N D L E S __________________________________________________________

// Tasks handles for LCD_Tasks
xTaskHandle	TH_Check_uC;
xTaskHandle TH_ShowOn_LCD;
xTaskHandle TH_Receive_Data;

// Queue Handles
xQueueHandle Wake_RxData;		// Queue to trigger task on time schedule

float DPen_Angle, DArm_Angle;

#endif /* _GLOBAL_H_ */
