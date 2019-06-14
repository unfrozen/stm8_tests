/*
 *  File name:  test_w1209.c
 *  Date first: 02/22/2019
 *  Date last:  06/14/2019
 *
 *  Description: Test/Example for STM8 Library for W1209 thermostat board.
 *
 *  Author:     Richard Hodges
 *
 *  Copyright (C) 2019 Richard Hodges. All rights reserved.
 *  Permission is hereby granted for any use.
 *
 ******************************************************************************
 *
 */

#include "stm8s_header.h"

#include "lib_bindec.h"
#include "lib_clock.h"
#include "lib_w1209.h"

volatile char	clock_tenths;	/* 1/10 second counter 0-255 */
volatile char   clock_msecs;	/* millisecond counter */

/* callbacks provided by lib_clock */

void clock_ms(void);	/* millisecond callback */
void clock_10(void);	/* 1/10 second callback */

void do_key(char);	/* handle keypress */

const char *words[];	/* words that can be shown on LED */

/*  Choose one of these display options  */

//#define SHOW_NUMBER	/* show incrementing number */
//#define SHOW_WORDS	/* show selected words */
#define SHOW_PROBE	/* show raw probe value */

/******************************************************************************
 *
 *  Do things with the W1209 board
 *
 *  Show second counter or words.
 *  Show key press/release in hex.
 *  Engage relay and LED while any key is pressed.
 *  Button #1 ends blink, #2 starts fast blink, #3 starts slow blink
 */

int main() {
    char	display[6];
    char	next_tenth;
    int		secs;
    char	key, wptr;

    w12_init();
    clock_init(clock_ms, clock_10);

    display;			/* suppress warning */
    next_tenth = 10;
    secs = 0;
    wptr = 0;

    w12_relay(1);		/* turn on LED to show reset */
    w12_puts("[-]");

    do {
	key = w12_getc();
	if (key)
	    do_key(key);

	if (next_tenth != clock_tenths)
	    continue;		/* wait for next whole second */
	next_tenth = clock_tenths + 10;
	secs++;
	w12_curs(0);
#ifdef SHOW_NUMBER
	bin16_dec(secs, display);
	w12_puts(display + 2);
#endif
#ifdef SHOW_WORDS
	w12_puts(words[wptr]);
	wptr++;
	if (!words[wptr])
	    wptr = 0;
#endif
#ifdef SHOW_PROBE
	bin16_dec(w12_probe(), display);
	w12_puts(display + 2);
#endif
    } while(1);
}

/******************************************************************************
 *
 *  Do something with keypress (or release)
 *  in: key (bit-7 set for release)
 */

void do_key(char key)
{
    char	display[6];

    if (key & 0x80)
	w12_relay(0);	/* key release turns off relay */
    else
	w12_relay(1);	/* key press turns on relay */

    switch (key) {
    case '0' : w12_blink(0);  break;
    case '1' : w12_blink(10); break;
    case '2' : w12_blink(25); break;
    }
    bin8_hex(key, display);
    w12_curs(0);
    w12_puts(display);		/* release will have bit-7 set */
    w12_putc('-');
}

/******************************************************************************
 *
 *  Millisecond callback from lib_clock
 */

void clock_ms(void)
{
    clock_msecs++;
    w12_poll();
}

/******************************************************************************
 *
 *  1/10 second callback from lib_clock
 */

void clock_10(void)
{
    clock_tenths++;
}

/******************************************************************************
 *
 *  Words that can be shown
 */

const char *words[] = {
    "ACE",
    "ACT",
    "ADD",
    "AID",
    "AIL",
    "AIR",
    "ALE",
    "ALL",
    "ALP",
    "APE",
    "ASH",
    "ASP",
    "ASS",
    "BAD",
    "BAT",
    "BED",
    "BEE",
    "BIB",
    "BID",
    "BIT",
    "BOA",
    "BOB",
    "BOT",
    "BUD",
    "BUS",
    "CAP",
    "CAR",
    "CHI",
    "COD",
    "COP",
    "CUB",
    "CUE",
    "CUP",
    "DAB",
    "DAD",
    "DID",
    "DIE",
    "DIP",
    "DOC",
    "DOT",
    "DOE",
    "DUE",
    "EBB",
    "ELF",
    "FAD",
    "FAN",
    "FED",
    "FEE",
    "HAD",
    "HEN",
    "HIP",
    "HIS",
    "HOP",
    "HOT",
    "HUB",
    "HUE",
    "ICE",
    "ILL",
    "ISO",
    "JAB",
    "JOB",
    "LAB",
    "LAP",
    "LED",
    "LID",
    "LIE",
    "LOB",
    "LOT",
    "ODD",
    "ODE",
    "OFF",
    "OIL",
    "OLD",
    "OPT",
    "OUT",
    "PAD",
    "PAL",
    "PEN",
    "PEP",
    "PET",
    "PHI",
    "POD",
    "POP",
    "PUT",
    "PSI",
    "PUB",
    "SAD",
    "SAP",
    "SEA",
    "SEE",
    "SHE",
    "SIP",
    "SOU",
    "SPA",
    "SUB",
    "SUE",
    "SUP",
    "TAB",
    "TAN",
    "TEA",
    "TIE",
    "TOP",
    "TUB",
    "USE",
    0
};

