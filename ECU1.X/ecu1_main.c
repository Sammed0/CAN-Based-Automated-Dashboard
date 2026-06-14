#include <xc.h>
#include "adc.h"
#include "can.h"
#include "ecu1_sensor.h"
#include "matrix_keypad.h"
#include "msg_id.h"
#include "uart.h"
#include "CLCD.h"

/* ---------- String constants ---------- 
            CLCD Display Messages
*/
static const unsigned char msg_speed[]   = {"SPEED"};
static const unsigned char msg_gear[]    = {"GEAR"};
static const unsigned char msg_crash[]   = {"COLLISION! HALT"};
static const unsigned char msg_limit[]   = {"SPEED LIMIT!    "};
static const unsigned char msg_ready[]   = {"SYSTEM READY    "};
static const unsigned char msg_reverse[] = {"REVERSE GEAR    "};
static const unsigned char msg_deconly[] = {"DECREMENT ONLY  "};

/* String Length Definitions*/
#define LEN_SPEED   5u   /* strlen("SPEED") */
#define LEN_GEAR    4u   /* strlen("GEAR")  */

/* CLCD Cursor Positions*/
#define POS_SPEED_VAL   (LEN_SPEED + 1u)
#define POS_GEAR_VAL    (LEN_GEAR  + 1u)

/*  Function : clcd_clear() */
static void clcd_clear(void)
{
    clcd_write(0x01u, instruction);
    __delay_ms(2);
}

/*Function : print_speed()*/
static void print_speed(uint16_t speed)
{
    unsigned char buf[3];
    
    /* Extract tens digit */
    buf[0] = (unsigned char)((speed / 10u) % 10u + '0');
    
     /* Extract ones digit */
    buf[1] = (unsigned char)( speed % 10u + '0');
    buf[2] = '\0';
    
     /* Display speed value */
    clcd_print(buf, Line1((unsigned char)POS_SPEED_VAL));
}

/* Function : init_config() */
static void init_config(void)
{
    /* Initialize Matrix Keypad */
    init_matrix_keypad();

    /* Initialize ADC Module */
    init_adc();

    /* Initialize CAN Module */
    init_can();

    /* Initialize UART Module */
    init_uart();

    /* Initialize CLCD Module */
    init_clcd();

    /* Display startup message */
    clcd_print(msg_ready, Line1(0));

    __delay_ms(1500);

    clcd_clear();
}

/* ---------- Main ---------- */
int main(void)
{
    /* Stores current vehicle speed */
    uint16_t car_speed = 0u;

    /* Stores current gear position */
    unsigned char car_gear = 1u;

    /* Stores previous gear position */
    unsigned char prev_gear = 0xFFu;

    /* Speed limit warning flag */
    unsigned char speed_limit_flag = 0u;

    /* Initialize all peripherals */
    init_config();

     while (1)
    {
        /*--------------------------------------
         * Read vehicle speed from ADC
         * and transmit via CAN
         *-------------------------------------*/
        car_speed = get_speed();

        /*--------------------------------------
         * Read gear position from keypad
         * and transmit via CAN
         *-------------------------------------*/
        car_gear = get_gear_pos();

        /*--------------------------------------
         * Collision Detection
         * Gear 0 indicates collision
         *-------------------------------------*/
        if (car_gear == COLLISION_GEAR)
        {
            clcd_clear();

            clcd_print(msg_crash, Line1(0));

            /* Stop vehicle */
            car_speed = 0u;

            /* Halt system permanently */
            while (1);
        }

        /*--------------------------------------
         * Refresh display when gear changes
         *-------------------------------------*/
        if (car_gear != prev_gear)
        {
            clcd_clear();

            prev_gear = car_gear;

            speed_limit_flag = 0u;
        }

        /*--------------------------------------
         * Reverse Gear Display
         *-------------------------------------*/
        if (car_gear == REVERSE_GEAR)
        {
            clcd_print(msg_reverse, Line1(0));

            clcd_print(msg_deconly, Line2(0));
        }
        else
        {
            /*----------------------------------
             * Speed Limit Warning
             *---------------------------------*/
            if (car_speed >= 99u)
            {
                if (!speed_limit_flag)
                {
                    clcd_clear();

                    speed_limit_flag = 1u;
                }

                clcd_print(msg_limit, Line1(0));
            }
            else
            {
                if (speed_limit_flag)
                {
                    clcd_clear();

                    speed_limit_flag = 0u;
                }

                /*------------------------------
                 * Display Speed
                 * Format: SPEED=XX
                 *-----------------------------*/
                clcd_print(msg_speed, Line1(0));

                clcd_putch('=',
                           Line1((unsigned char)LEN_SPEED));

                print_speed(car_speed);

                /* Clear remaining characters */
                clcd_print((const unsigned char *)"     ",
                           Line1((unsigned char)(POS_SPEED_VAL + 3u)));
            }

            /*----------------------------------
             * Display Gear
             * Format: GEAR=X
             *---------------------------------*/
            clcd_print(msg_gear, Line2(0));

            clcd_putch('=',
                       Line2((unsigned char)LEN_GEAR));

            clcd_putch((unsigned char)(car_gear + '0'),
                       Line2((unsigned char)POS_GEAR_VAL));

            /* Clear remaining characters */
            clcd_print((const unsigned char *)"          ",
                       Line2((unsigned char)(POS_GEAR_VAL + 1u)));
        }
    }
}