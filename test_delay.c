/*
 *  File name:  test_delay.c
 *  Date first: 09/16/2022
 *  Date last:  09/17/2022
 *
 *  Description: Test and example program for delay library.
 *
 *  Author:     Richard Hodges
 *
 *  Copyright (C) 2022 Richard Hodges. All rights reserved.
 *  Permission is hereby granted for any use.
 *
 ******************************************************************************
 *
 *  Test the delay functions:
 *
 *  delay_500ns()
 *  delay_50us()
 *  delay_usecs()
 *  delay_ms()
 */

#include "stm8s_header.h"

#include "lib_board.h"
#include "lib_clock.h"
#include "lib_delay.h"

void timer_ms(void);	/* millisecond timer call */
void timer_10(void);	/* 1/10 second timer call */

volatile unsigned int clock_tenths;

void local_setup(void);

void test_1(void);	/* Test delay_500ns() */
void test_2(void);	/* Test delay_50us() */
void test_3(void);	/* Test delay_usecs() */
void test_4(void);	/* Test delay_ms() */

#define TEST_LENGTH 50	/* Duration of each test, in 1/10 second. */

/* Using pin A3 (high speed) for output to scope. */

#define PORT_DDR	PA_DDR
#define PORT_ODR	PA_ODR
#define PORT_CR1	PA_CR1
#define PORT_CR2	PA_CR2
#define PORT_PIN	3
#define PORT_MASK	(1 << PORT_PIN)

/******************************************************************************
 *
 *  Test the delay library and verify with oscilloscope.
 */

int main() {

    board_init(0);
    local_setup();
    clock_init(timer_ms, timer_10);

    /* Choose one test to run at a time. */
    
    for (;;) {
	//test_1();	/* Test delay_500ns() */
	//test_2();	/* Test delay_50us() */
	//test_3();	/* Test delay_usecs() */
	test_4();	/* Test delay_ms() */
    }
}

/******************************************************************************
 *
 *  Test delay_500ns()
 */

void test_1(void)
{
    for(;;) {
	PORT_ODR ^= PORT_MASK;
	delay_500ns();
	PORT_ODR ^= PORT_MASK;
	delay_500ns();
	PORT_ODR ^= PORT_MASK;
	delay_500ns();
	PORT_ODR ^= PORT_MASK;
	delay_500ns();
	PORT_ODR ^= PORT_MASK;
	delay_500ns();
    }
}

/******************************************************************************
 *
 *  Test delay_50us()
 */

void test_2(void)
{
    for (;;) {
	PORT_ODR ^= PORT_MASK;
	delay_50us();
	PORT_ODR ^= PORT_MASK;
	delay_50us();
	PORT_ODR ^= PORT_MASK;
	delay_50us();
	PORT_ODR ^= PORT_MASK;
	delay_50us();
	PORT_ODR ^= PORT_MASK;
	delay_50us();
    }
}

/******************************************************************************
 *
 *  Test delay_usecs()
 */

void test_3(void)
{
    for (;;) {
	PORT_ODR ^= PORT_MASK;
	delay_usecs(100);
	PORT_ODR ^= PORT_MASK;
	delay_usecs(100);
	PORT_ODR ^= PORT_MASK;
	delay_usecs(100);
	PORT_ODR ^= PORT_MASK;
	delay_usecs(100);
	PORT_ODR ^= PORT_MASK;
	delay_usecs(100);
    }
}
/******************************************************************************
 *
 *  Test delay_ms()
 */

void test_4(void)
{
    for (;;) {
	PORT_ODR ^= PORT_MASK;
	delay_ms(10);
    }
}

/******************************************************************************
 *
 *  Board and globals setup
 */

void local_setup(void)
{
    clock_tenths = 0;

    PORT_DDR |= PORT_MASK;	/* Pin is output. */
    PORT_CR1 |= PORT_MASK;	/* Pin is push/pull. */
    PORT_CR2 |= PORT_MASK;	/* Pin is fast mode (10mhz). */
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
