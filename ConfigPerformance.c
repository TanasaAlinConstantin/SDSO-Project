#include "FreeRTOS.h"

#include "ConfigPerformance.h"

#define hwCHECON_PREFEN_BITS	  		( 0x03UL << 0x04UL )
#define hwCHECON_WAIT_STAT_BITS			( 0x07UL << 0UL )
#define hwMAX_FLASH_SPEED		  		( 30000000UL )
#define hwPERIPHERAL_CLOCK_DIV_BY_2		( 1UL << 0x13UL )
#define hwUNLOCK_KEY_0					( 0xAA996655UL )
#define hwUNLOCK_KEY_1					( 0x556699AAUL )
#define hwLOCK_KEY						( 0x33333333UL )
#define hwGLOBAL_INTERRUPT_BIT			( 0x01UL )
#define hwBEV_BIT 						( 0x00400000 )
#define hwEXL_BIT 						( 0x00000002 )
#define hwIV_BIT 						( 0x00800000 )

/* seteaza starea flash wait states pentru a configura viteza ceasului de CPU . */
static void prvConfigureWaitStates( void );

/* Divizor cu 2 pentru bus periferic */
static void prvConfigurePeripheralBus( void );

/* cache.*/
static void __attribute__ ((nomips16)) prvKSeg0CacheOn( void );

/*-----------------------------------------------------------*/

void vHardwareConfigurePerformance( void )
{
unsigned long ulStatus;
#ifdef _PCACHE
	unsigned long ulCacheStatus;
#endif

	ulStatus = _CP0_GET_STATUS();
	_CP0_SET_STATUS( ulStatus & ~hwGLOBAL_INTERRUPT_BIT );

	prvConfigurePeripheralBus();
	prvConfigureWaitStates();

	/* dezactiveaza DRM wait state. */
	BMXCONCLR = _BMXCON_BMXWSDRM_MASK;

	#ifdef _PCACHE
	{
		ulCacheStatus = CHECON;

		/* toti bitii PREFEN sunt setati, nu e nevoie de clear. */
		ulCacheStatus |= hwCHECON_PREFEN_BITS;

		/*scrie noua valoare. */
		CHECON = ulCacheStatus;
		prvKSeg0CacheOn();
	}
	#endif

	/* reseteaza status register la valoarea originala */
	_CP0_SET_STATUS( ulStatus );
}
/*-----------------------------------------------------------*/

void vHardwareUseMultiVectoredInterrupts( void )
{
unsigned long ulStatus, ulCause;
extern unsigned long _ebase_address[];

	/* citeste status curent. */
	ulStatus = _CP0_GET_STATUS();

	/* dezactiveaza intreruperi. */
	ulStatus &= ~hwGLOBAL_INTERRUPT_BIT;

	/* Seteaza BEV bit. */
	ulStatus |= hwBEV_BIT;

	/* scrie status inapoi. */
	_CP0_SET_STATUS( ulStatus );

	/* Setup EBase. */
	_CP0_SET_EBASE( ( unsigned long ) _ebase_address );
	
	/* spatiul de vectori de 0x20 bytes. */
	_CP0_XCH_INTCTL( 0x20 );

	/* seteaza IV bit in CAUSE register. */
	ulCause = _CP0_GET_CAUSE();
	ulCause |= hwIV_BIT;
	_CP0_SET_CAUSE( ulCause );

	/* Clear BEV si EXL bits in status. */
	ulStatus &= ~( hwBEV_BIT | hwEXL_BIT );
	_CP0_SET_STATUS( ulStatus );

	/* Seteaza MVEC bit. */
	INTCONbits.MVEC = 1;
	
	/*activeaza intreruperile din nou. */
	ulStatus |= hwGLOBAL_INTERRUPT_BIT;
	_CP0_SET_STATUS( ulStatus );
}
/*-----------------------------------------------------------*/

static void prvConfigurePeripheralBus( void )
{
unsigned long ulDMAStatus;
__OSCCONbits_t xOSCCONBits;

	/*deblocheaza dupa suspendare. */
	ulDMAStatus = DMACONbits.SUSPEND;
	if( ulDMAStatus == 0 )
	{
		DMACONSET = _DMACON_SUSPEND_MASK;

		/* asteapta suspendarea. */
		while( DMACONbits.SUSPEND == 0 );
	}

	SYSKEY = 0;
	SYSKEY = hwUNLOCK_KEY_0;
	SYSKEY = hwUNLOCK_KEY_1;

	xOSCCONBits.w = OSCCON;
	xOSCCONBits.PBDIV = 0;
	xOSCCONBits.w |= hwPERIPHERAL_CLOCK_DIV_BY_2;

	OSCCON = xOSCCONBits.w;

	xOSCCONBits.w = OSCCON;

	SYSKEY = hwLOCK_KEY;

	if( ulDMAStatus == 0 )
	{
		DMACONCLR=_DMACON_SUSPEND_MASK;
	}
}

static void prvConfigureWaitStates( void )
{
unsigned long ulSystemClock = configCPU_CLOCK_HZ - 1;
unsigned long ulWaitStates, ulCHECONVal;

	/* 1 wait state pentru fiecare hwMAX_FLASH_SPEED MHz. */
	ulWaitStates = 0;

	while( ulSystemClock > hwMAX_FLASH_SPEED )
	{
		ulWaitStates++;
		ulSystemClock -= hwMAX_FLASH_SPEED;
	}

	/* obtine valoarea CHECON curenta. */
	ulCHECONVal = CHECON;

	/* Clear state bits, apoi seteaza wait state bits calculati. */
	ulCHECONVal &= ~hwCHECON_WAIT_STAT_BITS;
	ulCHECONVal |= ulWaitStates;

	CHECON = ulWaitStates;
}

static void __attribute__ ((nomips16)) prvKSeg0CacheOn( void )
{
unsigned long ulValue;

	__asm volatile( "mfc0 %0, $16, 0" :  "=r"( ulValue ) );
	ulValue = ( ulValue & ~0x07) | 0x03;
	__asm volatile( "mtc0 %0, $16, 0" :: "r" ( ulValue ) );
}


