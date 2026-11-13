#include "TimeDelay.h"
#include "IOS.h"
#include "UART2.h"
#include "ADC.h"

extern uint16_t skip_next_trigger;
extern uint16_t waiting_trigger;
extern uint16_t T1_done;
extern uint16_t transmitting;
extern uint16_t new_adc_reading;
extern duty_cycle_t duty_cycle;


/**
 * Took some time to clean up some misinfo in the comments.
 */
void T1_init(){

    T1CONbits.TCKPS0 = 1; 
    T1CONbits.TCKPS1 = 1;
    T1CONbits.TCS = 0; 
    T1CONbits.TSIDL = 0; 
  
    IPC0bits.T1IP = 3; 
    IFS0bits.T1IF = 0; 
    IEC0bits.T1IE = 1; 
    
    PR2 = 65535;
    TMR1 = 0; 
}

void T2_init(){
    
    T2CONbits.T32 = 0; 
    T2CONbits.TCKPS0 = 0; // Everything else is prescaled 1:256, 
    T2CONbits.TCKPS1 = 1; // Prescale this 1:64 (might go lower) so we can get nice fast PWMs where blinking isn't visible.
    T2CONbits.TCS = 0; 
    T2CONbits.TSIDL = 0;  
    
    IPC1bits.T2IP = 3; 
    IFS0bits.T2IF = 0; 
    IEC0bits.T2IE = 1; 
    
    PR2 = 65535;
    TMR2 = 0; 
}

void T3_init(){
    
    T2CONbits.T32 = 0; 
    T3CONbits.TCKPS0 = 1;
    T3CONbits.TCKPS1 = 1;
    T3CONbits.TCS = 0; 
    T3CONbits.TSIDL = 0; 
    
    IPC2bits.T3IP = 2;
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 1; 
    
    PR3 = 65535; 
    TMR3 = 0;
}

/**
 * Creates two scenarios, one where the button is held more than time_ms and one
 * where it is held less. This allows two different events to occur using only one button.
 * In the case where the button is released before the timer finished, this function should be called with
 * time_ms = 0. 
 * During the wait, this function may continue transmitting through UART. This is specifically
 * for the case where timer_ms > time to transmit in main while loop.
 * @param time_ms
 */
void T1_button_hold(uint16_t time_ms){
    TMR1 = 0;
    T1_done = 0;
    if(time_ms == 0){
        T1CONbits.TON = 0;
        return;
    }
    PR1 = time_ms * 15.625;       
    T1CONbits.TON = 1;
    // Might wanna let main handle the idling if an idle in here is causing issues.
    // Also while in here it's blocking, meaning the ADC won't update!
    skip_next_trigger = 1;
    while(skip_next_trigger){ // Attempted to update the ADC while button holding, ended badly.
//        if(transmitting){
//            transmit(duty_cycle.on_cycle, duty_cycle.off_cycle, new_adc_reading);
//            T3_delay_ms(100);
//        }
//        else{
            Idle(); 
//        }
    }
}

// not even sure what to do with him tbh.
void T2_PWM(uint16_t time_quarter_ms){
    TMR2 = 0;
    if(time_quarter_ms == 0){
        T2CONbits.TON = 0;
        return;
    }
    PR2 = time_quarter_ms * 15.625;       
    T2CONbits.TON = 1;

}

void T3_delay_ms(uint16_t time_ms){
    PR3 = time_ms * 15.625;       
    T3CONbits.TON = 1;
    waiting_trigger = 1;
    // skip_next_trigger = 1;
    while(waiting_trigger || (skip_next_trigger && waiting_trigger)){
        Idle();
    }
}
