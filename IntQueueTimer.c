#include "FreeRTOS.h"
#include "IntQueueTimer.h"
#include "IntQueue.h"

#define timerINTERRUPT3_FREQUENCY	( 2000UL )
#define timerINTERRUPT4_FREQUENCY	( 2001UL )

void vT3InterruptHandler( void );
void vT4InterruptHandler( void );

void __attribute__( (interrupt(IPL0AUTO), vector(_TIMER_3_VECTOR))) vT3InterruptWrapper( void );
void __attribute__( (interrupt(IPL0AUTO), vector(_TIMER_4_VECTOR))) vT4InterruptWrapper( void );

void vInitialiseTimerForIntQueueTest( void )
{
	/* Timer 1 e folosit pt tick interrupt, timer 2 e folosit pentru high
	frequency interrupt test.  deci in fisier se fol timers 3 si 4. */

	T3CON = 0;
	TMR3 = 0;
	PR3 = ( unsigned short ) ( configPERIPHERAL_CLOCK_HZ / timerINTERRUPT3_FREQUENCY );

	/* Setup timer 3  prioritatea de interrupt deasupra kernel priority. */
	IPC3bits.T3IP = ( configMAX_SYSCALL_INTERRUPT_PRIORITY - 1 );

	IFS0bits.T3IF = 0;

	IEC0bits.T3IE = 1;

	T3CONbits.TON = 1;


	T4CON = 0;
	TMR4 = 0;
	PR4 = ( unsigned short ) ( configPERIPHERAL_CLOCK_HZ / timerINTERRUPT4_FREQUENCY );

	IPC4bits.T4IP = configMAX_SYSCALL_INTERRUPT_PRIORITY;

	IFS0bits.T4IF = 0;

	IEC0bits.T4IE = 1;

	T4CONbits.TON = 1;
}

void vT3InterruptHandler( void )
{
	IFS0CLR = _IFS0_T3IF_MASK;
	portEND_SWITCHING_ISR( xFirstTimerHandler() );
}

void vT4InterruptHandler( void )
{
	IFS0CLR = _IFS0_T4IF_MASK;
	portEND_SWITCHING_ISR( xSecondTimerHandler() );
}


