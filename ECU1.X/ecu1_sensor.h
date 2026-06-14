#ifndef ECU1_SENSOR_H
#define ECU1_SENSOR_H

#include <stdint.h>
#include <xc.h>
#include "matrix_keypad.h"   /* read_switches(), MK_SW1..MK_SW4 */
#include "adc.h"              /* CHANNEL4                         */

/* Maximum forward gear */
#define MAX_GEAR        5U
/* Reverse gear code */
#define REVERSE_GEAR    6U
/* Collision gear code (triggers system halt) */
#define COLLISION_GEAR  0U

/* Speed below which reverse is allowed (km/h) */
#define REVERSE_SPEED_LIMIT  10U

uint16_t      get_speed(void);
unsigned char get_gear_pos(void);

#endif