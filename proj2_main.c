/*
 * File:   proj2main.c
 * Author: Ayre Ibrahim, Kai Christie, Mustafa Ayobi
 *
 * Created on: USE THE INFORMATION FROM THE HEADER MPLAB X IDE GENERATES FOR YOU
 */

// FBS
#pragma config BWRP = OFF               // Table Write Protect Boot (Boot segment may be written)
#pragma config BSS = OFF                // Boot segment Protect (No boot program Flash segment)

// FGS
#pragma config GWRP = OFF               // General Segment Code Flash Write Protection bit (General segment may be written)
#pragma config GCP = OFF                // General Segment Code Flash Code Protection bit (No protection)

// FOSCSEL
#pragma config FNOSC = FRC              // Oscillator Select (Fast RC oscillator (FRC))
#pragma config IESO = OFF               // Internal External Switch Over bit (Internal External Switchover mode disabled (Two-Speed Start-up disabled))

// FOSC
#pragma config POSCMOD = NONE           // Primary Oscillator Configuration bits (Primary oscillator disabled)
#pragma config OSCIOFNC = ON            // CLKO Enable Configuration bit (CLKO output disabled; pin functions as port I/O)
#pragma config POSCFREQ = HS            // Primary Oscillator Frequency Range Configuration bits (Primary oscillator/external clock input frequency greater than 8 MHz)
#pragma config SOSCSEL = SOSCHP         // SOSC Power Selection Configuration bits (Secondary oscillator configured for high-power operation)
#pragma config FCKSM = CSECMD           // Clock Switching and Monitor Selection (Clock switching is enabled, Fail-Safe Clock Monitor is disabled)

// FWDT
#pragma config WDTPS = PS32768          // Watchdog Timer Postscale Select bits (1:32,768)
#pragma config FWPSA = PR128            // WDT Prescaler (WDT prescaler ratio of 1:128)
#pragma config WINDIS = OFF             // Windowed Watchdog Timer Disable bit (Standard WDT selected; windowed WDT disabled)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))

// FPOR
#pragma config BOREN = BOR3             // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware; SBOREN bit disabled)
#pragma config PWRTEN = ON              // Power-up Timer Enable bit (PWRT enabled)
#pragma config I2C1SEL = PRI            // Alternate I2C1 Pin Mapping bit (Default location for SCL1/SDA1 pins)
#pragma config BORV = V18               // Brown-out Reset Voltage bits (Brown-out Reset set to lowest voltage (1.8V))
#pragma config MCLRE = ON               // MCLR Pin Enable bit (MCLR pin enabled; RA5 input pin disabled)

// FICD
#pragma config ICS = PGx2               // ICD Pin Placement Select bits (PGC2/PGD2 are used for programming and debugging the device)

// FDS
#pragma config DSWDTPS = DSWDTPSF       // Deep Sleep Watchdog Timer Postscale Select bits (1:2,147,483,648 (25.7 Days))
#pragma config DSWDTOSC = LPRC          // DSWDT Reference Clock Select bit (DSWDT uses LPRC as reference clock)
#pragma config RTCOSC = SOSC            // RTCC Reference Clock Select bit (RTCC uses SOSC as reference clock)
#pragma config DSBOREN = ON             // Deep Sleep Zero-Power BOR Enable bit (Deep Sleep BOR enabled in Deep Sleep)
#pragma config DSWDTEN = ON             // Deep Sleep Watchdog Timer Enable bit (DSWDT enabled)

// #pragma config statements should precede project file includes.

#include <xc.h>
#include <p24F16KA101.h>
#include "clkChange.h"
#include "UART2.h"
#include "TimeDelay.h"
#include "IOs.h"
#include "LightControl.h"
#include "ADC.h"

/**
* A small series of defines and putting all of the globals into main should
* Make the code a wee bit easier to understand and follow.
* This is the final project after all, might as well make sure it's our best.
*/
#define MODE_OFF 0
#define MODE_ON 1

#define SWITCH_LED (LEDs.active = (volatile uint16_t*)((LEDs.active == (volatile uint16_t*)0x2c4) ? 0x2cc : 0x2c4))
#define LED_ON (*(LEDs.active) |= 0x240)  // Or equals  0b0010_0100_0000 -> 0x240
#define LED_OFF (*(LEDs.active) &= 0xdbf) // and equals 0b1101_1011_1111 -> 0xdbf

#define ON_CYCLE (duty_cycle.on_cycle)
#define OFF_CYCLE (duty_cycle.off_cycle)
#define NEXT_CYCLE (duty_cycle.next_cycle)
#define SWITCH_NEXT_CYCLE (NEXT_CYCLE = (NEXT_CYCLE == &(ON_CYCLE)) ? &(OFF_CYCLE) : &(ON_CYCLE))

// next 9 relate to button press states, might finally be using all of these!
uint16_t PB1_prev = 0;
uint16_t PB2_prev = 0;
uint16_t PB3_prev = 0;

uint16_t PB1 = 0; 
uint16_t PB2 = 0;
uint16_t PB3 = 0;

uint16_t PB1_event = 0; // new flag to tell if a button was pressed.
uint16_t PB2_event = 0; // Same for PB2
uint16_t PB3_event = 0;

// Next four relate to the project 2 major states
uint16_t mode = 0;
uint16_t mode_prev = 1;
uint16_t transmitting = 0;
uint16_t blinky = 0;

// Next two relate to interrupt and Idle control
uint16_t waiting_trigger = 1;
uint16_t skip_next_trigger = 0;

// These two relate to the adc buffer value
uint16_t adc_buf = 0;
uint16_t new_adc_reading = 0;

// These next three relate to the control of timer related and PWM functions
uint16_t T1_done = 0;
uint32_t timer2_past = 0; // making this wider as when it's 65535 some issues can happen that need more checks.
duty_cycle_t duty_cycle; 

// Lastly this one relates to the lights, obviously.
led_t LEDs; 

int main(void) {
    newClk(8);
    T1_init();
    T2_init();
    T3_init();
    IOinit(); 
    InitUART2();
    ADC_init();
    
    // These are the addresses of LATB9 and LATA6.
    LEDs.LED1 = (volatile uint16_t*)0x2c4; // 0xc = 0x4 = 0b0010 = 4
    LEDs.LED2 = (volatile uint16_t*)0x2cc; // 0xc = 0b1100 = 12 ... I'm gonna do wisardry
    LEDs.active = LEDs.LED1;

    // Some initial duty cycle testing, 
    ON_CYCLE = 27;
    OFF_CYCLE = 996;
    NEXT_CYCLE = &(ON_CYCLE);
    
    // These two vars on the stack help when switching between on and off mode.
    uint16_t on_prev = ON_CYCLE;
    uint16_t off_prev = OFF_CYCLE;
    
    SWITCH_LED;
    T2_PWM(*NEXT_CYCLE); // For demonstration for now, should only happen when initially put into on mode.
    // Also maybe just replace it with T2CONbits.TON = 1 or smth that may be nicer.
    
    while(1) {
        /**
         * A bunch of button presses. While not mutually exclusive,
         * The most recent one will override any ongoing ones.
         ************************************************************************/
        if(PB1_event){
            PB1_event = 0;
            T1_button_hold(15);
            
            if(T1_done && PB1 && !PB1_prev){
                T1_done = 0;
                T1_button_hold(650);
                if(T1_done && PB1 && !PB1_prev){
                    T1_done = 0;
                    if(mode == MODE_ON){
                        LED_OFF;
                        SWITCH_LED;
                        if(!blinky && OFF_CYCLE == 1) LED_ON;
                    }
                }
                else{
                    T1_button_hold(0);
                    mode_prev = mode;
                    mode = !mode;
                }
            }
            else{
                T1_button_hold(0);
            }
        }

        if(PB2_event){
            PB2_event = 0;
            T1_button_hold(30); // basically a debounce
            if(T1_done && PB2 && !PB2_prev){
                T1_done = 0;
                blinky = !blinky;
            }
            else{
                T1_button_hold(0);
            }
        }
        
        if(PB3_event){
           PB3_event = 0;   
            T1_button_hold(30);
            if(T1_done && PB3 && !PB3_prev){
                T1_done = 0;
                transmitting = !transmitting;
            }
            else{
                T1_button_hold(0);
            }        
        }
        T1_button_hold(0);
        /************************************************************************/
        
        /**
         * The two main states are contained here, on and off. Each 
         * have their own set of substates, namely blinking and transmitting.
         ************************************************************************/
        switch(mode){
            case MODE_OFF:
                LED_OFF;
                // Save these values
                if(mode_prev == MODE_ON){
                    transmitting = 0;
                    T2CONbits.TON = 0;
                    off_prev = OFF_CYCLE;
                    on_prev = ON_CYCLE; 
                    // New ones for off mode, s it should be "solid" light.
                    ON_CYCLE = 1022;
                    OFF_CYCLE = 1;
                    mode_prev = mode;
                }
                
                while(!PB1_event && !PB2_event){
                    T2CONbits.TON = blinky;
                    Idle();
                }
                break;

            case MODE_ON: 
                // Retrieve these incase they were changed
                if(mode_prev == MODE_OFF){
                    ON_CYCLE = on_prev;
                    OFF_CYCLE = off_prev;
                    T2CONbits.TON = 1;
                    mode_prev = mode;
                }
                
                new_adc_reading = do_ADC();
                update_cycle(new_adc_reading);
                
                // Save here again to prevent lag
                off_prev = OFF_CYCLE;
                on_prev = ON_CYCLE;

                if (transmitting){
                    transmit(ON_CYCLE, OFF_CYCLE, new_adc_reading);
                }
                T3_delay_ms(100); 
                break;
        }
        /************************************************************************/
    }
    return 0;
}


/**
 * While this ISR may look quite long, due to it's branching neighbor, it should only take 
 * around 20 cycles to complete in it's worst case scenario thanks to short circuiting!
 */
void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void){
    //Don't forget to clear the timer 2 interrupt flag!
    IFS0bits.T2IF = 0;
    T2CONbits.TON = 0;
    
    // For the blinking, if needed. Maybe move into the blinky check?
    if(PR2 != 31250){
        timer2_past += TMR2;
    }
    TMR2 = 0;
    
    if(ON_CYCLE == 1){
        LED_OFF;
        return;
    }
    
    // Special cases when blinking, 100% needs to be treated differently.
    if(blinky && PR2 != 31250 && timer2_past >= 31250){
        timer2_past = 0;
        PR2 = 31250;
        LED_OFF;
        T2CONbits.TON = 1;
        return;
    }
    
    if(OFF_CYCLE == 1 && !blinky){
        LED_ON;
        return;
    }
    
    if(NEXT_CYCLE == &ON_CYCLE || OFF_CYCLE == 1){
        LED_ON;
    }
    else{
        LED_OFF;
    }
    
    PR2 = *NEXT_CYCLE;
    SWITCH_NEXT_CYCLE;
    
    T2CONbits.TON = 1;
}

void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void){
    //Don't forget to clear the timer 2 interrupt flag!
    IFS0bits.T3IF = 0;
    waiting_trigger = 0;
    // skip_next_trigger = 0;
    TMR3 = 0;
    T3CONbits.TON = 0;
}

void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void){
    IFS0bits.T1IF = 0;
    // waiting_trigger = 0;
    skip_next_trigger = 0;
    T1_done = 1;
    T1CONbits.TON = 0;
    TMR1 = 0;
   
}

void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void){
    //Don't forget to clear the CN interrupt flag!
    IFS1bits.CNIF = 0;
    
    // This next bit helps ensure only a CN interrupt wakes the CPU from idle
    skip_next_trigger = 0;
    waiting_trigger = 0;
    
    // reset the delay timer every time a CN interrupt occurs
    T3CONbits.TON = 0;
    TMR3 = 0;
    
    T1_done = 0;
    
    PB1_prev = PB1;
    PB2_prev = PB2;
    PB3_prev = PB3;
    
    // This looks weird but it allows for a super fast state set
    PB1 = !PORTBbits.RB7;
    PB2 = !PORTBbits.RB4;
    PB3 = !PORTAbits.RA4;
    
    if(PB1 && !PB1_prev){
        PB1_event = 1;
    }
    
    if(PB2 && !PB2_prev){
        PB2_event = 1;
    }
    
    if(PB3 && !PB3_prev && mode == MODE_ON){ // Disable PB3 in off mode becuase it keeps slipping through...
        PB3_event = 1;
    }
}