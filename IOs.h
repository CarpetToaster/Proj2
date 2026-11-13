#ifndef IOS_H
#define IOS_H

#include <xc.h>
#include "TimeDelay.h"


// When constructing, attach addr of desired LED's register and the active one should only ever be one of these.
typedef struct __STRUCT_LEDS_{
    volatile uint16_t* LED1;
    volatile uint16_t* LED2;
    volatile uint16_t* active;
}led_t;
/**
 * Due to a fairly lucky coincedence, LATB6 isn't implemented and LATA9 doesn't exist,
 * This means we can do the same operation on both of the LED pointers and it won't actually
 * break anything, since th values at the mentioned ones don't matter... which is really cool.
 */


void IOinit();
// Initializes all input and output ports, relating to push buttns
// and LED lights.

#endif
