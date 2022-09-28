/*
 *  File name:  test_gpio_int.c
 *  Date first: 09/28/2022
 *  Date last:  09/28/2022
 *
 *  Description: Test and example program for STM8 GPIO interrupts.
 *
 *  Author:     Richard Hodges
 *
 *  Copyright (C) 2022 Richard Hodges. All rights reserved.
 *  Permission is hereby granted for any use.
 *
 ******************************************************************************
 *
 *  Test the GPIO interrupt functions
 *
 *  This test will set pin C4 as an input with interrupt on low level.
 *  C4 will be set up as input with weak pull-up and interrupt.
 *  C3 will be an output to trigger C4.
 *  Connect C3 and C4 with a jumper.
 *
 *  A3 will be an output that will be high during the ISR.
 *  The millisecond callback will lower C4 to trigger the interrupt.
 *  The ISR will raise C4 to clear the interrupt.
 *
 *  This code was tested with all four interrupt modes:
 *  00: Interrupt on falling edge and low level.
 *  01: Interrupt on rising edge only.
 *  10: Interrupt on falling edge only.
 *  11: Interrupt on rising and falling edge.
 */

#include "stm8s_header.h"

#include "lib_board.h"
#include "lib_clock.h"

void timer_ms(void);	/* millisecond timer call */
void timer_10(void);	/* 1/10 second timer call */

volatile unsigned int clock_tenths;
volatile unsigned char flag_trigger;	/* Time to trigger interrupt. */

void local_setup(void);

/******************************************************************************
 *
 *  Test the GPIO interrupt.
 */

int main() {
    int		clock_last;
    
    board_init(0);
    local_setup();
    clock_init(timer_ms, timer_10);

    flag_trigger = 0;
    clock_last = clock_tenths;
    for (;;) {
	if (flag_trigger) {
	    flag_trigger = 0;	/* Lower C3 to trigger interrupt. */
	    PC_ODR &= 0xf7;
	}
	if (clock_last == clock_tenths)
	    continue;
	clock_last = clock_tenths;

	/* Put 1/10 second code here if desired. */
    }
}

/******************************************************************************
 *
 *  Board and globals setup
 */

void local_setup(void)
{
    clock_tenths = 0;

    /* The EXTI_CR1 register defaults to what we want, so this is not
     * needed, but included for completeness.
     */
    __asm__ ("sim");		/* I0 & I1 must be 1 in CCR register. */
    EXTI_CR1 = 0x00;		/* Interrupt on falling edge and low level. */
    __asm__ ("rim");
    
    PA_DDR |= 0x08;		/* A3 is output, others are inputs. */
    PA_CR1 |= 0xff;		/* A3 is push-pull, inputs have pull-up. */
    PA_CR2 |= 0x08;		/* A3 is high speed. */
    
    PC_DDR |= 0x08;		/* C3 is output, others are inputs. */
    PC_ODR  = 0x08;		/* C3 is normally high. */
    PC_CR1 |= 0xff;		/* C3 is push-pull, inputs have pull-up. */
    PC_CR2 |= 0x18;		/* C3 is high speed, C4 has interrupt. */
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
    flag_trigger = 1;		/* Time to trigger interrupt. */
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

/******************************************************************************
 *
 *  Interrupt service routine for C4.
 */

void gpioc_isr(void) __interrupt (IRQ_EXTI2)
{
    PA_ODR |= 0x08;		/* Raise A3 to show on oscilloscope. */
    PC_ODR |= 0x08;		/* Raise C3 to clear interrupt. */
    PA_ODR &= 0xf7;		/* Lower A3 to show end of ISR. */
}
