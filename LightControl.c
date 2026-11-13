#include "LightControl.h"
#include "UART2.h"

extern uint16_t adc_buf;
extern duty_cycle_t duty_cycle;
extern uint16_t blinky;

void update_cycle(uint16_t buf_read){
    adc_buf = buf_read;
    
    if(buf_read < 10){
        adc_buf = buf_read;
        duty_cycle.on_cycle = 1;
        duty_cycle.off_cycle = 1022;
        return;
    }
    else if(buf_read >= 1013){
        adc_buf = buf_read;
        duty_cycle.on_cycle = 1022;
        duty_cycle.off_cycle = 1;
        if(blinky) T2CONbits.TON = 1; // Also turn on here if blinking.
        return;
    }

    T2CONbits.TON = 1; // incase timer was off from a solid
    /* Make a new delay cycle */

    duty_cycle.on_cycle = buf_read;
    duty_cycle.off_cycle = 1023 - duty_cycle.on_cycle;
}