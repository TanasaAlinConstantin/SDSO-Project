
#include <p32xxxx.h>
#include <sys/asm.h>
#include "ISR_Support.h"
 
#define portEXC_CODE_MASK ( 0x1f << 2 )

	.set	nomips16
 	.set 	noreorder
 	
 	.extern vT3InterruptHandler
 	.extern vT4InterruptHandler

 	
	.global vT3InterruptWrapper
	.global vT4InterruptWrapper


/******************************************************************/

	.set	noreorder
	.set 	noat
	.ent	vT3InterruptWrapper
	
vT3InterruptWrapper:

	portSAVE_CONTEXT

	jal		vT3InterruptHandler
	nop

	portRESTORE_CONTEXT

	.end vT3InterruptWrapper

/******************************************************************/

 	.set	noreorder
	.set 	noat
 	.ent	vT4InterruptWrapper
	
vT4InterruptWrapper:

	portSAVE_CONTEXT

	jal 	vT4InterruptHandler
	nop

	portRESTORE_CONTEXT

	.end vT4InterruptWrapper

