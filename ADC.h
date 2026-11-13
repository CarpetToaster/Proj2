#ifndef ADC_H
#define ADC_H

// next cycle should alternate between the on and off cycle.
typedef struct __DUTY_CYCLE_{
    uint16_t on_cycle;
    uint16_t off_cycle;
    uint16_t* next_cycle; 
}duty_cycle_t;

uint16_t do_ADC(void);
// DO the ADC stuff that needs to be done. Yeah. IDK

void ADC_init(void);
// Inits the ADC.

#endif

