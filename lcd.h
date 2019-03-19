#ifndef LCD_INC_H
#define LCD_INC_H


QueueHandle_t xStartLCDTask( void );

typedef struct
{

	TickType_t xMinDisplayTime;

	char *pcMessage;

} xLCDMessage;


#endif


