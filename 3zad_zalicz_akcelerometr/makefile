CC = arm-eabi-gcc
OBJCOPY = arm-eabi-objcopy
FLAGS = -mthumb -mcpu=cortex-m4
CPPFLAGS = -DSTM32F411xE
CFLAGS = $(FLAGS) -Wall -Wextra -g \
			-O2 -ffunction-sections -fdata-sections \
			-I/opt/arm/stm32/inc \
			-I/opt/arm/stm32/CMSIS/Include \
			-I/opt/arm/stm32/CMSIS/Device/ST/STM32F4xx/Include \
			-MD \
			-MP
LDFLAGS = $(FLAGS) -Wl,--gc-sections -nostartfiles \
			-L/opt/arm/stm32/lds -Tstm32f411re.lds
vpath %.c /opt/arm/stm32/src

OBJECTS = zad3_zalicz.o startup_stm32.o delay.o gpio.o i2c1_handlers.o \
			init_funcs.o queue.o
TARGET = comm

.SECONDARY: $(TARGET).elf $(OBJECTS)

all: $(TARGET).bin

%.elf : $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@
%.bin : %.elf
	$(OBJCOPY) $< $@ -O binary
clean :
	rm -f *.bin *.elf *.hex *.d *.o *.bak *~