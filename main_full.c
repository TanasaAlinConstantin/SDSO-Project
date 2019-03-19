#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include "partest.h"
#include "blocktim.h"
#include "flash_timer.h"
#include "semtest.h"
#include "GenQTest.h"
#include "QPeek.h"
#include "lcd.h"
#include "timertest.h"
#include "IntQueue.h"


/*Perioada dupa ce check timer expira, in ms. ms sunt convertite la echivalentul in ticks folosind constanta portTICK_PERIOD_MS. */
#define mainCHECK_TIMER_PERIOD_MS			( 3000UL / portTICK_PERIOD_MS )

#define mainERROR_CHECK_TIMER_PERIOD_MS 	( 200UL / portTICK_PERIOD_MS )

/* prioritatile aplicatiilor. */
#define mainSEM_TEST_PRIORITY				( tskIDLE_PRIORITY + 1 )
#define mainBLOCK_Q_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainCOM_TEST_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainINTEGER_TASK_PRIORITY           ( tskIDLE_PRIORITY )
#define mainGEN_QUEUE_TASK_PRIORITY			( tskIDLE_PRIORITY )

#define mainDONT_BLOCK						( 0 )

#define mainMAX_STRING_LENGTH				( 20 )

#define mainTEST_INTERRUPT_FREQUENCY		( 20000 )

#define mainEXPECTED_CLOCKS_BETWEEN_INTERRUPTS ( ( configCPU_CLOCK_HZ >> 1 ) / mainTEST_INTERRUPT_FREQUENCY )

#define mainNS_PER_CLOCK ( ( unsigned long ) ( ( 1.0 / ( double ) ( configCPU_CLOCK_HZ >> 1 ) ) * 1000000000.0 ) )

#ifdef PIC32_STARTER_KIT
	#define mainNUM_FLASH_TIMER_LEDS			( 2 )
	#define mainCHECK_LED						( 2 )
#else
	#define mainNUM_FLASH_TIMER_LEDS			( 3 )
	#define mainCHECK_LED						( 7 )
#endif


static void prvCheckTimerCallback( TimerHandle_t xTimer );


static void prvSetupHighFrequencyTimerTest( TimerHandle_t xTimer );


static void prvRegTestTask1( void *pvParameters );
static void prvRegTestTask2( void *pvParameters );


static QueueHandle_t xLCDQueue;


volatile unsigned long ulRegTest1Cycles = 0, ulRegTest2Cycles = 0;


int main_full( void )
{
TimerHandle_t xTimer = NULL;

	vStartLEDFlashTimers( mainNUM_FLASH_TIMER_LEDS );
    vCreateBlockTimeTasks();
    vStartSemaphoreTasks( mainSEM_TEST_PRIORITY );
    vStartGenericQueueTasks( mainGEN_QUEUE_TASK_PRIORITY );
    vStartQueuePeekTasks();
	vStartInterruptQueueTasks();

	xTaskCreate( prvRegTestTask1, "Reg1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL );
	xTaskCreate( prvRegTestTask2, "Reg2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL );

	xTimer = xTimerCreate( 	"CheckTimer", ( mainCHECK_TIMER_PERIOD_MS ), pdTRUE, ( void * ) 0,	prvCheckTimerCallback);

	if( xTimer != NULL )
	{
		xTimerStart( xTimer, mainDONT_BLOCK );
	}

	xTimer = xTimerCreate( "HighHzTimerSetup", 1, pdFALSE, ( void * ) 0, prvSetupHighFrequencyTimerTest );
	if( xTimer != NULL )
	{
		xTimerStart( xTimer, mainDONT_BLOCK );
	}

	vTaskStartScheduler();

	for( ;; );
}

static void prvRegTestTask1( void *pvParameters )
{
extern void vRegTest1( volatile unsigned long * );

	( void ) pvParameters;

	vRegTest1( &ulRegTest1Cycles );

	vTaskDelete( NULL );
}

static void prvRegTestTask2( void *pvParameters )
{
extern void vRegTest2( volatile unsigned long * );

	( void ) pvParameters;

	vRegTest2( &ulRegTest2Cycles );

	vTaskDelete( NULL );
}

static void prvCheckTimerCallback( TimerHandle_t xTimer )
{
static long lChangedTimerPeriodAlready = pdFALSE;
static unsigned long ulLastRegTest1Value = 0, ulLastRegTest2Value = 0;

static char cStringBuffer[ mainMAX_STRING_LENGTH ];

extern unsigned long ulHighFrequencyTimerInterrupts;
static xLCDMessage xMessage = { ( 200 / portTICK_PERIOD_MS ), cStringBuffer };

	if( ulLastRegTest1Value == ulRegTest1Cycles )
	{
		xMessage.pcMessage = "Error: Reg test2";
	}
	ulLastRegTest1Value = ulRegTest1Cycles;


	if( ulLastRegTest2Value == ulRegTest2Cycles )
	{
		xMessage.pcMessage = "Error: Reg test3";
	}
	ulLastRegTest2Value = ulRegTest2Cycles;


	if( xAreGenericQueueTasksStillRunning() != pdTRUE )
	{
		xMessage.pcMessage = "Error: Gen Q";
	}
	else if( xAreQueuePeekTasksStillRunning() != pdTRUE )
	{
		xMessage.pcMessage = "Error: Q Peek";
	}
	else if( xAreBlockTimeTestTasksStillRunning() != pdTRUE )
	{
		xMessage.pcMessage = "Error: Blck time";
	}
	else if( xAreSemaphoreTasksStillRunning() != pdTRUE )
	{
		xMessage.pcMessage = "Error: Sem test";
	}
	else if( xAreIntQueueTasksStillRunning() != pdTRUE )
	{
		xMessage.pcMessage = "Error: Int queue";
	}

	if( xMessage.pcMessage != cStringBuffer )
	{
		if( lChangedTimerPeriodAlready == pdFALSE )
		{
			lChangedTimerPeriodAlready = pdTRUE;

			xTimerChangePeriod( xTimer, ( mainERROR_CHECK_TIMER_PERIOD_MS ), mainDONT_BLOCK );
		}
	}
	else
	{
		sprintf( cStringBuffer, "Pass %u", ( unsigned int ) ulHighFrequencyTimerInterrupts );
	}

	vParTestToggleLED( mainCHECK_LED );
}

static void prvSetupHighFrequencyTimerTest( TimerHandle_t xTimer )
{
	vSetupTimerTest( mainTEST_INTERRUPT_FREQUENCY );
}

