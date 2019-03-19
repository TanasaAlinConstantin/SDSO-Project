#ifndef CONFIG_PERFORMANCE_H
#define CONFIG_PERFORMANCE_H

void vHardwareConfigurePerformance( void );

/* Configureaza controllerul de itreruperi sa foloseasca un vector separat pentru fiecare intrerupere */
void vHardwareUseMultiVectoredInterrupts( void );

#endif
