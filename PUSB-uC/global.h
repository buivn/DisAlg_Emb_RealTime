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

//_____ H A N D L E S __________________________________________________________

// Queue handles
xQueueHandle NomInputQueue;
xQueueHandle Choose_Frame_Queue;


// Semaphore handle
xSemaphoreHandle WakeTaskSe;

// Tasks handles
xTaskHandle TH_C_PWM;
xTaskHandle TH_PushedButton;

#endif /* _GLOBAL_H_ */