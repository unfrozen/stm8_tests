/*
 *  File name:  test_ping.c
 *  Date first: 11/05/2018
 *  Date last:  12/05/2018
 *
 *  Description: Test and example program for HC-SR04 ultrasonic range finder.
 *
 *  Author:     Richard Hodges
 *
 *  Copyright (C) 2018 Richard Hodges. All rights reserved.
 *  Permission is hereby granted for any use.
 *
 ******************************************************************************
 *
 */

#include "stm8s_header.h"

#include "lib_bindec.h"
#include "lib_clock.h"
#include "lib_delay.h"
#include "lib_ping.h"
#include "lib_uart.h"

void setup(void);

/* Using pin A2, D1, and D2 for triggers */

const IO_PIN pin_trig1 = {	/* A2 */
    &PA_ODR, 0x04
};
const IO_PIN pin_trig2 = {	/* D1 */
    &PD_ODR, 0x02
};
const IO_PIN pin_trig3 = {	/* D2 */
    &PD_ODR, 0x04
};

volatile char	clock_tenths;	/* 1/10 second counter 0-255 */
volatile char   clock_msecs;	/* millisecond counter */
volatile int	d1, d2, d3;	/* current distances */
volatile int	counts;
volatile char	flag_count;	/* got new distance count */

/* callbacks provided by lib_clock */

void clock_ms(void);	/* millisecond callback */
void clock_10(void);	/* 1/10 second callback */

/* callbacks provided by lib_ping */

void ping_cb1(int);	/* ping channel 1 callback */
void ping_cb2(int);	/* ping channel 2 callback */
void ping_cb3(int);	/* ping channel 3 callback */

void print_dist(int);
void wait_25ms(void);

/* Pins to use as triggers and callback functions */

const IO_CALL_INT ping_cfg[] = {
	{ &pin_trig1, ping_cb1 },
	{ &pin_trig2, ping_cb2 },
	{ &pin_trig3, ping_cb3 }
};

/******************************************************************************
 *
 *  Test the rangefinder library
 */

int main() {
    char	decimal[10];
    char	last_tenth;

    setup();
    clock_init(clock_ms, clock_10);
    ping_init(PING_CHAN1 | PING_CHAN2 | PING_CHAN3, ping_cfg);
    uart_init(BAUD_115200);

    last_tenth = 0;
    flag_count = 1;

    do {
	while (last_tenth == clock_tenths);
	last_tenth = clock_tenths;

	if (flag_count) {
	    flag_count = 0;

	    bin16_dec(counts, decimal);
	    decimal_rlz(decimal, 4);
	    uart_puts(decimal);
	    uart_put(' ');

	    print_dist(d1);
	    print_dist(d2);
	    print_dist(d3);

	    uart_crlf();
	}
	d1 = -1;	/* "no echo response" */
	d2 = -1;
	d3 = -1;

	ping_send(PING_CHAN1);
	wait_25ms();	/* separate the pings */
	ping_send(PING_CHAN2);
	wait_25ms();
	ping_send(PING_CHAN3);

    } while(1);
}

/******************************************************************************
 *
 *  Got new ping value (callback)
 *  in: round  trip time in microseconds
 *      -1 (0xffff) indicates no echo response.
 */

void ping_cb1(int val)
{
    d1 = val;
    if (val < 0)
	return;
    counts++;
    flag_count = 1;
}

void ping_cb2(int val)
{
    d2 = val;
    if (val < 0)
	return;
    counts++;
    flag_count = 1;
}
void ping_cb3(int val)
{
    d3 = val;
    if (val < 0)
	return;
    counts++;
    flag_count = 1;
}

/******************************************************************************
 *
 *  Wait 25 milliseconds between the triggers
 */

void wait_25ms(void)
{
    char	wait;

    wait = clock_msecs + 25;
    while (wait != clock_msecs);
}

/******************************************************************************
 *
 *  Millisecond callback
 */

void clock_ms(void)
{
    clock_msecs++;
}

/******************************************************************************
 *
 *  1/10 second callback
 */

void clock_10(void)
{
    clock_tenths++;
    if (clock_tenths & 3)
	PB_ODR |= 0x20;		/* LED off 75% */
    else
	PB_ODR &= 0xdf;		/* LED on 25% */
}

/******************************************************************************
 *
 *  Print distance
 */

void print_dist(int dist)
{
    char	decimal[6];

    if (dist < 0) {
	uart_puts(" no distance ");
	return;
    }
    bin16_dec(dist / PING_INCH, decimal);
    decimal_rlz(decimal, 4);
    uart_puts(decimal);
    uart_puts(" inches ");
}

/******************************************************************************
 *
 *  Board and globals setup
 */

void setup(void)
{
    CLK_CKDIVR = 0x00;	/* clock 16mhz if STM8S103 */

    clock_tenths = 0;
    counts = 0;

    PA_ODR = 0;
    PA_DDR = 0x04;	/* A2 is trigger */
    PA_CR1 = 0xff;
    PA_CR2 = 0x00;

    PB_DDR = 0x20;	/* output LED */
    PB_CR1 = 0xff;     	/* inputs have pullup, outputs not open drain */
    PB_CR2 = 0x00;	/* no interrupts, 2mhz output */

    PC_CR1 = 0xff;

    PD_DDR = 0x06;	/* D1, D2 are triggers */
    PD_CR1 = 0xff;

    __asm__ ("rim");
}
/* Available ports on STM8S103P3:
 *
 * A1..A3	A3 is HS
 * B4..B5	Open drain
 * C3..C7	HS
 * D1..D6	HS
 */
