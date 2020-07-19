/*
 *  File name:  test_spi.c
 *  Date first: 06/08/2020
 *  Date last:  07/08/2020
 *
 *  Description: Test/Example for STM8 SPI Library.
 *
 *  Author:     Richard Hodges
 *
 *  Copyright (C) 2020 Richard Hodges. All rights reserved.
 *  Permission is hereby granted for any use.
 *
 *  Location:
 *  https://github.com/unfrozen/stm8_tests
 *
 ******************************************************************************
 *
 */

#include "stm8s_header.h"

#include "lib_bindec.h"
#include "lib_board.h"
#include "lib_clock.h"
#include "lib_delay.h"
#include "lib_spi.h"
#include "lib_uart.h"

volatile char	clock_tenths;	/* 1/10 second counter 0-255 */
volatile char   clock_msecs;	/* millisecond counter */

/* callbacks provided by lib_clock */

void clock_ms(void);	/* millisecond callback */
void clock_10(void);	/* 1/10 second callback */

void local_setup(void); /* setup for this project */

void dump_hex(char *, char);	/* dump buffer as hex */

void tm1638_command(SPI_CTX *); /* Send TM1638 command. */
void tm1638_init(SPI_CTX *);	/* Set up TM1638. */

/*
 *  Chose the test to run.
 *  For the RX tests, connect MOSI (C6) to MISO (C7) and use a USB-TTL
 *  serial converter to get the results over serial.
 */
#define TEST_TX		/* Test TX only transaction. */
//#define TEST_RX	/* Test RX only transaction. */
#define TEST_TXRX	/* Test combined TX+RX transaction. */
//#define TEST_TM1638	/* Text bidirectional mode with LED/KEYPAD device. */

/*
 *  Choose whether you want an SPI transaction every millisecond (better
 *  for viewing on oscilloscope, or every tenth second (better for watching
 *  UART output on terminal.)
 */
//#define TENTHS
#define MILLISECONDS

/******************************************************************************
 *
 *  Test the SPI library
 *  Send test patterns to watch on the oscilloscope.
 *  Connect MOSI to MISO for loopback verification.
 */

int main() {
    SPI_CTX	ctx1, ctx2;	/* Two different configurations. */
    char	spi_tx1[10];
    char	spi_rx1[10];
    char	spi_tx2[20];
    char	spi_rx2[10];
    char	last_msec, last_tenth;

    board_init(0);
    local_setup();
    clock_init(clock_ms, clock_10);
    uart_init(BAUD_115200);

    ctx1.tx_buf = spi_tx1;
    ctx1.rx_buf = spi_rx1;
    ctx2.tx_buf = spi_tx2;
    ctx2.rx_buf = spi_rx2;
    /*
     * The first SPI configuration is big-endian with a slow clock speed
     * to make it easy to verify on an oscilloscope or protocol analyzer.
     * The tests with RX should have a jumper between MOSI and MISO so
     * you can verify the data using the UART and a terminal program.
     */
    ctx1.config =
	SPI_MSB_FIRST |		/* Is data little- or big- endian? */
	SPI_250K   |		/* SPI clock speed (start slow). */
	SPI_IDLE_1 |		/* MOSI pin when SPI is idle */
	SPI_EDGE_2;		/* Does data start at 1st or 2nd clock edge? */
    ctx1.flag_bidir = 0;	/* Do RX and TX share the same pin? */
    /*
     * The second SPI configuration is for a display and keypad device that
     * has a TM1638 driver. It is little-endian and can handle an SPI clock
     * just over 1mhz. Its single data pin is bidirectional.
     */
    ctx2.config =
	SPI_LSB_FIRST |		/* TM1638 is big-endian. */
	SPI_1MHZ   |		/* Minimum clock pulse is 400ns. */
	SPI_IDLE_1 |		/* MOSI pin when SPI is idle */
	SPI_EDGE_2;		/* Does data start at 1st or 2nd clock edge? */
    ctx2.flag_bidir = 1;	/* Data pin is bidirectional. */

    spi_init(&ctx1);
    spi_init(&ctx2);
    
    last_msec = 0;
    last_tenth = 0;
#ifdef TEST_TM1638
    tm1638_init(&ctx2);
#endif
    do {
#ifdef MILLISECONDS
	if (last_msec == clock_msecs)
	    continue;		/* wait for next millisecond */
	last_msec = clock_msecs;
#endif
#ifdef TENTHS
	if (last_tenth == clock_tenths)
	    continue;
	last_tenth = clock_tenths;
#endif	
	PA_ODR |= 0x02;		/* A1 is scope trigger and start marker. */
#ifdef TEST_TX
	spi_tx1[0] = 0xf0;
	spi_tx1[1] = 0xaa;
	spi_tx1[2] = 0x00;
	spi_tx1[3] = 0xff;
	ctx1.tx_count = 4;
	ctx1.rx_count = 0;
	spi_start(&ctx1);	/* This transaction will start immediately. */

	ctx1.tx_buf += 4;
	spi_tx1[4] = 0xde;
	spi_tx1[5] = 0xad;
	spi_tx1[6] = 0xbe;
	spi_tx1[7] = 0xef;
	spi_start(&ctx1);	/* This call will wait for the first. */
	ctx1.tx_buf -= 4;
#endif /* TEST_TX */
#ifdef TEST_RX
	ctx1.tx_count = 0;
	ctx1.rx_count = 5;
	spi_start(&ctx1);
#endif /* TEST_RX */
#ifdef TEST_TXRX
	spi_tx1[0] = 0xa5;
	spi_tx1[1] = 0xbb;
	spi_tx1[2] = 0x69;
	spi_tx1[3] = 0xff;
	ctx1.tx_count = 4;
	ctx1.rx_count = 8;	/* Expect to get a5 bb 69 ff xx xx xx xx */
	spi_start(&ctx1);
#endif /* TEST_TXRX */
#ifdef TEST_TM1638
	spi_wait();		/* Wait for previous TX/RX to finish. */
	spi_config(&ctx2);	/* Reconfigure SPI before the *enable. */
	
	PA_ODR &= 0xf7;		/* Assert *enable. */
	delay_500ns();
	spi_tx2[0] = 0x42;	/* Command to read keypad. */
	ctx2.tx_count = 1;
	ctx2.rx_count = 0;
	spi_start(&ctx2);

	spi_wait();
	delay_500ns();		/* Need at least 1 usec before read. */
	delay_500ns();
	
	ctx2.tx_count = 0;
	ctx2.rx_count = 4;	/* Keys encoded into 4 bytes. */
	spi_start(&ctx2);
	while(!(ctx2.flag_done));
	spi_wait();		/* Wait for data. */
	
	PA_ODR |= 0x08;		/* Disable. */
#endif /* TEST_TM1638 */
	spi_wait();		/* Wait for transaction to finish. */
	PA_ODR &= 0xfd;

	if (ctx1.rx_count) {
	    uart_puts("RX CTX1:\r\n");
	    dump_hex(ctx1.rx_buf, ctx1.rx_count);
	}
	if (ctx2.rx_count) {
	    uart_puts("RX CTX2:\r\n");
	    dump_hex(ctx2.rx_buf, ctx2.rx_count);
	}
    } while(1);
}

/******************************************************************************
 *
 *  Dump buffer as hex
 *  in: buffer, size
 */

void dump_hex(char *buf, char size)
{
    char	hex[3];
    char	pos;

    pos = 0;
    while (size--) {
	if (!(pos & 15)) {
	    bin8_hex(pos, hex);
	    uart_puts(hex);
	    uart_puts(": ");
	}
	bin8_hex(*buf++, hex);
	uart_puts(hex);
	uart_put(' ');
	pos++;
	if (!(pos & 15))
	    uart_crlf();
    }
    if (pos & 15)
	uart_crlf();
}

/******************************************************************************
 *
 *  Set up TM1638
 *  in: SPI context
 */

void tm1638_init(SPI_CTX *ctx)
{
    char	*spi_tx;

    spi_tx = ctx->tx_buf;
    
	spi_tx[0] = 0x8f;	/* Enable display, maximum brightness. */
	ctx->tx_count = 1;
	ctx->rx_count = 0;
	tm1638_command(ctx);
	
	spi_tx[0] = 0x40;	/* Data write, incrementing. */
	tm1638_command(ctx);
	
	spi_tx[0] = 0xc0;	/* Start at address zero. */
	spi_tx[1] = 0x3f;	/* '0' */
	spi_tx[3] = 0x06;
	spi_tx[5] = 0x5b;
	spi_tx[7] = 0x4f;	/* '3' */
	spi_tx[9] = 0x66;
	spi_tx[11] = 0x6d;
	spi_tx[13] = 0x7d;
	spi_tx[15] = 0x07;
	ctx->tx_count = 16;
	tm1638_command(ctx);	/* Write '01234567' */
}

/******************************************************************************
 *
 *  Send TM1638 command
 *  in: SPI context with TX data
 */

void tm1638_command(SPI_CTX *ctx)
{
    spi_wait();
    spi_config(ctx);

    PA_ODR &= 0xf7;		/* Strobe (A3) is active low. */
    delay_500ns();
    spi_start(ctx);
    while (!ctx->flag_done);
    spi_wait();
    delay_500ns();
    PA_ODR |= 0x08;
}

/******************************************************************************
 *
 *  Millisecond callback from lib_clock
 */

void clock_ms(void)
{
    clock_msecs++;
}

/******************************************************************************
 *
 *  1/10 second callback from lib_clock
 */

void clock_10(void)
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
 * I/O Ports on STM8S103F:
 *
 * A1..A3       A3 is HS
 * B4..B5       Open drain
 * C3..C7       HS
 * D1..D6       HS
 *
 * Note: B4 and B5 are "true open drain" and need a real pull-up resistor
 * to be used as output.
 *
 * SPI PINS for STM8S103F and STM8S003F:
 *
 * Clock:  C5 (pin 15)  Master output
 * MOSI:   C6 (pin 16)  Master output, slave input
 * MISO:   C7 (pin 17)  Master input, slave output
 * /CS:    Choose available output pin.
 *
 * If bidirectional mode is used (ctx->flag_bidir), both TX and RX
 * will be on MOSI (C6, pin 16).
 *
 * UART PINS (verifying receive).
 *
 * TX: D5 (pin 2)
 * RX: D6 (pin 3)
 *
 * Testing with TM1638:
 * CLK: C5
 * DIO: C6  Bidirectional I/O
 * STB: A3  Strobe goes low for transaction, then returns high.
 *
 ******************************************************************************
 *
 *  Local setup
 */

void local_setup(void)
{
    /* We will use A1 as a general purpose oscilloscope signal and
     * as a trigger for the signal analyzer. A2 will be used to
     * watch interrupt routine usage. A3 is the strobe signal for
     * testing with TM1638 LED+keypad.
     */
    PA_DDR |= 0x0e;		/* Using A1 as oscilloscope signal, A2. */
    PA_CR1 |= 0x0e;		/* Push-pull output. */

    PA_ODR = 8;			/* Strobe is active low. */
}
