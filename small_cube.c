/*
 * small_cube.c
 *
 * a controller for managing a 4x4x4 LED cube
 * the controller connects to 2 shift registers (74HC595) to control the 16 LEDs on each row
 * the rows are scanned and are turned on individually via a BC337 NPN transistor
 *
 * Created: 5/28/2011 1:11:02 PM
 * Author: Josh
 */ 

#define F_CPU 16000000
#define BAUD_RATE 9600
#define BAUD_PRESCALE (((F_CPU / (BAUD_RATE * 16UL))) - 1)

#include <avr/io.h>
#include <inttypes.h>
#include <util/delay.h>

#define SHIFT_REGISTER_DDR  DDRC
#define SHIFT_REGISTER_PORT PORTC
#define CLEAR_PIN	0
#define DATA_PIN	1
#define CLOCK_PIN	2
#define LATCH_PIN	3

#define ROW_DDR  DDRB
#define ROW_PORT PORTB
#define ROW0_PIN 0
#define ROW1_PIN 1
#define ROW2_PIN 2
#define ROW3_PIN 3


void uart_init() {
	
  //UBRRL = (uint8_t) (F_CPU / (16UL * BAUD_RATE)) - 1; 
  //UBRRH = (uint8_t) ((F_CPU / (16UL * BAUD_RATE)) - 1) >>8;
  //
  
	// enable receiver and transmitter
	UCSRB |= (1 << RXEN) | (1 << TXEN); 

	// frame format: 8 data bits, 2 stop bits
	UCSRC |= (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1);

  UBRRH = BAUD_PRESCALE>>8;
  UBRRL = BAUD_PRESCALE;
	
}

unsigned int uart_read_char() {
	while(!(UCSRA & (1 << RXC)))
		;
	
	return UDR; 
}

void uart_put_char(unsigned int data) {
	while (!(UCSRA & (1 << UDRE)))
		;
	UDR = data;
}


void shift_register_update() {
	SHIFT_REGISTER_PORT |= _BV(LATCH_PIN);
	SHIFT_REGISTER_PORT &= ~_BV(LATCH_PIN);
}

void shift_register_clear() {
	SHIFT_REGISTER_PORT &= ~_BV(CLEAR_PIN);
	SHIFT_REGISTER_PORT |= _BV(CLEAR_PIN);
	
	shift_register_update();
}

void shift_register_init() {
	
	SHIFT_REGISTER_DDR |= _BV(CLEAR_PIN);
	SHIFT_REGISTER_DDR |= _BV(LATCH_PIN);
	SHIFT_REGISTER_DDR |= _BV(CLOCK_PIN);
	SHIFT_REGISTER_DDR |= _BV(DATA_PIN);	
	
	SHIFT_REGISTER_PORT &= ~_BV(LATCH_PIN);
	SHIFT_REGISTER_PORT &= ~_BV(CLOCK_PIN);
	SHIFT_REGISTER_PORT &= ~_BV(DATA_PIN);	
	SHIFT_REGISTER_PORT |= _BV(CLEAR_PIN);
	
	shift_register_clear();
}

void shift_register_write(unsigned int data) {
	uint8_t i;
	for (i=0;i<8;i++) {
		if (data & _BV(i))
			SHIFT_REGISTER_PORT |= _BV(DATA_PIN);
		else
			SHIFT_REGISTER_PORT &= ~_BV(DATA_PIN);
		
		SHIFT_REGISTER_PORT |= _BV(CLOCK_PIN);
		SHIFT_REGISTER_PORT &= ~_BV(CLOCK_PIN);
	}
	
	SHIFT_REGISTER_PORT &= ~_BV(DATA_PIN);
}


int main(void)
{
	uart_init();
  /*
	ROW_DDR  |= _BV(ROW0_PIN) | _BV(ROW1_PIN) | _BV(ROW2_PIN) | _BV(ROW3_PIN);
	ROW_PORT |= _BV(ROW0_PIN) | _BV(ROW1_PIN) | _BV(ROW2_PIN) | _BV(ROW3_PIN);
	
	shift_register_init();
	
	unsigned int i;
	
	for (i=0;i<4;i++)
	{
		ROW_PORT = _BV(i);
		shift_register_write(0xFF);
		shift_register_write(0xFF);
		shift_register_update();
		_delay_ms(100);
	}

  ROW_PORT |= _BV(ROW0_PIN);
	
	for (i=0;i<=128;i++) {
		shift_register_write(i);
		shift_register_update();
		_delay_ms(100);
	}	
	shift_register_clear();
	
	_delay_ms(5);
	*/
    while(1)
    {
      uart_put_char('>');
      char data = uart_read_char();
		
      shift_register_write(data);
      shift_register_update();
      
      uart_put_char('[');
      uart_put_char(data);
      uart_put_char(']');			
    }
}
