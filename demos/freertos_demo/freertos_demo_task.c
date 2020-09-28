#include "freertos_demo_task.h"
#include "unistd.h"
#include "portmacro.h"
#include "FreeRTOSConfig.h"
#include "projdefs.h"
#include "semphr.h"

/*
 * The ISR for the simulated interrupt
 */
static unsigned long prvInterruptTaskHandler( void );

static volatile SemaphoreHandle_t xPrintSem;
static volatile TickType_t xTrigLatency;

void vInterruptTask (void *pvParameters)
{	
	/*
	 * Create the xPrintSem semaphore if necessary
	 * and release it.
	 */
	if (xPrintSem == NULL)
	{
		xPrintSem = xSemaphoreCreateBinary();
		xSemaphoreGive(xPrintSem);
	}
	
	vPortSetInterruptHandler(demotaskINTERRUPT_NUMBER, prvInterruptTaskHandler);

	for(;;)
	{
		/* Wait here for timer to expire */
		vTaskDelay(pdMS_TO_TICKS(100UL)); //100 ms wait

		/* Generate the intterupt */
		vPortGenerateSimulatedInterrupt(demotaskINTERRUPT_NUMBER);
	}
}

void vPrintTask (void *pvParameters)
{
	TickType_t xUpTime;
	uint32_t ulUpTime,ulMinutes,ulSeconds,ulMSeconds;
	
	/*
	 * Create the xPrintSem semaphore if necessary
	 * and release it.
	 */
	if (xPrintSem == NULL)
	{
		xPrintSem = xSemaphoreCreateBinary();
		xSemaphoreGive(xPrintSem);
	}
	
	for(;;)
	{
		/*
		 * Wait for and take the xPrintSem semaphore.
		 * Included safety timeout as a "best practices" measure.
		 * Wait, at most, 200ms.
		 */
		if (xSemaphoreTake(xPrintSem, pdMS_TO_TICKS(200UL)) == pdTRUE)
		{
			/*uncomment the next line to test interrupt-print latency*/
			//vTaskDelay(pdMS_TO_TICKS(10UL));

			xUpTime = xTaskGetTickCount(); //get the current tick count

			xTrigLatency = xUpTime - xTrigLatency; //calculate interrept-print latency

			ulUpTime = (uint32_t) xUpTime / (configTICK_RATE_HZ / 1000UL); //convert ticks to milliseconds
			
			ulMSeconds = ulUpTime % 1000UL;            //extract milliseconds
			ulSeconds = (ulUpTime % 60000UL) / 1000UL; //extract seconds
			ulMinutes = ulUpTime / 60000UL;            //extract minutes
			
			//print the up-time and latency
			printf("up-time: %u:%02u.%03u\tinterrupt-print latency: %4u ticks\n", ulMinutes, ulSeconds, ulMSeconds, xTrigLatency);
		}
		else
		{
			/*
			 * Time out expired. No action to take at this time.
			 */
		}
	}
}

static unsigned long prvInterruptTaskHandler( void )
{
	static BaseType_t xHigherPriorityTaskWoken;
	
	//Get the current tick value for the latency calculation.
	xTrigLatency = xTaskGetTickCountFromISR();

	//Release the xPrintSem semaphore.
	xSemaphoreGiveFromISR( xPrintSem, &xHigherPriorityTaskWoken );
	
	//If the semaphore unblocked a higher priority task yield on exit.
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	
	return pdTRUE;
}