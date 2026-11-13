#ifndef TIMEDELAY_H
#define TIMEDELAY_H
#include <xc.h>

void T1_init();

void T2_init();

void T3_init();

void T1_button_hold(uint16_t time_ms);
// button held down timer.

void T2_PWM(uint16_t time_ms);
// responsible for turning T2 on and off and setting up some PWM stuff.
// A lot of the logic will be in the ISR

void T3_delay_ms(uint16_t time_ms);
// Timer used for generating simple delays only escapable with button 
// presses.

#endif

