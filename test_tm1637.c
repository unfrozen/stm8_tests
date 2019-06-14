/*
 *  File name:  test_tm1637.c
 *  Date first: 06/10/2018
 *  Date last:  06/14/2019
 *
 *  Description: Test and example program for TM1637 library.
 *
 *  Author:     Richard Hodges
 *
 *  Copyright (C) 2018, 2019 Richard Hodges. All rights reserved.
 *  Permission is hereby granted for any use.
 *
 *  This code is derived from test_tm1638.
 *
 ******************************************************************************
 *
 */

#include "stm8s_header.h"

#include "lib_bindec.h"
#include "lib_clock.h"
#include "lib_tm1637.h"

void timer_ms(void);	/* millisecond timer call */
void timer_10(void);	/* 1/10 second timer call */

char clock_tenths;
char clock_last;
volatile char clock_ms;

void setup(void);

/* Uncomment to show words instead of numbers */
#define SHOW_WORDS

/* Uncomment to test blink function */
//#define TEST_BLINK	/* test blink function: 8 seconds on, 8 off */

static void show_status(void);

#pragma disable_warning 196	/* "pointer lost const" */

static char module_type;

const char *words[];

/******************************************************************************
 *
 *  Display a simulated clock on LED device
 */

int main() {
    char	 decimal[12];
    char	 tenths;
    int		 count16;
    char	 wptr;

    setup();
    tm1637_init();
    tm1637_bright(7);
    clock_init(timer_ms, timer_10);

    count16 = 0;
    tenths = 0;
    wptr = 0;

    do {
	if (clock_last == clock_tenths)
	    continue;
	clock_last = clock_tenths;
	tenths++;
	PB_ODR ^= 0x20;
	if (tenths == 5)
	    tm1637_colon(0);	/* colon off for half second */
	if (tenths < 10)
	    continue;
	tenths = 0;

	count16++;
	tm1637_curs(0);
#ifdef SHOW_WORDS
	tm1637_puts(words[wptr]);
	wptr++;
	if (!words[wptr])
	    wptr = 0;
#else /* SHOW_WORDS */

/* Every 4 seconds, switch between decimal count and clock string */

	if (count16 & 4) {
	    clock_string(decimal);
	    tm1637_puts(decimal + 3);
	}
	else {
	    bin16_dec_rlz(count16, decimal);
	    tm1637_puts(decimal + 1);
	}
#endif /* SHOW_WORDS */

#ifdef TEST_BLINK
	if (count16 & 7)
	    continue;
	if (count16 & 8)
	    tm1637_blink(25);
	else
	    tm1637_blink(0);
#endif
    } while(1);
}

/******************************************************************************
 *
 *  Board and globals setup
 */

void setup(void)
{
    CLK_CKDIVR = 0x00;	/* clock 16mhz */

    clock_tenths = 0;

    PB_DDR = 0x20;	/* output LED */
    PB_CR1 = 0xff;     	/* inputs have pullup */
    PB_CR2 = 0x00;	/* no interrupts, 2mhz output */

    __asm__ ("rim");
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
    tm1637_poll();
    clock_ms++;
}

/******************************************************************************
 *
 *  Tenths second timer callback
 */

void timer_10(void)
{
    clock_tenths++;
}


/******************************************************************************
 *
 *  Some words that a 7-segment module can display
 */

const char *words[] = {
    "ACID",
    "ACRE",
    "ACTS",
    "ALE ",
    "BALL",
    "BASH",
    "BELL",
    "BEER",
    "BOAT",
    "CAPS",
    "CHOP",
    "COLA",
    "DOLL",
    "HEAD",
    "HELL",
    "HELP",
    "HOPE",
    "JOB ",
    "LESS",
    "LOAD",
    "LOOP",
    "LOSS",
    "NOSE",
    "OPAL",
    "OPEN",
    "PALE",
    "PASS",
    "PEEP",
    "POLE",
    "PUFF",
    "SAFE",
    "SEED",
    "SOAP",
    "TEST",
    "USE ",
    0
};


