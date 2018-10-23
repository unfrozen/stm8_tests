/*
 *  File name:  test_tm1638.c
 *  Date first: 06/10/2018
 *  Date last:  06/24/2018
 *
 *  Description: Test and example program for TM1638 library.
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
#include "lib_tm1638.h"

void timer_ms(void);	/* millisecond timer call */
void timer_10(void);	/* 1/10 second timer call */

char clock_tenths;
char clock_last;

void setup(void);

const char *words[];

/* Choose one of the following options: */

//#define SHOW_NUMBER	/* show incrementing number */
//#define SHOW_CLOCK	/* show incrementing clock */
#define SHOW_WORDS	/* show selected words */
//#define SHOW_KEYS	/* read keys and echo to display */

/* Blink test may be combined with anything */
//#define TEST_BLINK	/* test blink function: 8 seconds on, 8 off */

static void test_keys(void);
static void show_status(void);

#pragma disable_warning 196	/* "pointer lost const" */

static char module_type;

/******************************************************************************
 *
 *  Display on LED device
 */

int main() {
    char	 decimal[12];
    char	 wptr;
    int		 count16;

    module_type = TM1638_8;	/* choose TM1638_8 or TM1638_16 */

    setup();
    tm1638_init(module_type);
    tm1638_bright(4);
    clock_init(timer_ms, timer_10);

//    tm1638_kmap("01234567");	/* load custom keyboard map */

    count16 = 0;
    wptr = 0;
    decimal;

    do {
	if (clock_last == clock_tenths)
	    continue;
	clock_last = clock_tenths;
#ifdef SHOW_KEYS
	test_keys();		/* handle keys every 1/10 second */
#endif
	if (clock_tenths > 1)
	    PB_ODR = 0x20;	/* LED off 8/10 second */
	if (clock_tenths < 10)
	    continue;
	clock_tenths -= 10;
	PB_ODR = 0;		/* LED on 2/10 second */

	count16++;
#ifdef SHOW_NUMBER
	bin16_dec(count16, decimal);
	tm1638_curs(3);
	tm1638_puts(decimal);
#endif
#ifdef SHOW_CLOCK
	clock_string(decimal);
	decimal[2] = ' ';
	decimal[5] = ' ';
	tm1638_curs(0);
	tm1638_puts(decimal);
#endif
#ifdef SHOW_WORDS
	tm1638_curs(0);
	tm1638_puts(words[wptr]);
	wptr++;
	if (!words[wptr])
	    wptr = 0;
#endif
#ifdef TEST_BLINK

	if (count16 & 7)
	    continue;
	if (count16 & 8)
	    tm1638_blink(25);
	else
	    tm1638_blink(0);
#endif
#ifndef SHOW_KEYS
	tm1638_setled(count16 & 7, (count16 >> 3) & 1);
#endif
	if (module_type == TM1638_16)
	    tm1638_push();
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
    tm1638_poll();
}

/******************************************************************************
 *
 *  Tenths second timer callback
 */

void timer_10(void)
{
    clock_tenths++;
}

/******************************************************************************
 *
 *  Display keys and indicate with LED
 *  (Note the Press/Release LED is based on the returned character.
 *   Loading a custom keymap will change the LED positions.)
 */

static void test_keys(void)
{
    char	key;

    key = tm1638_getc();
    if (!key)
	return;
    if (key & 0x80)		/* key release */
	tm1638_setled(key & 7, 0);
    else {			/* key press */
	tm1638_setled(key & 7, 1);
	tm1638_putc(key);
	if (module_type == TM1638_16)
	    tm1638_push();
    }
}

/******************************************************************************
 *
 *  Some words that a 7-segment module can display
 *  This is also a visual test of the decimal point.
 */

const char *words[] = {
#ifdef SHOW_WORDS
    "A.BASHED ",
    "AC.APULCO",
    "ACC.ESS  ",
    "ACCU.SED ",
    "ALCOH.OL ",
    "ALEHOU.SE",
    "ALFALFA. ",
    "APPEAL  .",
    "A.PPLAUSE",
    "BA.SEBALL",
    "BEC.AUSE ",
    "BELL.HOP ",
    "BLEAC.HED",
    "BUFFAL.O ",
    "CALCULU.S",
    "CAPSULE .",
    "C.ASCADE ",
    "CH.OPPED ",
    "CLO.UDED ",
    "COCA.COLA",
    "COFFE.E  ",
    "COLLOS.AL",
    "COUPLED. ",
    "CUEBALL .",
    "D.ECADE  ",
    "DO.LLFACE",
    "DOU.BLED ",
    "DUCH.ESS ",
    "ELAP.SED ",
    "ESCAP.ADE",
    "FABULO.US",
    "FLEECED. ",
    "FLOODED .",
    "F.OLDABLE",
    "HA.PLESS ",
    "HAS.SLE  ",
    "HEAD.ACHE",
    "HELLH.OLE",
    "HELPLE.SS",
    "HOPEFUL. ",
    "JABBED  .",
    "J.OBLESS ",
    "LA.UDABLE",
    "LEA.DOFF ",
    "LEEC.HES ",
    "LOADE.D  ",
    "LOOPHO.LE",
    "LOSSLES.S",
    "OCCLUDE .",
    "O.PALESCE",
    "OP.POSED ",
    "PAL.ACE  ",
    "PALE.FACE",
    "PALPA.BLE",
    "PASSAB.LE",
    "PEACEFU.L",
    "PEEPHOLE.",
    "P.LACEBO ",
    "PL.EASE  ",
    "POL.LED  ",
    "POOD.LE  ",
    "POPUL.ACE",
    "POSSES.S ",
    "PUDDLED. ",
    "PUFFBALL.",
    "S.ADDLED ",
    "SA.FEHOLD",
    "SCA.FFOLD",
    "SCAL.ABLE",
    "SCALP.EL ",
    "SCHOOL.ED",
    "SCOLDED. ",
    "SCOOPED .",
    "S.CUFFLED",
    "SE.ASCAPE",
    "SEC.LUDED",
    "SEED.LESS",
    "SELFL.ESS",
    "SHELLA.C ",
    "SHOELAC.E",
    "SHOULD  .",
    "S.HUFFLE ",
    "SL.ASHED ",
    "SLE.DDED ",
    "SLOP.ED  ",
    "SOAPS.UDS",
    "SOLUBL.E ",
    "SOUFFLE. ",
    "SPEECH  .",
    "S.PLASHED",
    "SP.OOFED ",
    "SPO.USE  ",
    "SUBC.LASS",
    "SUBDU.ED ",
    "SUBLEA.SE",
    "SUCCESS. ",
    "SUPPOSED.",
    "U.PHELD  ",
    "UP.SCALE ",
    "UPS.LOPE ",
    "USAB.LE  ",
    "USEFU.L  ",
    "USELES.S ",
    "USUAL  . ",
#endif
    0
};

