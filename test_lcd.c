/*
 *  File name:  test_lcd.c
 *  Date first: 12/31/2018
 *  Date last:  12/31/2018
 *
 *  Description: Test and example program for LCD library.
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
#include "lib_lcd.h"

void timer_ms(void);	/* millisecond timer call */
void timer_10(void);	/* 1/10 second timer call */

char clock_tenths;

void setup(void);

static void test_keys(void);
static void show_status(void);

#pragma disable_warning 196	/* "pointer lost const" */

static char module_type;

/******************************************************************************
 *
 *  Display things on LCD
 */

int main() {
    char	 dbuf[12];
    char	 clock_last, i;
    int		 count16;

    setup();
    clock_init(timer_ms, timer_10);
    lcd_init();

    count16 = 0;
    clock_last = 0;

    do {
	if (clock_last == clock_tenths)
	    continue;
	clock_last = clock_tenths;
	if (clock_last & 7)
	    continue;

	lcd_curs(0, 0);

	clock_string(dbuf);
	lcd_puts(dbuf);
	lcd_putc(' ');

	bin16_dec_rlz(count16, dbuf);
	lcd_puts(dbuf);

	lcd_puts(" 67890");

	lcd_curs(1, 0);
	for (i = 0; i < 20; i++)
	    lcd_putc((count16 & 31) + i + 'A');

	lcd_curs(2, 0);
	lcd_puts("Line #3..01234567890");
	lcd_curs(3, 0);
	lcd_puts("Line #4 !@#$%^&*()_-");

	count16++;
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
}

/******************************************************************************
 *
 *  Tenths second timer callback
 */

void timer_10(void)
{
    clock_tenths++;
}

