#include <xc.h>
#include "matrix_keypad.h"

void init_matrix_keypad(void)
{
    /* Configure PORTB pins as digital I/O */
    ADCON1 = 0x0F;

    /*
     * TRISB configuration:
     *   RB7 = output (ROW3)  ? 0
     *   RB6 = output (ROW2)  ? 0
     *   RB5 = output (ROW1)  ? 0
     *   RB4 = input  (COL2)  ? 1
     *   RB3 = input  (CAN RX)? 1   ? managed by CAN module, leave as input
     *   RB2 = output (CAN TX)? 0   ? managed by CAN module, leave as output
     *   RB1 = input  (COL1)  ? 1
     *   RB0 = input           ? 1  (unused, safe as input)
     *
     *   Binary: 0001_1011 = 0x1B
     *
     * NOTE: init_can() sets TRISB2=0 and TRISB3=1 independently.
     *       Call init_matrix_keypad() BEFORE init_can() so the CAN
     *       init has the final say on RB2/RB3 direction.
     */
    TRISB = 0x1B;

    /* Drive all rows HIGH (idle state) */
    LATBbits.LATB5 = HI;
    LATBbits.LATB6 = HI;
    LATBbits.LATB7 = HI;

    /* Enable weak pull-ups on PORTB (clears RBPU bit in INTCON2) */
    INTCON2bits.RBPU = 0;

    /* Wait for pull-ups to charge column lines */
    __delay_ms(50);
}

unsigned char scan_key(void)
{
    unsigned char result = 0xFF;

    /* --- ROW1 (RB5) low --- */
    LATBbits.LATB5 = LO;
    LATBbits.LATB6 = HI;
    LATBbits.LATB7 = HI;
    __delay_us(100);

    if      (PORTBbits.RB1 == LO) result = MK_SW1;  /* ROW1-COL1 */
    else if (PORTBbits.RB4 == LO) result = MK_SW2;  /* ROW1-COL2 */

    /* --- ROW2 (RB6) low --- */
    if (result == 0xFF)
    {
        LATBbits.LATB5 = HI;
        LATBbits.LATB6 = LO;
        LATBbits.LATB7 = HI;
        __delay_us(100);

        if      (PORTBbits.RB1 == LO) result = MK_SW3;  /* ROW2-COL1 */
        else if (PORTBbits.RB4 == LO) result = MK_SW4;  /* ROW2-COL2 */
    }

    /* --- ROW3 (RB7) low --- */
    if (result == 0xFF)
    {
        LATBbits.LATB5 = HI;
        LATBbits.LATB6 = HI;
        LATBbits.LATB7 = LO;
        __delay_us(100);

        if      (PORTBbits.RB1 == LO) result = MK_SW5;  /* ROW3-COL1 */
        else if (PORTBbits.RB4 == LO) result = MK_SW6;  /* ROW3-COL2 */
    }

    /* Restore all rows HIGH */
    LATBbits.LATB5 = HI;
    LATBbits.LATB6 = HI;
    LATBbits.LATB7 = HI;

    return result;
}

unsigned char read_switches(unsigned char detection_type)
{
    static unsigned char once = 1;
    unsigned char key;

    if (detection_type == STATE_CHANGE)
    {
        key = scan_key();
        if (key != 0xFF && once)
        {
            once = 0;
            return key;
        }
        else if (key == 0xFF)
        {
            once = 1;
        }
    }
    else if (detection_type == LEVEL_CHANGE)
    {
        return scan_key();
    }

    return 0xFF;
}