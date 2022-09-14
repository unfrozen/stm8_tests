/*
 *  File name:  test_bindec.c
 *  Date first: 06/10/2018
 *  Date last:  09/14/2022
 *
 *  Description: Test and example program for bindec library.
 *
 *  Author:     Richard Hodges
 *
 *  Copyright (C) 2018, 2022 Richard Hodges. All rights reserved.
 *  Permission is hereby granted for any use.
 *
 *  Code copied and modified from test_tm1638.c
 *  Uses tm1638 display to show results.
 *
 ******************************************************************************
 *
 * Display: T..99999
 * With test number on left, progress count on right.
 * Error causes halt and display flashing.
 * Successful test ends with "PASS" on right.
 *
 * 1: bin16_dec (full test)
 *    dec_bin16 (tested against each other)
 * 2: bin8_dec2 (full test)
 * 3: bin8_hex  (full test)
 * 4: bin32_dec (test low 16 bits)
 * 5: bin32_dec (test with prime pattern, verify low 16 bits)
 */

#include "stm8s_header.h"

#include "lib_bindec.h"
#include "lib_board.h"
#include "lib_clock.h"
#include "lib_tm1638.h"

void timer_ms(void);	/* millisecond timer call */
void timer_10(void);	/* 1/10 second timer call */

char clock_tenths;
char clock_last;

void local_setup(void);

void test_1(void);	/* Test bin16_dec and dec_bin16. */
void test_2(void);	/* Test bin8_dec2. */
void test_3(void);	/* Test bin8_hex. */
void test_4(void);	/* Test bin32_dec, low 16 bits. */
void test_5(int);	/* Test bin32_dec, prime pattern, verify low 16 bits */

void test_number(char);	/* Display test number. */
void test_fail(void);	/* Flash display and stop. */

char hex_bin8(char *);	/* 2 char hex to binary */

//#define FORCE_FAIL	/* Force failure for testing. */

#define PRIME_PATTERN 13

static char module_type;

#define disp_blink tm1638_blink
#define disp_clear tm1638_clear
#define disp_curs  tm1638_curs
#define disp_putc  tm1638_putc
#define disp_puts  tm1638_puts

/******************************************************************************
 *
 *  Test the lib_bindec library and show results.
 */

int main() {
    module_type = TM1638_8;	/* choose TM1638_8 or TM1638_16 */

    board_init(0);
    local_setup();
    tm1638_init(module_type);
    tm1638_bright(4);
    clock_init(timer_ms, timer_10);

    test_1();	/* Test bin16_dec and dec_bin16. */
    test_2();	/* Test bin8_dec2. */
    test_3();	/* Test bin8_hex. */
#ifdef FORCE_FAIL
    test_fail();
#endif
    test_4();	/* Test bin32_dec, low 16 bits. */
    test_5(PRIME_PATTERN); /* Test bin32_dec with pattern, verify low 16 bits */

    disp_clear();
    disp_curs(4);
    disp_puts("PASS");
    for (;;);
}

/******************************************************************************
 *
 *  Test bin16_dec and dec_bin16.
 */

void test_1(void)
{
    int		count, retval;
    char	decimal[6];
    
    test_number(1);

    count = 0;
    for (;;) {
	bin16_dec(count, decimal);
	disp_curs(3);
	disp_puts(decimal);
	retval = dec_bin16(decimal);
	if (retval != count)
	    test_fail();
	count++;
	if (count == 0)
	    break;
    }
}

/******************************************************************************
 *
 *  Test bin8_dec2.
 */

void test_2(void)
{
    int		count, retval;
    char	decimal[3];

    test_number(2);

    count = 0;
    for (;;) {
	bin8_dec2(count, decimal);
	disp_curs(6);
	disp_puts(decimal);
	retval = dec_bin16(decimal);
	if (retval != count) {
	    disp_curs(2);
	    bin8_dec2(retval, decimal);
	    disp_puts(decimal);
	    test_fail();
	}
	count++;
	if (count == 100)
	    break;
    }
}

/******************************************************************************
 *
 *  Test bin8_hex.
 */

void test_3(void)
{
    int		count, retval;
    char	hex[3];

    test_number(3);

    count = 0;
    for (;;) {
	bin8_hex(count, hex);
	disp_curs(6);
	disp_puts(hex);
	retval = hex_bin8(hex);
	if (retval != count) {
	    disp_curs(2);
	    bin8_hex(retval, hex);
	    disp_puts(hex);
	    test_fail();
	}
	count++;
	if (count == 256)
	    break;
    }

}
/******************************************************************************
 *
 *  Test bin32_dec, low 16 bits.
 */

void test_4(void)
{
    unsigned long count;
    unsigned int retval;
    char	decimal[11];
    
    test_number(4);

    count = 0;
    for (;;) {
	bin32_dec(count, decimal);
	disp_curs(2);
	disp_puts(decimal + 4);
	retval = dec_bin16(decimal);
	if (retval != count)
	    test_fail();
	count++;
	if (count == 0x10000)
	    break;
    }

}

/******************************************************************************
 *
 *  Test bin32_dec with pattern, verify low 16 bits.
 */

void test_5(int pattern)
{
    unsigned long count;
    unsigned int retval;
    char	decimal[11];
    
    test_number(5);

    count = 0;
    for (;;) {
	bin32_dec(count, decimal);
	disp_curs(2);
	disp_puts(decimal + 4);
	retval = dec_bin16(decimal);
	if (retval != (count & 0xffff))
	    test_fail();
	count += pattern;
	if (count > 0x100000)
	    break;
    }
}

/******************************************************************************
 *
 *  Clear display and show test number.
 */

void test_number(char tnum)
{
    tnum += '0';	/* Workaround for SDCC 4.2 issue. */
    disp_clear();
    disp_curs(0);
    disp_putc(tnum);
}

/******************************************************************************
 *
 *  Flash display and stop.
 */

void test_fail(void)
{
    disp_blink(25);	/* Rate is 25/100 second. */
    for (;;);
}

/******************************************************************************
 *
 *  Convert 2 char hex to binary
 */

char hex_digit(char hex)
{
    if (hex < '9' + 1)
	return hex - '0';
    if (hex > 'F')
	hex -= 'a' - 'A';
    return hex + 10 - 'A';
}
char hex_bin8(char *hex)
{
    return hex_digit(hex[1]) + (hex_digit(hex[0]) << 4);
}

/******************************************************************************
 *
 *  Board and globals setup
 */

void local_setup(void)
{
    clock_tenths = 0;
#ifdef UNDEF
    PB_DDR = 0x20;	/* output LED */
    PB_CR1 = 0xff;     	/* inputs have pullup */
    PB_CR2 = 0x00;	/* no interrupts, 2mhz output */
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
    tm1638_poll();
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
