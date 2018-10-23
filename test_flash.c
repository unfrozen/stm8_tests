/*
 *  File name:  test_flash.c
 *  Date first: 10/17/2018
 *  Date last:  10/19/2018
 *
 *  Description: Test and example program for Flash library
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
#include "lib_flash.h"
#include "lib_uart.h"

void setup(void);

char clock_1ms;         /* milliseconds 0-255 */
char clock_ms;          /* milliseconds 0-99 */
char clock_10;          /* 1/10 second 0-255 */
char clock_tenths;      /* 1/10 second 0-9 */
char clock_secs;


static void mem_write (char *, int, char);	/* write block */
static int  mem_verify(char *, int, char);	/* verify block */

const char primes[] = {
     3,  5,  7, 11, 13, 17, 19, 23, 29, 31,
     37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 0 };

#ifdef STM8105
#define LED_ODR	PE_ODR
#define LED_DDR	PE_DDR
#define LED_CR1	PE_CR1
#else
#define LED_ODR	PB_ODR
#define LED_DDR	PB_DDR
#define LED_CR1	PB_CR1
#endif

/******************************************************************************
 *
 *  Test the Flash library
 */

int main() {
    char	decimal[8];
    char	last_tenth;
    char	*ptr, iv;
    char	count, i;
    int		size, retval;

    setup();
    flash_init();
    uart_init(BAUD_115200);

    last_tenth = 0;
    i = 0;
    count = 0;

    uart_get();
    uart_puts("Memory test starting.\r\n");

    do {
	while (last_tenth == clock_tenths);
	last_tenth = clock_tenths;

	ptr = (char *)0x8800;
#ifdef STM8105
	size = 0x3800;
#else
	size = 0x1800;
#endif
	iv = 17;

	retval = flash_unlock();
	uart_puts("Unlocked. retval=");
	bin8_hex(retval, decimal);
	uart_puts(decimal);
	uart_crlf();

	uart_puts("Writing memory at ");
	bin8_hex((int)(ptr) >> 8, decimal);
	bin8_hex((char)(ptr) & 255, decimal+2);
	uart_puts(decimal);
	uart_puts(" IV=");
	bin8_dec2(iv, decimal);
	uart_puts(decimal);
	uart_crlf();

	mem_write(ptr, size, iv);

	uart_puts("Verifying memory...\r\n");
	retval = mem_verify(ptr, size, iv);
#ifdef UNDEF
	if (!retval)
	    continue;
#endif
	bin16_dec(retval, decimal);
	uart_puts(decimal_rlz(decimal, 4));
	uart_puts(" Memory errors\r\n");

	flash_clear(ptr + 0x0087, 0x100);
	flash_clear(ptr + 0x0200, 0x043);
	flash_clear(ptr + 0x0400, 0x155);
	flash_clear(ptr + 0x1234, 0x200);
#ifdef STM8105
	flash_clear(ptr + 0x3000, 0x100);
#endif
	while (1);

    } while(1);
}

/******************************************************************************
 *
 *  Verify memory
 *
 *  in: start, size, interval (zero for block of zeroes)
 */

static int mem_verify(char *ptr, int size, char iv)
{
    char	val;
    int		retval;

    val = 0;
    retval = 0;

    while (size--) {
	if (*ptr++ != val)
	    retval++;
	val += iv;
    }
    return retval;
}

/******************************************************************************
 *
 *  Write pseudo-random block to memory
 *
 *  in: start, size, interval (zero for block of zeroes)
 */

static void mem_write(char *ptr, int size, char iv)
{
    char	val;

    val = 0;

    flash_unlock();
    while (size--) {
	*ptr++ = val;
	val += iv;
    }
    flash_lock();
}

/******************************************************************************
 *
 *  Board and globals setup
 */

void setup(void)
{
    CLK_CKDIVR = 0x00;	/* clock 16mhz if STM8S103 */
#ifdef STM8105
    CLK_ECKR = 1;       /* enable crystal oscillator */
    CLK_SWCR = 2;       /* enable clock switch */
    CLK_SWR = 0xb4;     /* HSE is master (8 mhz crystal) */
#endif
    clock_1ms = 0;
    clock_ms = 0;
    clock_10 = 0;
    clock_tenths = 0;
    clock_secs   = 0;

    LED_DDR = 0x20;	/* LED on board */
    LED_CR1 = 0xff;	/* inputs have pullup, outputs not open drain */

#ifdef STM8105
    TIM4_PSCR = 5;      /* prescaler = 32 for 8mhz */
#else
    TIM4_PSCR = 6;      /* prescaler = 64 for 16mhz */
#endif
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

    clock_1ms++;
    clock_ms++;
    if (clock_ms < 100)
	return;
    clock_ms -= 100;
    clock_10++;
    clock_tenths++;
    if (clock_tenths > 1)
	LED_ODR = 0x20;
    if (clock_tenths < 10)
	return;
    LED_ODR = 0;

    clock_tenths = 0;
    clock_secs++;
}
