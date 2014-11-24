/* 
 * Samer Koda
 * EmbII, CET 384
 * Homework #1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "p24FJ128GA010.h"
#include "lcd.h"
#include "adc.h"
#include "eeprom.h"


//disable JTAG and Watchdog

_CONFIG1(JTAGEN_OFF & GCP_OFF & GWRP_OFF & COE_OFF & FWDTEN_OFF & ICS_PGx2)
//set CPU clock to FRCPLL (32MHz)
_CONFIG2(POSCMOD_NONE & FNOSC_FRCPLL & OSCIOFNC_ON & FCKSM_CSECME)



#ifdef _ISR
#undef _ISR
#define _ISR __attribute__((interrupt, auto_psv))
#endif


#define LED_PERIOD 15625

typedef enum TreadmillState_t {
    ST_PowerUp = 0,
    ST_Ready = 1,
    ST_Running = 2,
    ST_History = 3,
} TreadmillState_t;

typedef struct History_EEPROM_t {
    unsigned short Dist;
    unsigned short Cal;
    unsigned short Checksum;

} History_EEPROM_t;


//============================================================================
//static gloable variables
//============================================================================
static unsigned char tmr1_int_flag = 0;
static unsigned char cn15_int_flag = 0;
static unsigned char cn16_int_flag = 0;
static unsigned char cn19_int_flag = 0;
//static unsigned char led_mode = 0;
//static unsigned char led_state = 1;

static TreadmillState_t TrdmState = ST_PowerUp;
static char LCDFirstLn[17];
static char LCDSecondLn[17];
static float Distance = 0;
static float Speed = 0.0;
static unsigned short Calories = 0;
static unsigned char HeartBeat = 60;

static unsigned char LEDsquence = 0x80;

//Temp Display variables
static unsigned short a2d;
static float volt;
static float temperature;

static History_EEPROM_t History;

//============================================================================
//Timer1 interrupt ISR
//============================================================================

void _ISR _T1Interrupt(void) {
    tmr1_int_flag = 1;
    //clear the interrupt flag
    _T1IF = 0;
}

//============================================================================
//CN interrupt ISR
//============================================================================

void _ISR _CNInterrupt(void) {
    //clear the interrupt flag
    if ((PORTD & 0x0040) == 0) {
        cn15_int_flag = 1;
    } else if ((PORTD & 0x0080) == 0) {
        cn16_int_flag = 1;
    } else if ((PORTD & 0x2000) == 0) {
        cn19_int_flag = 1;
    }

    _CNIF = 0;
}



//============================================================================
//Dummy delay function using for loop
//============================================================================

void mydelay(void) {
    unsigned long i;
    for (i = 0; i < 120000; i++) {
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
    }
}

//============================================================================
//Delay function using TIMER2, 16MHz, prescaler=256, one tick = 16us
//============================================================================

void mydelay_us(unsigned long period) {
    TMR2 = 0;
    while (TMR2 < (period >> 4));
}

//============================================================================
//Delay function using TIMER2, 16MHz, prescaler=256, one tick = 16us
//============================================================================

void mydelay_ms(unsigned long period) {
    TMR2 = 0;
    while (TMR2 < (period << 6));
}

//============================================================================
//TIMR1 init function
//============================================================================

void TMR1_init(void) {
    //Init Timer1, used to generate heart beat.
    TMR1 = 0; // clear the timer
    PR1 = 62500; // 1sec interrupt
    _T1IP = 4; // this is the default value anyway
    _T1IF = 0; // Clear int flag
    _T1IE = 1; // Enable int
    T1CON = 0x8030; // TMR1 runs, prescaler 1:256 Tcy=Tclk/2=16MHz
}

//============================================================================
//TIMR2 init function
//============================================================================

void TMR2_init(void) {
    //Init Timer2, used as free run timer
    T2CON = 0x8030; // TMR2 runs, prescaler 1:256 Tcy=Tclk/2=16MHz, one tick=16us
}

//============================================================================
//CN init function
//============================================================================

void CN_init(void) {
    _CNIP = 4; // this is the default value anyway
    _CNIF = 0; // Clear int flag
    _CNIE = 1; // Enable int
    _CN15IE = 1; //Enable RD6 (S3) interrupt
    _CN16IE = 1; //Enable RD7 (S6) interrupt
    _CN19IE = 1; //Enable RD13 (S4) interrupt
}

void ProcessTmr1Int(void) {
    switch (TrdmState) {
        case ST_PowerUp:
        case ST_Ready:
            PORTA ^= 0xff;
            break;
        case ST_Running:
            PORTA = (LEDsquence);
            LEDsquence = (LEDsquence >> 1);
            if (LEDsquence == 0) {
                LEDsquence = 0x80;
            }
            Distance += (Speed / 3600) * ((float) PR1 / 6250.0); // calc for distance
            Calories = Distance * 100.0; // calc for cakories
            HeartBeat = 60 + (((unsigned) readADC(POT) * 120) >> 10); // calc for heartbeat
            break;
        case ST_History:
            break;
        default:
            break;
    }
}

void ProcessS3Int(void) {
    switch (TrdmState) {
        case ST_PowerUp:
            TrdmState = ST_Ready;
            LCDclr();
            break;
        case ST_Ready:
            TrdmState = ST_Running;
            Speed = 5;
            break;
        case ST_Running:
            if (PR1 > 8000) {
                Speed = Speed + 5;
                PR1 = PR1 - 3000;
            }
            break;
        case ST_History:
            break;
        default:
            break;
    }
}

void ProcessS4Int(void) {
    switch (TrdmState) {
        case ST_PowerUp:
            break;
        case ST_Ready:
            TrdmState = ST_PowerUp;
            PR1 = 62500;
            PORTA = 0x00;
            /*******************************************************/
            History.Dist = (unsigned short) (Distance * 10.0);
            iWriteNVM(0, History.Dist);
            /*******************************************************/
            History.Cal = (unsigned short) (Calories);
            iWriteNVM(2, History.Cal);
            /*******************************************************/
            History.Checksum = History.Dist + History.Cal + (unsigned short) 0x5555;
            iWriteNVM(4, History.Checksum);
            /*******************************************************/
            break;
        case ST_Running:
            TrdmState = ST_PowerUp;
            PR1 = 62500;
            PORTA = 0x00;
            /*******************************************************/
            History.Dist = (unsigned short) (Distance * 10.0);
            iWriteNVM(0, History.Dist);
            /*******************************************************/
            History.Cal = (unsigned short) (Calories);
            iWriteNVM(2, History.Cal);
            /*******************************************************/
            History.Checksum = History.Dist + History.Cal + 0xAAAA;
            iWriteNVM(4, History.Checksum);
            /*******************************************************/
            Distance = 0.0;
            Speed = 0.0;
            Calories = 0;
            break;
        case ST_History:
            break;
        default:
            break;
    }
}

void ProcessS6Int(void) {
    switch (TrdmState) {
        case ST_PowerUp:
            TrdmState = ST_History;
            break;
        case ST_Ready:
            // do nothing
            break;
        case ST_Running:
            if (Speed > 0) {
                Speed = Speed - 5;
                PR1 = PR1 + 3000;
                if (Speed < 5) {
                    TrdmState = ST_Ready;
                    PORTA = 0xff; // make sure all LES's on
                    PR1 = 62500; // make sure TMR1 interurupt is back to 1sec
                }
            }
            break;
        case ST_History:
            TrdmState = ST_Ready;
            break;
        default:
            break;
    }
}

void LCDUpdate(void) {
    switch (TrdmState) {
        case ST_PowerUp:
            memset(LCDFirstLn, 0, 17);
            memset(LCDSecondLn, 0, 17);
            LCDcmd(0x80);
            sprintf(LCDFirstLn, "Treadmill %2.1fC", (double) temperature);
            putsLCD(LCDFirstLn);
            LCDcmd(0xc0);
            sprintf(LCDSecondLn, "S3=Strt  S6=Hist");
            putsLCD(LCDSecondLn);
            break;
        case ST_Ready:
        case ST_Running:
            memset(LCDFirstLn, 0, 17);
            memset(LCDSecondLn, 0, 17);
            LCDcmd(0x80);
            sprintf(LCDFirstLn, "V=%2.1f C=%d ", (double) Speed, Calories);
            putsLCD(LCDFirstLn);
            LCDcmd(0xc0);
            sprintf(LCDSecondLn, "D=%2.1f Hrt=%d ", (double) Distance, HeartBeat);
            putsLCD(LCDSecondLn);
            break;
        case ST_History:
            if (History.Checksum == (History.Dist + History.Cal + 0x5555)) {
                LCDcmd(0x80);
                sprintf(LCDFirstLn, "Distance = %d   ", History.Dist);
                putsLCD(LCDFirstLn);
                LCDcmd(0xc0);
                sprintf(LCDSecondLn, "Calories = %d  ", History.Cal);
                putsLCD(LCDSecondLn);
            }
            if (History.Checksum != (History.Dist + History.Cal + 0x5555)) {
                LCDcmd(0x80);
                sprintf(LCDFirstLn, "No History");
                putsLCD(LCDFirstLn);
            }
            break;
        default:
            break;
    }
}

//============================================================================
// main function
//============================================================================

int main(int argc, char** argv) {



    //----------------------------------------------------
    //Make sure clock runs at 32MHz
    CLKDIV = 0;

    //----------------------------------------------------
    //do not allow nested interrupts
    _NSTDIS = 1;

    //----------------------------------------------------
    //Init LED controls
    TRISA = 0xff00;
    PORTA = 0x00;

    //----------------------------------------------------
    //Init PORTD for detect button press
    TRISD = (TRISD | 0x20c0);

    //----------------------------------------------------
    //Init TIMR1/2 for heart beat signal and free run timer
    TMR1_init();
    TMR2_init();

    //----------------------------------------------------
    //Init CN
    CN_init();

    //-----------------------------------------------------
    //Init LCD
    LCDinit();

    //----------------------------------------------------
    //Init CN
    initADC(AINPUTS);

    a2d = (unsigned short) readADC(TSENS);
    volt = (float) a2d * 3300.0 / 1023.0;
    temperature = (volt - 500.0) / 10.0;

    //----------------------------------------------------
    //InitNVM
    InitNVM();
    History.Dist = iReadNVM(0);
    History.Cal = iReadNVM(2);
    History.Checksum = iReadNVM(4);


    while (1) {
        if (tmr1_int_flag > 0) {
            tmr1_int_flag = 0;
            ProcessTmr1Int();
        }


        if (cn15_int_flag > 0) { //s3
            mydelay_ms(200);
            cn15_int_flag = 0;
            ProcessS3Int();
        }

        if (cn19_int_flag > 0) { //s4
            mydelay_ms(200);
            cn19_int_flag = 0;
            ProcessS4Int();
        }

        if (cn16_int_flag > 0) { //s6
            mydelay_ms(200);
            cn16_int_flag = 0;
            ProcessS6Int();
        }

        LCDUpdate();
    }


    return (EXIT_SUCCESS);
}

