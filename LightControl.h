#ifndef LIGHTCONTROL_H
#define LIGHTCONTROL_H
#include <xc.h>
#include "TimeDelay.h"
#include "ADC.h"

void update_cycle(uint16_t buf_read);
/**
 * This function should take a value between 0 and 1023 and find 2 delays,
 * one representing the time the LED spends on, and the other representing 
 * the time it spends off. This should correspond to a brightness. 
 * It should return 0 for the off cycle when buf_read is 1023 and
 * 0 for the on cycle when buf_read is 0.
 */

#endif