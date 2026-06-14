#ifndef MATRIX_KEYPAD_H
#define MATRIX_KEYPAD_H

#define _XTAL_FREQ          20000000

/*
 * PIC18F4580 ? Revised keypad layout
 * ====================================
 * RB2 and RB3 are reserved for CAN TX/RX and MUST NOT be used as GPIO.
 *
 * Only RB1 and RB4 are used as column inputs.
 * 3 rows × 2 columns = 6 keys (SW1-SW6).
 *
 * Wiring:
 *   ROW1 = RB5 (output)   COL1 = RB1 (input)
 *   ROW2 = RB6 (output)   COL2 = RB4 (input)
 *   ROW3 = RB7 (output)
 *
 * Key map:
 *          COL1(RB1)   COL2(RB4)
 *  ROW1     SW1          SW2
 *  ROW2     SW3          SW4
 *  ROW3     SW5          SW6
 *
 * Gear logic assignment (ecu1_sensor.c):
 *   SW1 = Gear UP
 *   SW2 = Gear DOWN
 *   SW3 = Reverse
 *   SW4 = Collision
 */

#define STATE_CHANGE        1
#define LEVEL_CHANGE        0

/* Rows: output via LATB */
#define ROW1                LATBbits.LATB5
#define ROW2                LATBbits.LATB6
#define ROW3                LATBbits.LATB7

/* Columns: input via PORTB ? RB2 and RB3 deliberately omitted (CAN pins) */
#define COL1                PORTBbits.RB1
#define COL2                PORTBbits.RB4

/* Switch identifiers */
#define MK_SW1              1   /* ROW1-COL1 : Gear UP      */
#define MK_SW2              2   /* ROW1-COL2 : Gear DOWN    */
#define MK_SW3              3   /* ROW2-COL1 : Reverse      */
#define MK_SW4              4   /* ROW2-COL2 : Collision    */
#define MK_SW5              5   /* ROW3-COL1 : spare        */
#define MK_SW6              6   /* ROW3-COL2 : spare        */

#define ALL_RELEASED        0xFF

#define HI                  1
#define LO                  0

void init_matrix_keypad(void);
unsigned char scan_key(void);
unsigned char read_switches(unsigned char detection_type);

#endif