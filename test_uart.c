/*
 *  File name:  test_uart.c
 *  Date first: 09/17/2022
 *  Date last:  09/18/2022
 *
 *  Description: Test and example program for STM8 UART library.
 *
 *  Author:     Richard Hodges
 *
 *  Copyright (C) 2022 Richard Hodges. All rights reserved.
 *  Permission is hereby granted for any use.
 *
 ******************************************************************************
 *
 *  Test the UART functions
 *
 *  TX is pin D5
 *  RX is pin d6
 *
 *  When byte is received, print it as ASCII and hex, except
 *  backspace (0x08) prints a long message.
 *  Otherwise, print the clock uptime every second.
 */

#include "stm8s_header.h"

#include "lib_bindec.h"
#include "lib_board.h"
#include "lib_clock.h"
#include "lib_uart.h"

void timer_ms(void);	/* millisecond timer call */
void timer_10(void);	/* 1/10 second timer call */

volatile unsigned int clock_tenths;

void local_setup(void);

void handle_byte(void);
void show_clock(void);

const char *mesg;

#define KEY_FOR_MESSAGE 8	/* Backspace to print message. */

/******************************************************************************
 *
 *  Test the UART library.
 */

int main() {
    int		clock_last, diff;
    
    board_init(0);
    local_setup();
    clock_init(timer_ms, timer_10);
    uart_init(BAUD_115200);

    uart_puts("UART test. Press BACKSPACE for message.\r\n"
	      "Other keys echo back with hex value.\r\n");
    clock_last = clock_tenths;
    for (;;) {
	if (uart_rsize())
	    handle_byte();
	diff = clock_tenths - clock_last;
	if (diff < 10)
	    continue;
	clock_last = clock_tenths;
	show_clock();
    }
}

/******************************************************************************
 *
 *  Handle byte input.
 */

void handle_byte(void)
{
    char	c, hex[3];
    
    while (uart_rsize()) {
	c = uart_get();
	if (c == KEY_FOR_MESSAGE) {
	    uart_puts((char *)mesg);
	    continue;
	}
	uart_puts("Byte received '");
	uart_put(c);
	uart_puts("' Hex value: ");
	bin8_hex(c, hex);
	uart_puts(hex);
	uart_crlf();
    }
}

/******************************************************************************
 *
 *  Show clock.
 */

void show_clock(void)
{
    char	clock[9];

    clock_string(clock);
    uart_puts(clock);
    uart_crlf();
}

/******************************************************************************
 *
 *  Board and globals setup
 */

void local_setup(void)
{
    clock_tenths = 0;
#ifdef EXAMPLE
    PORT_DDR |= PORT_MASK;	/* Pin is output. */
    PORT_CR1 |= PORT_MASK;	/* Pin is push/pull. */
    PORT_CR2 |= PORT_MASK;	/* Pin is fast mode (10mhz). */
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
 *  Long message
 */

const char *mesg =
    "\r\n"
    "If you ever want to have a lot of fun, I recommend that you go off and\r\n"
    "program an embedded system.  The salient characteristic of an embedded\r\n"
    "system is that it cannot be allowed to get into a state from which\r\n"
    "only direct intervention will suffice to remove it.  An embedded \r\n"
    "can't permanently trust anything it hears from the outside world.\r\n"
    "It must sniff around, adapt, consider, sniff around, and adapt again.\r\n"
    "I'm not talking about ordinary modular programming carefulness here.\r\n"
    "No.  Programming an embedded system calls for undiluted raging \r\n"
    "maniacal paranoia.  For example, our ethernet front ends need to know\r\n"
    "what network number they are on so that they can address and route \r\n"
    "PUPs properly.  How do you find out what your network number is?  \r\n"
    "Easy, you ask a gateway.  Gateways are required by definition to\r\n"
    "know their correct network numbers.  Once you've got your network \r\n"
    "number, you start using it and before you can blink you've got it \r\n"
    "wired into fifteen different sockets spread all over creation.  \r\n"
    "Now what happens when the panic-stricken operator realizes he was\r\n"
    "running the wrong version of the gateway which was giving out the \r\n"
    "wrong network number?  Never supposed to happen.  Tough.  Supposing\r\n"
    "that your software discovers that the gateway is now giving out a \r\n"
    "different network number than before, what's it supposed to do about \r\n"
    "it?  This is not discussed in the protocol document.  Never supposed\r\n"
    "to happen.  Tough.  I think you get my drift.\r\n";
