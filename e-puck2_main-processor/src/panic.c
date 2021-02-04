#include <ch.h>
#include <hal.h>
char reasons[100];

void panic_handler(const char *reason)
{
    (void)reason;

	palClearPad(GPIOD, GPIOD_LED1);
	palClearPad(GPIOD, GPIOD_LED3);
	palClearPad(GPIOD, GPIOD_LED5);
	palClearPad(GPIOD, GPIOD_LED7);
	palClearPad(GPIOD, GPIOD_LED_FRONT);
	palClearPad(GPIOB, GPIOB_LED_BODY);
	
	for(char i=0; i<100;i++)
	{
	    if(reason[i] == '\0')
	        break;
	    else
	        reasons[i] = reason[i];
	}

    while (true) {

    }
}
