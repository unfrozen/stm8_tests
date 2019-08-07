/*
 *  File name:  test_max7219.c
 *  Date first: 03/15/2018
 *  Date last:  08/07/2019
 *
 *  Description: Test and example program MAX7219 LED controller.
 *
 *  Author:     Richard Hodges
 *
 *  Copyright (C) 2018 Richard Hodges. All rights reserved.
 *  Permission is hereby granted for any use.
 *
 ******************************************************************************
 *
 */

#include "stm8_103.h"
#include "vectors.h"

#include "lib_bindec.h"
#include "lib_max7219.h"

/* Choose one of the following three test options */

#define LED_MODE	MAX7219_7SEG	/* 8 7-segment LEDs per module */
//#define LED_MODE	MAX7219_DOT	/* dot matrix ASCII font */
//#define LED_MODE	MAX7219_GRAPH	/* pure graphics */

/* If using MAX7219_DOT, choose one of the following two options */

//#define GRAPHIC_CLOCK		/* show clock on graphic display */
#define GRAPHIC_MARQUEE		/* show marquee on graphic display */

/* Use the following if you have 2 graphic modules connected together (8x64 pixels) */
//#define MODULE_CT	8		/* number of LED modules */
//#define CHAR_CT		10		/* number of characters */

/* Use the following if you have 3 7-segment modules connected together (3x8 digits) */
#define MODULE_CT	3

char clock_ms;		/* milliseconds */
char clock_10;		/* 1/10 second 0-255 */
char clock_tenths;
char clock_secs;
char clock_mins;
char clock_hours;

void setup(void);
void get_clock(char *);

const char marquee[] = "ATTENTION: Flight 121 from Oahu to Los Angeles has "
    "snakes on the plane. Enjoy your flight.  ";

#if LED_MODE == MAX7219_7SEG
const char *words[];
#endif

/******************************************************************************
 *
 *  Show hour:minute:second, 16 bit counter, and/or graphics
 */

int main() {
    char	 decimal[12];
    char	*mptr, *m2, mfrac;
    char	 wptr;
    char	 ac, i;
    int		 clock_last;
    int		 count16;
    
    m2, i;

    setup();
    m7219_init(LED_MODE, MODULE_CT);

    clock_last = 0;
    count16 = 0;
    ac = ' ';

    mptr = marquee;
    mfrac = 0;
    wptr = 0;

    do {
	if (clock_last == clock_10)	/* update every 1/10 second */
	    continue;
	clock_last = clock_10;

	count16++;
	bin16_dec(count16, decimal);
#if LED_MODE == MAX7219_7SEG
	m7219_curs(2, 0);
	m7219_puts(decimal);

	get_clock(decimal);
	m7219_curs(0, 0);
	m7219_puts(decimal);
	if (clock_tenths)		/* update words only every second */
	    continue;
	m7219_curs(1, 0);
	m7219_puts(words[wptr]);
	wptr++;
	if (!words[wptr])
	    wptr = 0;
#endif
#if LED_MODE == MAX7219_GRAPH
	m7219_option(MAX7219_WRAP);
	m7219_putc(clock_10);
#endif
#if LED_MODE == MAX7219_DOT
#ifdef GRAPHIC_CLOCK
	get_clock(decimal);
	m7219_curs(0, 0);
	m7219_puts(decimal);
	m7219_curs(6, 4);	/* justify 2 chars on right */
	m7219_putc('.');
	m7219_putc(clock_tenths + '0');
#endif
#ifdef GRAPHIC_MARQUEE
	m7219_option(MAX7219_NOWRAP);
	m2 = mptr;
	if (mfrac) {		/* starting at end with marquee wrap around */
	    m7219_curs(7, 8 - mfrac);
	    m7219_option(MAX7219_MARQUEE);	/* start marquee */
	}
	else
	    m7219_curs(0, 0);
	for (i = 0; i < CHAR_CT + 2; i++) {	/* extra 2 to show all pixels */
	    m7219_putc(*m2++);
	    if (!*m2)
		m2 = marquee;
	}
	mfrac++;
	if (mfrac == 6) {
	    mfrac = 0;
	    mptr++;
	    if (!*mptr)
		mptr = marquee;
	}
#endif
#endif

    } while(1);
}

/******************************************************************************
 *
 *  Board and globals setup
 */

void setup(void)
{
    CLK_CKDIVR = 0x00;	/* clock 16mhz */

    clock_ms = 0;
    clock_10 = 0;
    clock_tenths = 0;
    clock_secs   = 0;
    clock_mins   = 0;
    clock_hours  = 0;

    PB_DDR = 0x20;	/* output LED */
    PB_CR1 = 0xff;     	/* inputs have pullup */
    PB_CR2 = 0x00;	/* no interrupts, 2mhz output */

    TIM4_PSCR = 6;	/* prescaler = 64 */
    TIM4_ARR  = 250;	/* reset and interrupt every 1.0 ms */
    TIM4_CR1  = 1;	/* enable timer4 */
    TIM4_IER  = 1;	/* enable timer4 interrupt */

    __asm__ ("rim");
}
/* Available ports on STM8S103:
 *
 * A1..A3	A3 is HS
 * B4..B5	Open drain
 * C3..C7	HS
 * D1..D6	HS
 */

/******************************************************************************
 *
 *  Timer 4 interrupt
 */

void timer4_isr(void) __interrupt (IRQ_TIM4)
{
    TIM4_SR = 0;		/* clear the interrupt */

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
    if (clock_secs < 60)
	return;
    clock_secs = 0;
    clock_mins++;
    if (clock_mins < 60)
	return;
    clock_mins = 0;
    clock_hours++;
    if (clock_hours < 24)
	return;
    clock_hours = 0;
}

/******************************************************************************
 *
 *  Get the clock as string
 */

void get_clock(char *clk)
{
#if LED_MODE == MAX7219_7SEG
#define CLOCK_SEP '-'
#else
#define CLOCK_SEP ':'
#endif

    char	decimal[6];

    bin16_dec(clock_hours, decimal);
    *clk++ = decimal[3];
    *clk++ = decimal[4];
    *clk++ = CLOCK_SEP;
    bin16_dec(clock_mins, decimal);
    *clk++ = decimal[3];
    *clk++ = decimal[4];
    *clk++ = CLOCK_SEP;
    bin16_dec(clock_secs, decimal);
    *clk++ = decimal[3];
    *clk++ = decimal[4];
    *clk   = 0;
}

/******************************************************************************
 *
 *  Some words that a 7-segment module can display
 *  This is also a visual test of the decimal point.
 */

#if LED_MODE == MAX7219_7SEG
const char *words[] = {
    "A.BASHED ",
    "A.BSOLUTE",
    "AC.APULCO",
    "ACC.ESS  ",
    "ACCU.SED ",
    "AERA.TION",
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
    "BUTTER.ED",
    "CALCULU.S",
    "CAPSULE .",
    "C.ASCADE ",
    "CH.OPPED ",
    "CLO.UDED ",
    "COCA.COLA",
    "COFFE.E  ",
    "COLLOS.AL",
    "CONTEN.TS",
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
    "INSERTE.D",
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
    0
};
#endif
