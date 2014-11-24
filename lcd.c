
#include <p24fj128ga010.h>
#include "lcd.h"



//============================================================================
//extern declaritions (functions provided by other modules)
//============================================================================
extern void mydelay_us(unsigned long period);




//============================================================================
//Init LCD, Timer2 must be running now because we use delay function
//============================================================================
void LCDinit( void)
{
    // PMP initialization
    PMCON = 0x83BF;         // Enable the PMP, long waits
    PMMODE = 0x3FF;         // Master Mode 1
    PMAEN = 0x0001;         // PMA0 and PMA1 enabled, but we use only PMA0 for Register Select (RS)

    mydelay_us(32000);		// Delay for 32msec

    PMADDR = LCDCMD;        // select the command register (ADDR = 0)
    PMDATA = 0b00111000;    // function set: 8-bit interface, 2 lines, 5x7
    mydelay_us(48);			// Delay for 48us

    PMDATA = 0b00001100;    // display ON, cursor off, blink off
    mydelay_us(1600);			// Delay for 48us

    PMDATA = 0b00000001;    // clear display
    mydelay_us(1600);		// Delay for 1600us

    PMDATA = 0b00000110;    // increment cursor, no shift
    mydelay_us(1600);		// Delay for 1600us

    //PMDATA = 0b00000010;    // cusor home
    //mydelay_us(1600);		// Delay for 1600us
}


//============================================================================
//LCD Read function, used by other macros
//============================================================================
char LCDread( unsigned int addr)
{
    unsigned int dummy;
    while( PMMODEbits.BUSY);    // wait for PMP to complete previous commands
    PMADDR = addr;              // select the command address
    dummy = PMDATA;             // initiate a read cycle, dummy read
    while( PMMODEbits.BUSY);    // wait for PMP to complete the sequence
    return ((char)PMDATA);      // read the true data
}


//============================================================================
//LCD write function, used by other macros
//============================================================================
void LCDwrite( unsigned int addr, char c)
{
    while( LCDbusy());
    while( PMMODEbits.BUSY);    // wait for PMP to be available
    PMADDR = addr;
    PMDATA = c;
}


//============================================================================
//putsLCD(), a function that will send an entire
//null terminated string to the display module:
//============================================================================
/*
void putsLCD( char *s, unsigned int len)
{
    unsigned int i;
    for (i=0; i<len; i++)
	{
        putLCD(s[i]);
	}
}*/

void putsLCD( char *s)
{
    while( *s)
    putLCD( *s++);
}
