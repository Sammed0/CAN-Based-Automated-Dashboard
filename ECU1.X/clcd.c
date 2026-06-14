#include <xc.h>
#include "CLCD.h"

void clcd_write(unsigned char byte, unsigned char type)
{
    /* Set RW=0 (write mode), RS=type (instruction or data) */
    RW = 0;
    RS = type;
 
    /* Put data on PORTD */
    port_D = byte;
 
    /* EN pulse - minimum 450ns high pulse at 20MHz */
    EN = 1;
    __delay_us(1);
    EN = 0;
    __delay_us(1);
 
    /* Poll busy flag on RD7 (PORTD bit 7, NOT TRISD7) */
    RW = 1;               /* Switch to read mode        */
    BUSY_FLAG_TRIS = 1;   /* Make RD7 an input          */
    RS = 0;               /* Select instruction register */
 
    do {
        EN = 1;
        __delay_us(1);
        EN = 0;
        __delay_us(1);
    } while (BUSY_FLAG == 1);   /* Wait until BF=0 (LCD ready) */
 
    /* Restore RD7 as output and switch back to write mode */
    RW = 0;
    BUSY_FLAG_TRIS = 0;
}
 
void init_clcd(void)
{
    /* PORTD all output for data bus */
    tris_D = OUTPUT;
 
    /* RC0=RW, RC1=RS, RC2=EN must be outputs - use AND to CLEAR those bits */
    tris_C = tris_C & 0xF8;
 
    /* Power-on delay: LCD needs > 30ms after VCC rises */
    __delay_ms(40);
 
    /*
     * 8-bit initialization sequence (HD44780 datasheet):
     * Send 0x30 three times WITHOUT busy polling (LCD not ready yet).
     * Drive manually: set RW=0, RS=0, data, EN pulse.
     */
    RW = 0;
    RS = 0;
 
    port_D = 0x30;
    EN = 1; __delay_us(1); EN = 0;
    __delay_ms(5);          /* Must wait > 4.1ms after first attempt */
 
    port_D = 0x30;
    EN = 1; __delay_us(1); EN = 0;
    __delay_us(200);        /* Must wait > 100us after second attempt */
 
    port_D = 0x30;
    EN = 1; __delay_us(1); EN = 0;
    __delay_us(200);        /* Third attempt, LCD now in 8-bit mode  */
 
    /* From here busy polling works - use clcd_write() normally */
 
    /* Function Set: 8-bit bus, 2 display lines, 5x8 font */
    clcd_write(0x38, instruction);
 
    /* Display OFF */
    clcd_write(0x08, instruction);
 
    /* Clear Display - needs > 1.64ms to complete */
    clcd_write(0x01, instruction);
    __delay_ms(2);
 
    /* Entry Mode Set: increment cursor, no display shift */
    clcd_write(0x06, instruction);
 
    /* Display ON, cursor OFF, blink OFF */
    clcd_write(0x0C, instruction);
}
 
void clcd_putch(const unsigned char info, unsigned char address)
{
    clcd_write(address, instruction);   /* Set DDRAM address */
    clcd_write(info, data);             /* Write character   */
}
 
void clcd_print(const unsigned char *info, unsigned char address)
{
    clcd_write(address, instruction);   /* Set DDRAM address */
    while (*info != '\0')
    {
        clcd_write(*info++, data);
    }
}
 