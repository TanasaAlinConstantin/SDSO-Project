#include <plib.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "lcd.h"

static void vLCDTask( void *pvParameters );

static void prvSetupLCD( void );

static void prvLCDGotoRow( unsigned short usRow );

static void prvLCDPutString( char *pcString );


static void prvLCDClear( void );


/* intarziere pentru a permite LCD sa fie sincron cu comenzile */
#define lcdVERY_SHORT_DELAY	( 1 )
#define lcdSHORT_DELAY		( 8 / portTICK_PERIOD_MS )
#define lcdLONG_DELAY		( 15 / portTICK_PERIOD_MS )

#define LCD_CLEAR_DISPLAY_CMD			0x01
#define LCD_CURSOR_HOME_CMD				0x02
#define LCD_ENTRY_MODE_CMD				0x04
#define LCD_ENTRY_MODE_INCREASE			0x02
#define LCD_DISPLAY_CTRL_CMD			0x08
#define LCD_DISPLAY_CTRL_DISPLAY_ON		0x04
#define LCD_FUNCTION_SET_CMD			0x20
#define LCD_FUNCTION_SET_8_BITS			0x10
#define LCD_FUNCTION_SET_2_LINES		0x08
#define LCD_FUNCTION_SET_LRG_FONT		0x04
#define LCD_NEW_LINE					0xC0
#define LCD_COMMAND_ADDRESS				0x00
#define LCD_DATA_ADDRESS				0x01

/* lungime queue folosita pentru a trimite mesaje catre LCD gatekeeper task. */
#define lcdQUEUE_SIZE		3


QueueHandle_t xLCDQueue;

static void prvLCDCommand( char cCommand );
static void prvLCDData( char cChar );


QueueHandle_t xStartLCDTask( void )
{
	xLCDQueue = xQueueCreate( lcdQUEUE_SIZE, sizeof( xLCDMessage ));

	xTaskCreate( vLCDTask, "LCD", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL );

	return xLCDQueue;
}

static void prvLCDGotoRow( unsigned short usRow )
{
	if(usRow == 0) 
	{
		prvLCDCommand( LCD_CURSOR_HOME_CMD );
	} 
	else 
	{
		prvLCDCommand( LCD_NEW_LINE );
	}
}

static void prvLCDCommand( char cCommand ) 
{
	PMPSetAddress( LCD_COMMAND_ADDRESS );
	PMPMasterWrite( cCommand );
   	vTaskDelay( lcdSHORT_DELAY );
}

static void prvLCDData( char cChar )
{
	PMPSetAddress( LCD_DATA_ADDRESS );
	PMPMasterWrite( cChar );
	vTaskDelay( lcdVERY_SHORT_DELAY );
}

static void prvLCDPutString( char *pcString )
{
	/* scrie caracter cu delay intre el si anteriorul */
	while(*pcString)
	{
		prvLCDData(*pcString);
		pcString++;
		vTaskDelay(lcdSHORT_DELAY);
	}
}

static void prvLCDClear(void)
{
	prvLCDCommand(LCD_CLEAR_DISPLAY_CMD);
}

static void prvSetupLCD(void)
{
	/* asteapta power up. */
	vTaskDelay( lcdLONG_DELAY );
	
	/* deschide PMP port */
	mPMPOpen((PMP_ON | PMP_READ_WRITE_EN | PMP_CS2_CS1_EN |
			  PMP_LATCH_POL_HI | PMP_CS2_POL_HI | PMP_CS1_POL_HI |
			  PMP_WRITE_POL_HI | PMP_READ_POL_HI),
			 (PMP_MODE_MASTER1 | PMP_WAIT_BEG_4 | PMP_WAIT_MID_15 |
			  PMP_WAIT_END_4),
			  PMP_PEN_0, 0);
			 
	/* asteapta ca LCD sa porneasca corespunzator */
	vTaskDelay( lcdLONG_DELAY );
	vTaskDelay( lcdLONG_DELAY );
	vTaskDelay( lcdLONG_DELAY );

	prvLCDCommand( LCD_FUNCTION_SET_CMD | LCD_FUNCTION_SET_8_BITS | LCD_FUNCTION_SET_2_LINES | LCD_FUNCTION_SET_LRG_FONT );
	
	/* porneste display. */
	prvLCDCommand( LCD_DISPLAY_CTRL_CMD | LCD_DISPLAY_CTRL_DISPLAY_ON );
	
	/* curata display. */
	prvLCDCommand( LCD_CLEAR_DISPLAY_CMD );
	vTaskDelay( lcdLONG_DELAY );	
	
	/* incrementeaza cursor. */
	prvLCDCommand( LCD_ENTRY_MODE_CMD | LCD_ENTRY_MODE_INCREASE );
	vTaskDelay( lcdLONG_DELAY );		  	
	vTaskDelay( lcdLONG_DELAY );		  	
	vTaskDelay( lcdLONG_DELAY );
}

static void vLCDTask(void *pvParameters)
{
xLCDMessage xMessage;
unsigned short usRow = 0;

	/* initializare hw.  fol delay ca sa nu fie apelat inainte de scheduler */
	prvSetupLCD();

	/* scrie mesaj */
	prvLCDPutString( "SDSO TEST" );

	for(;;)
	{
		while( xQueueReceive( xLCDQueue, &xMessage, portMAX_DELAY ) != pdPASS );

		prvLCDClear();

		prvLCDGotoRow( usRow & 0x01 );
		usRow++;
		prvLCDPutString( xMessage.pcMessage );

		vTaskDelay( xMessage.xMinDisplayTime );		
	}
}




