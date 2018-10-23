/*
 *  File name:  test_pwm.c
 *  Date first: 08/21/2018
 *  Date last:  08/22/2018
 *
 *  Description: Test and example program for PWM/servo library
 *
 *  Author:     Richard Hodges
 *
 *  Copyright (C) 2018 Richard Hodges. All rights reserved.
 *
 ******************************************************************************
 *
 */

#include "stm8s_header.h"

#include "lib_pwm.h"

char clock_1ms;		/* milliseconds 0-255 */
char clock_ms;		/* milliseconds 0-99 */
char clock_10;		/* 1/10 second 0-255 */
char clock_tenths;	/* 1/10 second 0-9 */
char clock_secs;

void setup(void);

//#define TEST_DUTY		/* sweep duty cycle over 20 seconds */
#define TEST_SERVO		/* sweep servo over 5 seconds */

/******************************************************************************
 *
 *  Test the PWM/servo library
 */

int main() {
    char	 clock_last;
    char	 pwm;
    
    setup();
#ifdef TEST_DUTY
    pwm_init(PWM_DUTY, PWM_C1 | PWM_C2);
#endif
#ifdef TEST_SERVO
    pwm_init(PWM_SERVO, PWM_C1 | PWM_C2);
#endif
    clock_last = 0;
    pwm = 0;

    do {
#ifdef TEST_DUTY
	if (clock_last == clock_10)	/* update every 1/10 second */
	    continue;
	clock_last = clock_10;
	pwm_duty(PWM_C1, pwm);
	pwm_duty(PWM_C2, 200 - pwm);
	pwm++;
	if (pwm == 201)
	    pwm = 0;
#endif
#ifdef TEST_SERVO
	if (clock_last != clock_1ms)
	    continue;
	clock_last += 20;
	pwm_servo(PWM_C1, pwm);
	pwm_servo(PWM_C2, 192 - pwm);
	pwm++;
#endif
    } while(1);
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

    PB_DDR = 0x20;	/* output LED */
    PB_CR1 = 0xff;     	/* inputs have pullup */
    PB_CR2 = 0x00;	/* no interrupts, 2mhz output */

#ifdef STM8103
    TIM4_PSCR = 6;      /* prescaler = 64 for 16mhz */
#endif
#ifdef STM8105
    TIM4_PSCR = 5;      /* prescaler = 32 for 8mhz */
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
 * pin  1 (D4) PWM channel 1
 * pin 20 (D3) PWM channel 2
 * pin 10 (A3) PWM channel 3
 *
 * Testing on STM8S105K4:
 *
 * pin 29 (D4) PWM channel 1
 * pin 28 (D3) PWM channel 2
 * pin 27 (D2) PWM channel 3
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
    if (clock_tenths < 10)
	return;
    clock_tenths = 0;
    PB_ODR ^= 0x20;		/* toggle LED on board */
    clock_secs++;
}
