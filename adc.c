/*
 * File:   main.c
 * Author: jz95c2
 *
 * Created on February 2, 2014, 4:16 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "p24FJ128GA010.h"


void initADC( int amask)
{
    AD1PCFG = amask;       // select analog input pins
    AD1CON1 = 0x00E0;      // automatic conversion start after sampling
    AD1CSSL = 0;           // no scanning required
    AD1CON2 = 0;           // use MUXA, AVss and AVdd are used as Vref+/-
    AD1CON3 = 0x1F02;      // Tsamp = 32 x Tad; Tad=125ns
    AD1CON1bits.ADON = 1;   // turn on the ADC
} //initADC

int readADC( int ch)
{
    AD1CHS  = ch;             // 1. select analog input channel
    AD1CON1bits.SAMP = 1;     // 2. start sampling
    while (!AD1CON1bits.DONE);  // 3. wait for the conversion to complete
    return ADC1BUF0;        // 4. read the conversion result
} // readADC