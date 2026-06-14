#include <xc.h>
#include <string.h>
#include "message_handler.h"
#include "msg_id.h"
#include "can.h"
#include "clcd.h"
#include <stdio.h>
#include <stdint.h>

// Global flags and states
volatile unsigned char led_state = LED_OFF; // LED state tracker
volatile unsigned char status = e_ind_off; // Indicator status

volatile unsigned char c_flag = 0;  // Collision flag

// Gear display strings
char *gear_array[] =
{
    "C ",   // 0 = Collision
    "G1",   // gear 1
    "G2",   // gear 2
    "G3",   // gear 3
    "G4",   // gear 4
    "G5",   // gear 5
    "GR"    // reverse gear 
};

//Receive can speed message
void handle_speed_data(uint8_t *data, uint8_t len)
{
    // Ignore empty frames
    if(len < 1)
    {
        return;
    }
    
    
    //Implement the speed function
    char speed_str[3];
    
    if(c_flag)  // Collision active ? force speed = 00
    {
       sprintf(speed_str, "%02d", 0);
    }
    else
    {
        sprintf(speed_str, "%02d", data[0]); // Format speed value
    }
    
    clcd_print((unsigned char *)speed_str, LINE2(0));// Display on LCD
}

//Receive Gear message
void handle_gear_data(uint8_t *data, uint8_t len) 
{
    if(len < 1)
    {
        return;
    }
    
    if(data[0] == 0)// Collision gear
    {
        c_flag = 1; // Set collision flag
        clcd_print((unsigned char *)gear_array[0], LINE2(3)); // Show "C "
        clcd_print((unsigned char *)"00", LINE2(0)); // Force speed 00
    }
    //Implement the gear function
    else
    {
        c_flag = 0;
        if(data[0] <= 6)//valid gear index
        {
            clcd_print((unsigned char *)gear_array[data[0]], LINE2(3));//display gear
        }
    }
}

//Receive rpm message
void handle_rpm_data(uint8_t *data, uint8_t len) 
{
    if(len < 1)
    {
        return;
    }
    
    char rpm_str[5];//buffer for rpm string
    uint16_t rpm;

    if(c_flag)   // collision flag set ? force 0000
    {
        rpm = 0U;
    }
    else
    {
        rpm = (uint16_t)data[0] * 1000U; // Scale raw value

        // Clamp minimum to 1000 in normal operation
        if(rpm < 1000U)
        {
            rpm = 1000U;
        }
    }

    sprintf(rpm_str, "%04u", rpm); // Clamp minimum RPM
    clcd_print((unsigned char *)rpm_str, LINE2(6)); //Display on clcd
}

//Receives engine temperature message
void handle_engine_temp_data(uint8_t *data, uint8_t len) 
{
    if(len < 1)
    {
        return;
    }
    
    //Implement the temperature function
    char temp_str[3];
    
    sprintf(temp_str, "%02u", data[0]);//format for temp
    
    clcd_print((unsigned char *)temp_str, LINE2(11));//display on clcd
}

//Receive indicator message
void handle_indicator_data(uint8_t *data, uint8_t len) 
{
    //Implement the indicator function
    if(len < 1)
    {
        return;
    }
   
    switch (data[0]) // Decode indicator state
    {
        case e_ind_left://left indicator
            clcd_print((unsigned char *) "<-", LINE2(14));
            break;

        case e_ind_right://right indicator
            clcd_print((unsigned char *) "->", LINE2(14));
            break;

        case e_ind_both://right and left indicator
            clcd_print((unsigned char *) "<>", LINE2(14));
            break;

        case e_ind_off://indicator off
            clcd_print((unsigned char *) "  ", LINE2(14));
            break;
    }
}

// Dispatcher for CAN messages
void process_canbus_data(void)
{
    uint16_t msg_id;//Message id
    uint8_t data[8];//data buffer
    uint8_t len;//data length

    can_receive(&msg_id, data, &len);//Receive can frame
    
    if(len == 0)
    {
        return;
    }
    
    // Route message to appropriate handler
    switch (msg_id) 
    {
        case SPEED_MSG_ID:
            handle_speed_data(data, len);
            break;

        case GEAR_MSG_ID:
            handle_gear_data(data, len);
            break;

        case RPM_MSG_ID:
            handle_rpm_data(data, len);
            break;

        case ENG_TEMP_MSG_ID:
            handle_engine_temp_data(data, len);
            break;

        case INDICATOR_MSG_ID:
            handle_indicator_data(data, len);
            break;
    }
}