# Choose the _103 or the _105 part here.
SDCC = sdcc -mstm8 -I../libs -L../libs -DSTM8103
#SDCC = sdcc -mstm8 -I../libs -L../libs -DSTM8105

LIBS = lib_stm8.lib
OBJS = test_flash.ihx test_keypad.ihx test_max7219.ihx \
	test_pwm.ihx test_tm1638.ihx test_ping.ihx test_lcd.ihx \
	test_tm1637.ihx test_w1209.ihx test_m9808.ihx test_spi.ihx \
	test_clock.ihx test_bindec.ihx test_delay.ihx test_uart.ihx \
	test_i2c.ihx test_gpio_int.ihx test_max6675.ihx

all: $(OBJS)

.SUFFIXES : .rel .c .ihx

.rel.ihx :
	$(SDCC) $< $(LIBS) 

.c.rel :
	$(SDCC) -c $<
clean:
	- rm -f *.adb *.asm *.cdb *.ihx *.lk *.lst *.map *.rel *.rst *.sym

