#include "ecu2_sensor.h"
#include "adc.h"
#include "can.h"
#include "msg_id.h"
#include "uart.h"

static uint16_t adc_value_rpm  = 0U;
static uint16_t adc_value_temp = 0U;
static IndicatorStatus indicator_value = e_ind_off;
static unsigned char key;

uint16_t get_rpm(void)
{
    adc_value_rpm = read_adc(CHANNEL4);

    if (adc_value_rpm > 1023U)
        adc_value_rpm = 1023U;

    unsigned long output = adc_value_rpm * 8000UL;
    uint16_t rpm = (uint16_t)(output / 1023UL);

    return rpm;
}

uint16_t get_engine_temp(void)
{
    uint32_t voltage_mv;
    uint16_t temp_tenths;

    adc_value_temp = read_adc(CHANNEL6);

    /* LM35: 10mV per degree C
     * voltage_mv = (adc * 5000) / 1024
     * temp_C     = voltage_mv / 10
     */
    voltage_mv  = ((uint32_t)adc_value_temp * 5000U) / 1024U;
    temp_tenths = (uint16_t)(voltage_mv / 10U);

    if (temp_tenths >= 99U)
        temp_tenths = 99U;

    return temp_tenths;
}

IndicatorStatus process_indicator(void)
{
    /* Use LEVEL so we don't miss keypresses in slow loop */
    key = read_digital_keypad(LEVEL);

    if (key == SWITCH1)
    {
        indicator_value = e_ind_left;
        PORTB = (PORTB & 0x0F) | 0x30;    /* RB4, RB5 ON  (LEFT)       */
    }
    else if (key == SWITCH2)
    {
        indicator_value = e_ind_right;
        PORTB = (PORTB & 0x0F) | 0xC0;    /* RB6, RB7 ON  (RIGHT)      */
    }
    else if (key == SWITCH3)
    {
        indicator_value = e_ind_both;
        PORTB = PORTB | 0xF0;              /* RB4,RB5,RB6,RB7 ON (BOTH) */
    }
    else if (key == SWITCH4)
    {
        indicator_value = e_ind_off;
        PORTB = PORTB & 0x0F;              /* RB4,RB5,RB6,RB7 OFF       */
    }

    return indicator_value;
}