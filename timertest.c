#include "FreeRTOS.h"

#define timerMAX_COUNT				0xffff


void __attribute__( (interrupt(IPL0AUTO), vector(_TIMER_2_VECTOR))) vT2InterruptWrapper( void );


/* Incrementat la fiecare 20,000 interuperi*/
unsigned long ulHighFrequencyTimerInterrupts = 0;

/* frecventa la care timerul se intrerupe */
static unsigned long ulFrequencyHz;


void vSetupTimerTest( unsigned short usFrequencyHz )
{
	ulFrequencyHz = ( unsigned long ) usFrequencyHz;

	T2CON = 0;
	TMR2 = 0;

	PR2 = ( unsigned short ) ( ( configPERIPHERAL_CLOCK_HZ / ( unsigned long ) usFrequencyHz ) - 1 );

	IPC2bits.T2IP = ( configMAX_SYSCALL_INTERRUPT_PRIORITY + 1 );

	IFS0bits.T2IF = 0;

	IEC0bits.T2IE = 1;

	T2CONbits.TON = 1;
}

void vT2InterruptHandler( void )
{
static unsigned long ulCalls = 0;

	++ulCalls;
	if( ulCalls >= ulFrequencyHz )
	{
		ulHighFrequencyTimerInterrupts++;
		ulCalls = 0;
	}

	IFS0CLR = _IFS0_T2IF_MASK;
}


