#include "IOs.h"
#include "LightControl.h"
#include "UART2.h"

void IOinit(){
    TRISBbits.TRISB12 = 1;
    AD1PCFG = 0xffff; // analogs are now required for this!
                      // bits 15 and 14 set to 0 for internal band gap
                      // (Part 1) bit 5 set to 0 to do analog
                  
    // LED1, more on these in IOs.h
    TRISBbits.TRISB9 = 0;
    LATBbits.LATB9 = 0;
    
    // LED2, also more on this in IOs.h
    TRISAbits.TRISA6 = 0;
    LATAbits.LATA6 = 0;
    
    // Disable ADC interrupts
    IPC3bits.AD1IP = 0;
    IEC0bits.AD1IE = 1;
    
    TRISAbits.TRISA4 = 1;
    CNPU1bits.CN0PUE = 1;
    CNEN1bits.CN0IE = 1;
    
    TRISBbits.TRISB4 = 1;
    CNPU1bits.CN1PUE = 1;
    CNEN1bits.CN1IE = 1;
    
    TRISBbits.TRISB7 = 1;
    CNPU2bits.CN23PUE = 1;
    CNEN2bits.CN23IE = 1;
    
    IPC4bits.CNIP = 6;
    IFS1bits.CNIF = 0;
    IEC1bits.CNIE = 1;
    
    //CNPD1bits.CN14PDE = 1; // Pull down for the potentiometer.
}