/*
 *  File name:  test_keypad.c
 *  Date first: 10/13/2018
 *  Date last:  10/14/2018
 *
 *  Description: Test and example program for keypad library
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

#include "lib_keypad.h"
#include "lib_uart.h"

void setup(void);

char clock_1ms;         /* milliseconds 0-255 */
char clock_ms;          /* milliseconds 0-99 */
char clock_10;          /* 1/10 second 0-255 */
char clock_tenths;      /* 1/10 second 0-9 */
char clock_secs;

/* Columns are D3, D2, D1, C7 */

const KP_PIN cfg_cols[] = {
    { &PD_ODR, 0x08},
    { &PD_ODR, 0x04},
    { &PD_ODR, 0x02},
    { &PC_ODR, 0x80},
    { (void *) 0, 0}
};

/* Rows are C6, C5, C4, C3 */

const KP_PIN cfg_rows[] = {
    { &PC_ODR, 0x40},
    { &PC_ODR, 0x20},
    { &PC_ODR, 0x10},
    { &PC_ODR, 0x08},
    { (void *) 0, 0}
};

/*  Keymap is Digits plus Cancel, Left, Enter, Up, Down, Right */
static char key_map[] = "147C2580369LEUDR";

/******************************************************************************
 *
 *  Test the keypad library
 */

int main() {
    char	last_tenth;
    char	key;

    setup();
    keypad_init(cfg_rows, cfg_cols);
    keypad_kmap(key_map);
    uart_init(BAUD_115200);

    last_tenth = 0;
    key = 0;

    do {
	if (last_tenth != clock_tenths) {
	    last_tenth = clock_tenths;
	}
	key = keypad_getc();
	if (!key)
	    continue;
	uart_puts("Got key :");
	uart_put(key & 0x7f);
	if (key & 0x80)
	    uart_puts(" (released)");
	uart_crlf();


    } while(1);
}

/******************************************************************************
 *
 *  Board and globals setup
 */

void setup(void)
{
    CLK_CKDIVR = 0x00;	/* clock 16mhz if STM8S103 */

    clock_1ms = 0;
    clock_ms = 0;
    clock_10 = 0;
    clock_tenths = 0;
    clock_secs   = 0;

    PB_DDR = 0x20;	/* output LED */
    PB_CR1 = 0xff;     	/* inputs have pullup */
    PB_CR2 = 0x00;	/* no interrupts, 2mhz output */


    TIM4_PSCR = 6;      /* prescaler = 64 for 16mhz */
    TIM4_ARR  = 249;	/* reset and interrupt every 1.0 ms */
    TIM4_CR1  = 1;	/* enable timer4 */
    TIM4_IER  = 1;	/* enable timer4 interrupt */

    __asm__ ("rim");
}
/* Available ports on STM8S103P3:
 *
 * A1..A3	A3 is HS
 * B4..B5	Open drain
 * C3..C7	HS
 * D1..D6	HS
 *
 * Note: B4 and B5 are "true open drain" and need a real pull-up resistor
 * to be used as keypad I/O.
 */

/******************************************************************************
 *
 *  Timer 4 interrupt
 */

void timer4_isr(void) __interrupt (IRQ_TIM4)
{
    TIM4_SR = 0;		/* clear the interrupt */

    /* Profiling with Timer4 gives 36 uSecs per keyboard poll.
     * If spending 3% of CPU cycles is too much, polling every 2 or 4
     * milliseconds will be fine.
     */
    keypad_poll();

    clock_1ms++;
    clock_ms++;
    if (clock_ms < 100)
	return;
    clock_ms -= 100;
    clock_10++;
    clock_tenths++;
    if (clock_tenths < 10)
	return;
    clock_tenths = 0;
    PB_ODR ^= 0x20;		/* toggle LED on board */
    clock_secs++;
}
