#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include "partest.h"

#define mainQUEUE_RECEIVE_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define	mainQUEUE_SEND_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )

/* Rata la care datele sunt trimise catre queue.  Valoarea de 200ms este convertita in ticks folosind constanta portTICK_PERIOD_MS . */
#define mainQUEUE_SEND_FREQUENCY_MS			( 200 / portTICK_PERIOD_MS )

/*Numarul de obiecte pe care queue poate sa o tina.  E 1 pentru ca receive task va scoate din obiecte pe masura ce sunt adaugate. Practic, send task va gasi mereu queue goala. */
#define mainQUEUE_LENGTH					( 1 )

/* Valorile sunt trecute la cele doua tasks doar penru a verifica functionalitatea parametrilor de task. */
#define mainQUEUE_SEND_PARAMETER			( 0x1111UL )
#define mainQUEUE_RECEIVE_PARAMETER			( 0x22UL )

/* Perioada pentru blinky software timer.  Se specifica in ms si apoi e convertita in ticks folosind constanta portTICK_PERIOD_MS . */
#define mainBLINKY_TIMER_PERIOD				( 50 / portTICK_PERIOD_MS )

#define mainTASKS_LED						( 0 )
#define mainTIMER_LED						( 1 )

#define mainDONT_BLOCK						( 0 )


static void prvQueueReceiveTask( void *pvParameters );
static void prvQueueSendTask( void *pvParameters );

/* callback function pentru blinky software timer */
static void prvBlinkyTimerCallback( TimerHandle_t xTimer );


void main_blinky( void );

/* queue fol de ambele tasks  */
static QueueHandle_t xQueue = NULL;

void main_blinky( void )
{
TimerHandle_t xTimer;

	/* creaza queue. */
	xQueue = xQueueCreate( mainQUEUE_LENGTH, sizeof( unsigned long ) );

	if( xQueue != NULL )
	{
		/* creaza doua tasks */
		xTaskCreate( prvQueueReceiveTask,					/* functia care implementeaza task. */
					"Rx", 									/* numele atribuit lui task- debug. Nu e fol de kernel. */
					configMINIMAL_STACK_SIZE, 				/* dimensiunea stivei alocata lui task. */
					( void * ) mainQUEUE_RECEIVE_PARAMETER, /* parametrul trimis lui task - pentru verificarea functionalitatii. */
					mainQUEUE_RECEIVE_TASK_PRIORITY, 		/* prioritatea atribuita lui task. */
					NULL );									/* nu e nevoie de task handle*/

		xTaskCreate( prvQueueSendTask, "TX", configMINIMAL_STACK_SIZE, ( void * ) mainQUEUE_SEND_PARAMETER, mainQUEUE_SEND_TASK_PRIORITY, NULL );

		xTimer = xTimerCreate( 	"Blinky",					/* nume. doar debug. */
								( mainBLINKY_TIMER_PERIOD ),/* perioada timer. */
								pdTRUE,						/* auto-reload timer, deci xAutoReload e setat la pdTRUE. */
								( void * ) 0,				/* ID nu e fol. poate sa fie orice. */
								prvBlinkyTimerCallback		/* callback function care verifica statusul tuturor task-urilor*/
							);

		if( xTimer != NULL )
		{
			xTimerStart( xTimer, mainDONT_BLOCK );
		}

		vTaskStartScheduler();
	}

	for( ;; );
}

static void prvQueueSendTask( void *pvParameters )
{
TickType_t xNextWakeTime;
const unsigned long ulValueToSend = 100UL;

	configASSERT( ( ( unsigned long ) pvParameters ) == mainQUEUE_SEND_PARAMETER );

	xNextWakeTime = xTaskGetTickCount();

	for( ;; )
	{

		vTaskDelayUntil( &xNextWakeTime, mainQUEUE_SEND_FREQUENCY_MS );

		xQueueSend( xQueue, &ulValueToSend, 0U );
	}
}

static void prvQueueReceiveTask( void *pvParameters )
{
unsigned long ulReceivedValue;

	configASSERT( ( ( unsigned long ) pvParameters ) == mainQUEUE_RECEIVE_PARAMETER );

	for( ;; )
	{
		xQueueReceive( xQueue, &ulReceivedValue, portMAX_DELAY );

		if( ulReceivedValue == 100UL )
		{
			vParTestToggleLED( mainTASKS_LED );
			ulReceivedValue = 0U;
		}
	}
}
/*-----------------------------------------------------------*/

static void prvBlinkyTimerCallback( TimerHandle_t xTimer )
{
	vParTestToggleLED( mainTIMER_LED );
}

