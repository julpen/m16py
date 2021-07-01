TARGET     ?= main

INPUT_FOLDERS = ./ ../common/crc display uart ../interpreter

#HEX ?= imem.hex

#MCU         ?= attiny2313
MCU         ?= atmega16

noop=
space = $(noop) $(noop)

LIBPATHS    = . ../interpreter ../common

CCLD        = avr-gcc
CCCONVERT   = avr-objcopy
CCFLAGS     = -Os -mmcu=$(MCU) -Wall -std=c99 -I $(subst $(space), -I,$(LIBPATHS)) -fdata-sections -ffunction-sections
LDFLAGS     = -mmcu=$(MCU) -Wall -L $(subst $(space), -L,$(LIBPATHS)) -Wl,--gc-sections

PROG        = avrdude
PORT        = /dev/ttyACM0
PROGRAMMER  = stk500v2
PRFLAGS     = -P $(PORT) -c $(PROGRAMMER) -p $(MCU) -B 15 -F

PROG16      = at16prog
PR16FLAGS   = -dpart=$(MCU)


SRCS = $(shell find $(INPUT_FOLDERS) -maxdepth 1 -name '*.c')
OBJS    = $(SRCS:.c=.o)


SRCS_NOMAIN = $(filter-out ./$(TARGET).c, $(SRCS))
OBJS_NOMAIN    = $(SRCS_NOMAIN:.c=.o)

HEADERS = $(shell find $(INPUT_FOLDERS) -maxdepth 1 -name '*.h')


all: $(TARGET).elf

size: $(TARGET).elf
	avr-size -C --mcu=$(MCU) $(TARGET).elf

$(TARGET).elf: $(OBJS) $(HEADERS)
	$(CCLD) $(OBJS) $(LDFLAGS) -o $@

$(TARGET).hex: $(TARGET).elf
	$(CCCONVERT) -O ihex $< $@


%.o: %.c $(HEADERS)
	$(CCLD) $(CCFLAGS) -c -o $@ $< $(LIBFLAGS)

hex: $(HEX)
	$(PROG) $(PRFLAGS) -U eeprom:w:$(HEX)

vhex: $(HEX)
	$(PROG) $(PRFLAGS) -U eeprom:v:$(HEX)

install: $(TARGET).elf $(TARGET).hex
ifeq ($(MCU), attiny2313)
	$(PROG) $(PRFLAGS) -U flash:w:$(TARGET).elf
endif
ifeq ($(MCU), atmega16)
	$(PROG16) $(PR16FLAGS) --segment=flash --erase
	$(PROG16) $(PR16FLAGS) --segment=flash --upload if=$(TARGET).hex
	$(PROG16) $(PR16FLAGS) --segment=flash --verify if=$(TARGET).hex
endif

verify: $(TARGET).elf
	$(PROG) $(PRFLAGS) -U flash:v:$<

clean:
	rm -f $(TARGET).elf $(OBJS) *.elf *.o $(TARGET).hex


