#ifndef _FREERTOS_DEMO_TASK_H_
#define _FREERTOS_DEMO_TASK_H_

/* The simulated interrupt to be triggered.
For the freertos Windows port only */
#define  demotaskINTERRUPT_NUMBER 3

/*
 * Generates an interrupt periodically. Takes the place
 * of a hardware-implemented periodic interrupt timer peripheral
 * for the freertos windows port.
 */
void vInterruptTask (void *pvParameters);

/*
 * Prints the current up-time to stdout.
 * Waits for vInterruptTask to release semaphore.
 */
void vPrintTask (void *pvParameters);

#endif //_FREERTOS_DEMO_TASK_H_