#include "FreeRTOS.h"
#include "task.h"
#include "partest.h"
#include "ConfigPerformance.h"

#pragma config FPLLMUL = MUL_20, FPLLIDIV = DIV_2, FPLLODIV = DIV_1, FWDTEN = OFF
#pragma config POSCMOD = HS, FNOSC = PRIPLL, FPBDIV = DIV_2
#pragma config CP = OFF, BWP = OFF, PWP = OFF

#if defined(__32MX460F512L__)
	#pragma config UPLLEN = OFF
#elif defined(__32MX795F512L__)
	#pragma config UPLLEN = OFF
	#pragma config FSRSSEL = PRIORITY_7
#endif

/* seteaza 1 pentru blinking demo  */
#define mainCREATE_SIMPLE_BLINKY_DEMO_ONLY	0 

static void prvSetupHardware( void );

/*
 * main_blinky() este folosit cand mainCREATE_SIMPLE_BLINKY_DEMO_ONLY e setat 1.
 * main_full() este folosit cand mainCREATE_SIMPLE_BLINKY_DEMO_ONLY e setat 0.
 */
extern void main_blinky( void );
extern void main_full( void );

/*
 * creaza task-urile de demo demo tasks cand pornesti scheduler.
 */
int main( void )
{
	/* preg. hw. */
	prvSetupHardware();

	#if mainCREATE_SIMPLE_BLINKY_DEMO_ONLY == 1
	{
		main_blinky();
	}
	#else
	{
		main_full();
	}
	#endif

	return 0;
}

static void prvSetupHardware( void )
{
	/* config hw. */
	vHardwareConfigurePerformance();

	/* setup pentru folosirea controllerului de intreruperi. */
	vHardwareUseMultiVectoredInterrupts();

	portDISABLE_INTERRUPTS();

	/* Setup pentru IO digital pentru LED-uri. */
	vParTestInitialise();
}

void vApplicationMallocFailedHook( void )
{
	taskDISABLE_INTERRUPTS();
	for( ;; );
}

void vApplicationIdleHook( void )
{

}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	taskDISABLE_INTERRUPTS();
	for( ;; );
}

void vApplicationTickHook( void )
{

}

void _general_exception_handler( unsigned long ulCause, unsigned long ulStatus )
{
	for( ;; );
}

void vAssertCalled( const char * pcFile, unsigned long ulLine )
{
volatile unsigned long ul = 0;

	( void ) pcFile;
	( void ) ulLine;

	__asm volatile( "di" );
	{
		/* seteaza ul la o valoare non-zero folosind debugger pentru a iesi de aici . */
		while( ul == 0 )
		{
			portNOP();
		}
	}
	__asm volatile( "ei" );
}
