/*
 *  File name:  test_max6675.c
 *  Date first: 12/12/2022
 *  Date last:  12/12/2022
 *
 *  Description: Test and example program for MAX6675 thermocouple library.
 *
 *  Author:     Richard Hodges
 *
 *  Copyright (C) 2022 Richard Hodges. All rights reserved.
 *  Permission is hereby granted for any use.
 *
 ******************************************************************************
 *
 *  Test the MAX6675.
 *  Read temperature every second and output to UART.
 *  Encoded result bits can be checked with oscilloscope.
 *
 *  UART pins:
 *  TX is pin D5
 *  RX is pin d6
 *
 *  MAX6675 pins:
 *  D3: SPI clock (out)
 *  A1: SPI in (MISO)
 *  A2: CS* (slow pin)
 */

#include "stm8s_header.h"

#include "lib_bindec.h"
#include "lib_board.h"
#include "lib_clock.h"
#include "lib_max6675.h"
#include "lib_uart.h"

void timer_ms(void);	/* millisecond timer call */
void timer_10(void);	/* 1/10 second timer call */

volatile unsigned int clock_tenths;

void local_setup(void);

void show_temp(void);	/* Output current temperature. */

/******************************************************************************
 *
 *  Test the MAX6675 library.
 */

int main() {
    int		clock_last, diff;
    
    board_init(0);
    local_setup();
    clock_init(timer_ms, timer_10);
    max6675_init();
    uart_init(BAUD_115200);

    uart_puts("Now testing MAX6675 thermocouple device.\r\n");

    clock_last = clock_tenths;
    for (;;) {
	diff = clock_tenths - clock_last;
	if (diff < 10)
	    continue;
	clock_last = clock_tenths;
	show_temp();
    }
}

/******************************************************************************
 *
 *  Show current temperature.
 */

void show_temp(void)
{
    int16_t	tempc;		/* Temperature, in 0.25C */
    char	dec[9];
    const char *frac[4] = {".00", ".25", ".50", ".75"};

    tempc = max6675_read();
    if (tempc == MAX6675_ERROR) {
	uart_puts("Error: Check thermocouple.\r\n");
	return;
    }
    /* Show temperature in Celcius. */
    uart_puts(bin16_dec_rlz(tempc >> 2, dec));
    uart_puts((char *)frac[tempc & 3]);
    uart_puts("C    ");
    
    /* Show temperature in Fahrenheit. */
    tempc *= 9;
    tempc /= 5;
    tempc += 32 << 2;
    uart_puts(bin16_dec_rlz(tempc >> 2, dec));
    uart_puts((char *)frac[tempc & 3]);
    uart_puts("F\r\n");
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
