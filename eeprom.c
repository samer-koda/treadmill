/*
 * File:   eeprom.c
 * Author: jz95c2
 *
 * Created on February 2, 2014, 4:16 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "p24FJ128GA010.h"

#include "eeprom.h"


void InitNVM(void)
{
    // init the SPI peripheral
    TCSEE = 0;        // make SSEE pin output
    CSEE = 1;        // de-select the Serial EEPROM
    SPI2CON1 = SPI_MASTER;      // select mode
    SPI2STAT = SPI_ENABLE;      // enable the peripheral
}//InitNVM


int WriteSPI2( int data)
{// send one byte of data and receive one back at the same time
    SPI2BUF = data;      // write to buffer for TX
    while( !SPI2STATbits.SPIRBF);  // wait for transfer to complete
    return SPI2BUF;          // read the received value
}//WriteSPI2

int ReadSR( void)
{// Check the Serial EEPROM status register
   int i;
    CSEE = 0;        // select the Serial EEPROM
    WriteSPI2( SEE_STAT);      // send a READ STATUS COMMAND
    i = WriteSPI2( 0);      // send/receive
    CSEE = 1;        // deselect to terminate command
    return i;
} //ReadSR


int iReadNVM( int address)
{ // read a 16-bit value starting at an even address
  int lsb, msb;
  // wait until any work in progress is completed
  while ( ReadSR() & 0x3);  // check the two lsb WEN and WIP

  // perform a 16-bit read sequence (two byte sequential read)
  CSEE = 0;      // select the Serial EEPROM
  WriteSPI2( SEE_READ);   // read command
  WriteSPI2( address>>8);  // address MSB first
  WriteSPI2( address & 0xfe);  // address LSB (word aligned)
  msb = WriteSPI2( 0);   // send dummy, read msb
  lsb = WriteSPI2( 0);   // send dummy, read lsb
  CSEE = 1;
  return ( (msb<<8)+ lsb);
}//iReadNVM


void WriteEnable( void)
{ // send a Write Enable command
  CSEE = 0;        // select the Serial EEPROM
  WriteSPI2( SEE_WEN);     // write enable command
  CSEE = 1;        // deselect to complete the command
}//WriteEnable


void iWriteNVM( int address, int data)
{ // write a 16-bit value starting at an even address
  //int lsb, msb;
  // wait until any work in progress is completed
  while ( ReadSR() & 0x3);    // check the two lsb WEN and WIP

  // Set the Write Enable Latch
  WriteEnable();

  // perform a 16-bit write sequence (2 byte page write)
  CSEE = 0;        // select the Serial EEPROM
  WriteSPI2( SEE_WRITE);    // write command
  WriteSPI2( address>>8);    // address MSB ? rst
  WriteSPI2( address & 0xfe);    // address LSB (word aligned)
  WriteSPI2( data >>8);     // send msb
  WriteSPI2( data & 0xff);    // send lsb
  CSEE = 1;
}//iWriteNVM

