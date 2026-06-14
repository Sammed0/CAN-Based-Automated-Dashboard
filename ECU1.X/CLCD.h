#ifndef CLCD_H
#define	CLCD_H
#define _XTAL_FREQ          20000000
 
/* Port mappings */
#define port_D              PORTD
#define tris_D              TRISD
#define tris_C              TRISC
 
/* Control pins on PORTC */
#define RS                  RC1
#define RW                  RC0
#define EN                  RC2
 
/* Busy flag pin - READ from PORT not TRIS */
#define BUSY_FLAG           PORTDbits.RD7
#define BUSY_FLAG_TRIS      TRISD7
 
/* Type selectors */
#define instruction         0
#define data                1
#define INPUT               0xFF
#define OUTPUT              0x00
 
/* DDRAM address macros */
#define Line1(x)            (0x80 + (x))
#define Line2(x)            (0xC0 + (x))
 
/* Function prototypes */
void init_clcd(void);
void clcd_write(unsigned char byte, unsigned char type);
void clcd_print(const unsigned char *info, unsigned char address);
void clcd_putch(const unsigned char info, unsigned char address);
 
#endif

