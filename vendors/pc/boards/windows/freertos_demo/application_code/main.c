/**
 * @file main.c
 * @brief Implements the main function.
 */
 
 /* FreeRTOS include. */
#include <FreeRTOS.h>
#include "task.h"

/* Standard includes. */
#include <signal.h>
#include <conio.h>
#include <setjmp.h>
#include <time.h>

/* Unity includes. */
#include "unity.h"

/* Application includes */
#include "freertos_demo_task.h"

/* Windows-NT VectoredHandler callback function. */
static LONG CALLBACK prvExceptionHandler(_In_ PEXCEPTION_POINTERS ExceptionInfo);

/*
 * Writes trace data to a disk file when the trace recording is stopped.
 * This function will simply overwrite any trace files that already exist.
 */
static void prvSaveTraceFile(void);

/*-----------------------------------------------------------*/

/* Notes if the trace is running or not. */
static BaseType_t xTraceRunning = pdTRUE;

int main(void)
{
    /* Register the Windows VEH for exceptions. */
    AddVectoredExceptionHandler(1, prvExceptionHandler);

    vTraceEnable(TRC_START);

    /* Initialize logging for libraries that depend on it. */
    vLoggingInit(
        pdTRUE,
        pdFALSE,
        pdFALSE,
        0,
        0);

    xTaskCreate(vPrintTask,
        "PrintTask",
        2000,
        NULL,
        tskIDLE_PRIORITY,
        NULL);

    xTaskCreate(vInterruptTask,
        "InterruptTask",
        2000,
        NULL,
        tskIDLE_PRIORITY,
        NULL);

    vTaskStartScheduler();

    for (;;)
    {

    }

    return 0;
}

static LONG CALLBACK prvExceptionHandler(_In_ PEXCEPTION_POINTERS ExceptionInfo)
{
    return EXCEPTION_CONTINUE_EXECUTION;
}

void vApplicationIdleHook(void)
{
    const uint32_t ulMSToSleep = 1;
    const TickType_t xKitHitCheckPeriod = pdMS_TO_TICKS(1000UL);
    static TickType_t xTimeNow, xLastTimeCheck = 0;

    /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
     * to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
     * task.  It is essential that code added to this hook function never attempts
     * to block in any way (for example, call xQueueReceive() with a block time
     * specified, or call vTaskDelay()).  If application tasks make use of the
     * vTaskDelete() API function to delete themselves then it is also important
     * that vApplicationIdleHook() is permitted to return to its calling function,
     * because it is the responsibility of the idle task to clean up memory
     * allocated by the kernel to any task that has since deleted itself. */

     /* _kbhit() is a Windows system function, and system functions can cause
      * crashes if they somehow block the FreeRTOS thread.  The call to _kbhit()
      * can be removed if it causes problems.  Limiting the frequency of calls to
      * _kbhit() should minimize the potential for issues. */
    xTimeNow = xTaskGetTickCount();

    if ((xTimeNow - xLastTimeCheck) > xKitHitCheckPeriod)
    {
        if (_kbhit() != pdFALSE)
        {
            if (xTraceRunning == pdTRUE)
            {
                xTraceRunning = pdFALSE;
                vTraceStop();
                prvSaveTraceFile();
            }
        }

        /* Uncomment the print line to get confirmation that tests are still
         * running if you suspect a previous run resulted in a crash. */
         /* configPRINTF( ( "Running...\n" ) ); /**/
        xLastTimeCheck = xTimeNow;
    }

    /* This is just a trivial example of an idle hook.  It is called on each
     * cycle of the idle task if configUSE_IDLE_HOOK is set to 1 in
     * FreeRTOSConfig.h.  It must *NOT* attempt to block.  In this case the
     * idle task just sleeps to lower the CPU usage. */
    Sleep(ulMSToSleep);
}
/*-----------------------------------------------------------*/



void vAssertCalled( const char * pcFile,
                    uint32_t ulLine )
{
    const uint32_t ulLongSleep = 1000UL;
    volatile uint32_t ulBlockVariable = 0UL;
    volatile char * pcFileName = ( volatile char * ) pcFile;
    volatile uint32_t ulLineNumber = ulLine;

    ( void ) pcFileName;
    ( void ) ulLineNumber;

    printf( "vAssertCalled %s, %ld\n", pcFile, ( long ) ulLine );
    fflush( stdout );

    /* Setting ulBlockVariable to a non-zero value in the debugger will allow
     * this function to be exited. */
    taskDISABLE_INTERRUPTS();
    {
        while( ulBlockVariable == 0UL )
        {
            Sleep( ulLongSleep );
        }
    }
    taskENABLE_INTERRUPTS();
}
/*-----------------------------------------------------------*/

static void prvSaveTraceFile(void)
{
    FILE* pxOutputFile;

    fopen_s(&pxOutputFile, "Trace.dump", "wb");

    if (pxOutputFile != NULL)
    {
        fwrite(RecorderDataPtr, sizeof(RecorderDataType), 1, pxOutputFile);
        fclose(pxOutputFile);
        printf("\r\nTrace output saved to Trace.dump\r\n");
    }
    else
    {
        printf("\r\nFailed to create trace dump file\r\n");
    }
}
/*-----------------------------------------------------------*/