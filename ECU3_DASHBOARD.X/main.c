#include <xc.h>
#include <stdint.h>
#include "can.h"
#include "clcd.h"
#include "msg_id.h"
#include "message_handler.h"
//#include "isr.h"
#include "timer0.h"

// Function to initialize LEDs
static void init_leds(void) {
    TRISB = 0x08; // Set RB2 as output, RB3 as input, remaining as output
    PORTB = 0x00;// Clear PORTB (all outputs low)
}

// Function to initialize all peripherals
static void init_config(void) {
    // Initialize CLCD and CANBUS
    init_clcd();
    init_can();
    init_leds();

    // Enable Interrupts
    PEIE = 1;// Enable peripheral interrupts
    GIE = 1;// Enable global interrupts
    init_timer0();// Initialize Timer0
}


void main(void) {
    // Initialize peripherals
    init_config();
    
    CLEAR_DISP_SCREEN;   // Clear LCD display
    
    clcd_print((unsigned char *)"SP", LINE1(0));//Speed
    clcd_print((unsigned char *)"GR", LINE1(3));//Gear
    clcd_print((unsigned char *)"RPM", LINE1(6));//Rpm
    clcd_print((unsigned char *)"TP", LINE1(11));//Temprature
    clcd_print((unsigned char *)"IN", LINE1(14));//Indicator
    
    /* ECU1 main loop */
    while (1) {
        // Read CAN Bus data and handle it
        process_canbus_data();
    }

    return;
}
