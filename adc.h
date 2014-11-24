/* 
 * File:   adc.h
 * Author: jz95c2
 *
 * Created on March 25, 2014, 6:05 PM
 */

#ifndef ADC_H
#define	ADC_H

#ifdef	__cplusplus
extern "C" {
#endif






// TC1047 Temperature sensor with voltage output
#define TSENS   (4)

// 10k potentiometer connected to AN5 input
#define POT     (5)

// Analog inputs for Explorer16 POT and TSENS
#define AINPUTS (0xffcf)




void initADC( int amask);
int readADC( int ch);








#ifdef	__cplusplus
}
#endif

#endif	/* ADC_H */

