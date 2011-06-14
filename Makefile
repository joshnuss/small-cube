CC=avr-gcc
OBJCOPY=avr-objcopy
TARGET=small_cube
PROGRAMMER=usbtiny
PART=m48
CFLAGS=-g -mmcu=atmega8

rom.hex : $(TARGET).out
	$(OBJCOPY) -j .text -O ihex $(TARGET).out rom.hex

$(TARGET).out : $(TARGET).o
	$(CC) $(CFLAGS) -o $(TARGET).out -Wl,-Map,$(TARGET).map $(TARGET).o

$(TARGET).o : $(TARGET).c
	$(CC) $(CFLAGS) -Os -c $(TARGET).c

clean:
	rm *.o *.out *.hex *.map

program: rom.hex
	avrdude -c $(PROGRAMMER) -p $(PART) -U flash:w:rom.hex

fuses:
	avrdude -c $(PROGRAMMER) -p $(PART) -U lfuse:w:0xee:m -U hfuse:w:0xdf:m -U efuse:w:0x1:m

