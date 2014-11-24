/* 
 * File:   eeprom.h
 * Author: jz95c2
 *
 * Created on April 1, 2014, 7:20 PM
 */

#ifndef EEPROM_H
#define	EEPROM_H

#ifdef	__cplusplus
extern "C" {
#endif




// I/O de? nitions for PIC24 + Explorer16 demo board
#define CSEE   _RD12
// select line for Serial EEPROM

#define TCSEE   _TRISD12
// tris control for CSEE pin

// peripheral con? gurations
#define SPI_MASTER  0x0122
// select 8-bit master mode, CKE=1, CKP=0

#define SPI_ENABLE 0x8000
// enable SPI port, clear status

// 25LC256 Serial EEPROM commands
#define SEE_WRSR  1
    // write status register
#define SEE_WRITE  2
    // write command
#define SEE_READ  3
    // read command
#define SEE_WDI  4
    // write disable
#define SEE_STAT  5
    // read status register
#define SEE_WEN  6
    // write enable


/*
** NVM storage library
**
** encapsulates 25LC256 Serial EEPROM
** as a NVM storage device for PIC24 + Explorer16 applications
*/
// initialize access to memory device
void InitNVM(void);
// 16-bit integer read and write functions
// NOTE: address must be an even value between 0x0000 and 0x7ffe
// (see page write restrictions on the device datasheet)
int  iReadNVM ( int address);
void iWriteNVM( int address, int data);

    

#ifdef	__cplusplus
}
#endif

#endif	/* EEPROM_H */

