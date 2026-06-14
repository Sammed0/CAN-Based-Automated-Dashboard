#include "ecu1_sensor.h"
#include "adc.h"
#include "can.h"
#include "msg_id.h"
#include "uart.h"

/* Stores latest ADC reading */
static uint16_t adc_value = 0U;

/* Initial gear position is 1st gear */
static unsigned char gear_value = 1U;


/* Function : get_speed() */

uint16_t get_speed(void)
{
    /* Read ADC value from Channel 4 */
    adc_value = read_adc(CHANNEL4);

    /* Limit ADC value to valid 10-bit range */
    if (adc_value > 1023U)
    {
        adc_value = 1023U;
    }

    /* Convert ADC value to speed range (0 - 99 km/h) */
    unsigned long scaled = (unsigned long)adc_value * 99UL;
    uint16_t speed = (uint16_t)(scaled / 1023UL);

    /* Transmit speed only when collision has not occurred */
    if (gear_value != 0U)
    {
        uint8_t speed_data[1];

        /* Store speed in CAN data buffer */
        speed_data[0] = (uint8_t)speed;

        /* Send speed message through CAN */
        can_transmit(SPEED_MSG_ID, speed_data, 1u);

        __delay_ms(50);
    }

    return speed;
}

/* Function : get_gear_pos() */
unsigned char get_gear_pos(void)
{
    unsigned char key;
    unsigned char prev_gear;
    uint16_t current_speed;
    uint8_t gear_data[1];

    /* Read keypad switch status */
    key = read_switches(STATE_CHANGE);

    /* Save previous gear to detect gear changes */
    prev_gear = gear_value;

    /* SW1 : Gear UP */
    if (key == MK_SW1)
    {
        /* Increase gear from 1st to 5th */
        if (gear_value < 5U)
        {
            gear_value++;
        }
    }

    /* SW2 : Gear DOWN */
    else if (key == MK_SW2)
    {
        /* If in reverse, return to 1st gear */
        if (gear_value == 6U)
        {
            gear_value = 1U;
        }
        /* Normal gear decrement */
        else if (gear_value > 1U)
        {
            gear_value--;
        }
    }

    /* SW3 : Reverse Gear */
    else if (key == MK_SW3)
    {
        /* Calculate current speed */
        current_speed = (uint16_t)(((unsigned long)adc_value * 99UL) / 1023UL);

        /* Allow reverse only below 10 km/h */
        if (current_speed <= 10U)
        {
            gear_value = 6U;     /* Reverse gear */
        }
    }

    /* SW4 : Collision Detection */
    else if (key == MK_SW4)
    {
        /* Set gear to 0 indicating collision */
        gear_value = 0U;
    }

    /* Transmit gear data only if gear changed */
    if (gear_value != prev_gear)
    {
        /* Store gear value in CAN data buffer */
        gear_data[0] = gear_value;

        /* Send gear information through CAN */
        can_transmit(GEAR_MSG_ID, gear_data, 1u);

        __delay_ms(50);
    }

    return gear_value;
}