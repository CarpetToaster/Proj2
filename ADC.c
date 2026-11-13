#include "xc.h"
#include "ADC.h"

uint16_t do_ADC(void){
    AD1CON1bits.ADON = 1;
    AD1CON1bits.SAMP = 1;
    AD1CON1bits.DONE = 0;
    
    while(AD1CON1bits.DONE == 0){}
    uint16_t adc_val = ADC1BUF0; // This is the output value
    AD1CON1bits.SAMP = 0;
    AD1CON1bits.ADON = 0;
    return adc_val;
}

void ADC_init(void){
    AD1CON2bits.VCFG = 0b000; // use internal Vref- and Vref+
    AD1CON3bits.ADRC = 0;      // using system clock
    AD1CON2bits.BUFM = 0;
    
    AD1PCFGbits.PCFG12 = 0;
    
    AD1CON2bits.ALTS = 0;
    AD1CON2bits.CSCNA = 0; // IDEFK WHAT SCANNING MEANS MAN
    
    AD1CON1bits.ASAM = 0;      // do when samp = 1
    
    AD1CHSbits.CH0NA = 0; // channel 0 neg is VR-
    AD1CHSbits.CH0SA = 12; // channel 0 pos is AN5
    
    // AD1CON3bits.ADCS = 0b111011; // Just setting it at 16 for now cuz I'm not actually sure what these values represent.
    AD1CON1bits.SSRC = 0b111; // auto convert after sample
    
    AD1CON3bits.SAMC = 0b11111; // Sample rate = 10T_AD

    AD1CON1bits.FORM = 0b00;  // unsigned integer
}
