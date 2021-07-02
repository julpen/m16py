TARGET     ?= m16py

INPUT_FOLDERS = . ../interpreter ../common/crc

#HEX ?= imem.hex

#MCU         ?= attiny2313
MCU         ?= atmega16

noop=
space = $(noop) $(noop)

LIBPATHS    = . ../interpreter ../common

CCLD        = gcc
CCFLAGS     = -D PROGRAM=1 -Os -Wall -std=c99 -I $(subst $(space), -I,$(LIBPATHS)) -fdata-sections -ffunction-sections
LDFLAGS     = -D PROGRAM=1 -Wall -lreadline -L $(subst $(space), -L,$(LIBPATHS)) -Wl,--gc-sections

SRCS = $(shell find $(INPUT_FOLDERS) -maxdepth 1 -name '*.c')
OBJS    = $(SRCS:.c=.o)


SRCS_NOMAIN = $(filter-out ./$(TARGET).c, $(SRCS))
OBJS_NOMAIN    = $(SRCS_NOMAIN:.c=.o)

HEADERS = $(shell find $(INPUT_FOLDERS) -maxdepth 1 -name '*.h')


all: $(TARGET)

size: $(TARGET)
	size $(TARGET)

$(TARGET): $(OBJS) $(HEADERS)
	$(CCLD) $(OBJS) $(LDFLAGS) -o $@


%.o: %.c $(HEADERS)
	$(CCLD) $(CCFLAGS) -c -o $@ $< $(LIBFLAGS)

clean:
	rm -f $(TARGET) $(OBJS) *.elf *.o $(TARGET).hex


