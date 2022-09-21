/*
 *  File name:  test_i2c.c
 *  Date first: 09/21/2022
 *  Date last:  09/21/2022
 *
 *  Description: Test and example program for STM8 I2C library.
 *
 *  Author:     Richard Hodges
 *
 *  Copyright (C) 2022 Richard Hodges. All rights reserved.
 *  Permission is hereby granted for any use.
 *
 ******************************************************************************
 *
 *  Test the I2C functions
 *
 *  I2C clock is pin D2
 *  I2C data is pin D3
 *
 *  Every second, send a series of I2C bytes.
 *  Verify with decoding oscilloscope.
 */

#include "stm8s_header.h"

#include "lib_board.h"
#include "lib_clock.h"
#include "lib_i2c.h"

void timer_ms(void);	/* millisecond timer call */
void timer_10(void);	/* 1/10 second timer call */

volatile unsigned int clock_tenths;

void local_setup(void);
void send_series(unsigned char, unsigned char);

/******************************************************************************
 *
 *  Test the I2C library.
 */

int main() {
    int		clock_last, diff;
    unsigned char i2c_val = 0;
    
    board_init(0);
    local_setup();
    clock_init(timer_ms, timer_10);
    i2c_init();

    clock_last = clock_tenths;
    for (;;) {
	diff = clock_tenths - clock_last;
	if (diff < 10)
	    continue;
	clock_last = clock_tenths;
	send_series(i2c_val, 4);
	i2c_val += 4;
    }
}

/******************************************************************************
 *
 *  Send series of bytes
 *  in: first value, byte count
 */

void send_series(unsigned char val, unsigned char count)
{
    i2c_start();
    while (count--) {
	i2c_txbit8(val++);
	i2c_getack();
    }
    i2c_stop();
}

/******************************************************************************
 *
 *  Board and globals setup
 */

void local_setup(void)
{
    clock_tenths = 0;
#ifdef EXAMPLE
    PORT_DDR |= PORT_MASK;	/* Pin is output. */
    PORT_CR1 |= PORT_MASK;	/* Pin is push/pull. */
    PORT_CR2 |= PORT_MASK;	/* Pin is fast mode (10mhz). */
#endif
}
/* Available ports on STM8S103:
 *
 * A1..A3	A3 is HS
 * B4..B5	Open drain
 * C3..C7	HS
 * D1..D6	HS
 *
 ******************************************************************************
 *
 *  Millisecond timer callback
 */

void timer_ms(void)
{
}

/******************************************************************************
 *
 *  Tenths second timer callback
 */

void timer_10(void)
{
   static char blink;
    
    clock_tenths++;

    blink++;
    if (blink < 4) {
        board_led(blink & 1);   /* blink twice */
        return;
    }
    board_led(0);               /* off for 7/10 second */
    if (blink < 10)
        return;
    blink = 0;
}
