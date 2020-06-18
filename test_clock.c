/*
 *  File name:  test_clock.c
 *  Date first: 06/17/2020
 *  Date last:  06/18/2020
 *
 *  Description: Test/Example to verify lib_clock.
 *
 *  Author:     Richard Hodges
 *
 *  Copyright (C) 2020 Richard Hodges. All rights reserved.
 *  Permission is hereby granted for any use.
 *
 *  Location:
 *  https://github.com/unfrozen/stm8_tests
 *
 ******************************************************************************
 *
 */

#include "stm8s_header.h"

#include "lib_bindec.h"
#include "lib_board.h"
#include "lib_clock.h"
#include "lib_uart.h"

volatile char	clock_tenths;	/* 1/10 second counter 0-255 */
volatile char   clock_msecs;	/* millisecond counter */

char *days[8] = {
		 "error",
		 "Sunday",
		 "Monday",
		 "Tuesday",
		 "Wednesday",
		 "Thursday",
		 "Friday",
		 "Saturday"
};
/* callbacks provided by lib_clock */

void clock_ms(void);	/* millisecond callback */
void clock_10(void);	/* 1/10 second callback */

void local_setup(void); /* setup for this project */

/******************************************************************************
 *
 *  Test the lib_clock library.
 *  Go through tests to verify the calendar function.
 *  Output results to UART.
 */

int main() {
    CLOCK_CAL	date;
    char	time[10];
    char	dec[6];
    char	last_tenth;

    board_init(0);
    local_setup();
    clock_init(clock_ms, clock_10);
    uart_init(BAUD_115200);

    uart_puts("Press any key to start calendar test.\r\n"
	      "This will print every day from year 2000\r\n"
	      "to the end of 2009. Turn on your capture file!\r\n");
    uart_get();
    last_tenth = 0;
    do {
	while (last_tenth == clock_tenths);
	last_tenth = clock_tenths;
	clock_string(time);
	uart_puts(time);
	uart_put('\r');

	if (uart_rsize() == 0)
	    continue;
	uart_puts("\r\nCalendar test start.\r\n");

	date.year  = 2000;
	date.month = 1;		/* January */
	date.date  = 1;		/* 1st */
	date.day   = 7;		/* Saturday */
	clock_cal_set(&date);

	while (date.year < 2022) {
	    bin16_dec(date.year, dec);
	    uart_puts(dec);
	    uart_put(' ');
	    
	    bin8_dec2(date.month, dec);
	    uart_puts(dec);
	    uart_put(' ');
	    
	    bin8_dec2(date.date, dec);
	    uart_puts(dec);
	    uart_put(' ');
	    
	    bin8_dec2(date.day, dec);
	    uart_puts(dec);
	    uart_put(' ');
	    uart_puts(days[date.day]);
	    uart_crlf();
	    
	    clock_inc_calendar();
	    clock_cal_get(&date);
	    if (date.date == 1)
		uart_crlf();	/* separate the months */
	}
	uart_puts("Calendar test complete.\n\r");
	while (1);
    } while (1);
}

/******************************************************************************
 *
 *  Millisecond callback from lib_clock
 */

void clock_ms(void)
{
    clock_msecs++;
}

/******************************************************************************
 *
 *  1/10 second callback from lib_clock
 */

void clock_10(void)
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

/******************************************************************************
 *
 * I/O Ports on STM8S103F:
 *
 * A1..A3       A3 is HS
 * B4..B5       Open drain
 * C3..C7       HS
 * D1..D6       HS
 *
 * Note: B4 and B5 are "true open drain" and need a real pull-up resistor
 * to be used as keypad I/O.
 *
 ******************************************************************************
 *
 *  Local setup
 */

void local_setup(void)
{
    /* This is where we set up IO ports, etc.
     *
     * PA_DDR \= 0x0e; // A1, A2, A3 are outputs
     * PA_CR1 \= 0x0e; // and push-pull
     */
}
