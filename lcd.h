/* 
 * File:   lcd.h
 * Author: JZ95C2
 *
 * Created on March 18, 2014, 5:43 PM
 */

#ifndef LCD_H
#define	LCD_H

#ifdef	__cplusplus
extern "C" {
#endif



// RS = 1 ; access data register
#define LCDDATA 1
// RS = 0 ; access command register
#define LCDCMD  0
// PMP data buffer
#define PMDATA  PMDIN1          

#define LCD_CMD_CLEAR	(1)
#define LCD_CMD_CURSOR_HOME	(2)

#define LCDbusy()       (LCDread(LCDCMD) & 0x80)
#define LCDaddr()       (LCDread(LCDCMD) & 0x7F)
#define readLCDRAM()    (LCDread(LCDDATA))

//putLCD() will send ASCII data to the LCD module:
#define putLCD(d)	LCDwrite(LCDDATA, (d))
//LCDcmd() will send generic commands to the LCD module:
#define LCDcmd(c)  	LCDwrite(LCDCMD, (c))
//LCDhome() will reposition the cursor on the first character of the fi rst row:
#define LCDhome()       LCDwrite(LCDCMD, 2)
//LCDclr() will clear the entire contents of the display:
#define LCDclr()        LCDwrite(LCDCMD, 1)



//============================================================================
//Init LCD, Timer2 must be running now because we use delay function
//============================================================================
void LCDinit(void);

//============================================================================
//LCD Read function, used by other macros
//============================================================================
char LCDread(unsigned int addr);

//============================================================================
//LCD write function, used by other macros
//============================================================================
void LCDwrite(unsigned int addr, char c);

//============================================================================
//putsLCD(), a function that will send an entire
//null terminated string to the display module:
//============================================================================
//void putsLCD( char *s, unsigned int len);
void putsLCD(char *s);





#ifdef	__cplusplus
}
#endif

#endif	/* LCD_H */

