#include "ecu2_sensor.h"
#include "adc.h"
#include "can.h"
#include "msg_id.h"
#include "uart.h"
#include "matrix_keypad.h"

void print_rpm(uint16_t data)
{
    if (data >= 8000U)
        data = 8000U;

    unsigned char rpm[5];
    rpm[0] = (unsigned char)((data / 1000u) % 10u + '0');
    rpm[1] = (unsigned char)((data / 100u)  % 10u + '0');
    rpm[2] = (unsigned char)((data / 10u)   % 10u + '0');
    rpm[3] = (unsigned char)((data / 1u)    % 10u + '0');
    rpm[4] = '\0';
    puts("RPM = ");
    puts((const char *)rpm);
    puts("\n\r");
}

void print_temp(uint16_t data)
{
    if (data >= 99U)
        data = 99U;

    unsigned char temp[3];
    temp[0] = (unsigned char)((data / 10u) % 10u + '0');
    temp[1] = (unsigned char)( data % 10u         + '0');
    temp[2] = '\0';
    puts("TEMP = ");
    puts((const char *)temp);
    puts(" C\n\r");
}

void init_config(void)
{
    init_adc();
    init_can();
    init_digital_keypad();
    init_matrix_keypad();
    ADCON1 = 0x0D;       /* Re-apply after matrix keypad init overwrites it */
    TRISB = TRISB & 0x0F;
    PORTB = PORTB & 0x0F; 
    init_uart();

    puts("ECU2 READY\n\r");
}

int main(void)
{
    init_config();

    uint16_t rpm_value;
    uint16_t temp_value;
    IndicatorStatus indicator_value = e_ind_off;
    IndicatorStatus pre_value       = e_ind_off;  /* initialized! */

    while (1)
    {
        /* --- RPM --- */
        rpm_value = get_rpm();
        print_rpm(rpm_value);

        uint8_t rpm_data[1];
        rpm_data[0] = (uint8_t)(rpm_value / 1000U);  /* sends 0-8 range */
        can_transmit(RPM_MSG_ID, rpm_data, 1);
        __delay_ms(50);

        /* --- Temperature --- */
        temp_value = get_engine_temp();
        print_temp(temp_value);
        uint8_t temp_data[1];
        temp_data[0] = (uint8_t)temp_value;
        can_transmit(ENG_TEMP_MSG_ID, temp_data, 1);
        __delay_ms(50);
        
        /* --- Indicator --- */
        indicator_value = process_indicator();

        puts("Indicator = ");
        switch (indicator_value)
        {
            case e_ind_off:   puts("INDICATOR IS OFF\n\r");  break;
            case e_ind_left:  puts("LEFT ===>\n\r");         break;
            case e_ind_right: puts("<===RIGHT\n\r");         break;
            case e_ind_both:  puts("<===BOTH ON===>\n\r");   break;
            default:                                         break;
        }

        /* Transmit CAN only when indicator state changes */
        if (indicator_value != pre_value)
        {
            uint8_t indicator_data[1];
            indicator_data[0] = (uint8_t)indicator_value;
            can_transmit(INDICATOR_MSG_ID, indicator_data, 1);
            __delay_ms(50);
            
            pre_value = indicator_value;   /* update after transmit */
        }
    }

    return 0;
}