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
#define BAUD_RATE 9600UL
#define BAUD_PRESCALE ((F_CPU / (BAUD_RATE << 4)) - 1)

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
#define ROW_MASK (_BV( ROW0_PIN ) | _BV( ROW1_PIN ) | _BV( ROW2_PIN ) | _BV( ROW3_PIN ))


void uart_init() {
	
  UBRR0H = (uint8_t)BAUD_PRESCALE >> 8;
  UBRR0L = (uint8_t)BAUD_PRESCALE;
  
	// frame format: no partiy, 8 data bits, 1 stop bits
  UCSR0C |= (0 << USBS0) | (1 << UCSZ01) | (1 << UCSZ00);

	// enable receiver and transmitter
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0); 
}

unsigned int uart_read_char() {
	while(!(UCSR0A & (1 << RXC0)))
		;
	
	return UDR0; 
}

void uart_put_char(unsigned int data) {
	while (!(UCSR0A & (1 << UDRE0)))
		;
	UDR0 = data;
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


#define CUBE_SIZE 4

uint16_t data[CUBE_SIZE];

void cube_draw_level(uint8_t level) {
  uint16_t value = data[level];

  ROW_PORT &= ~ROW_MASK;
  ROW_PORT |= _BV(level);

  shift_register_write(value);
  shift_register_write(value >> 8);
  shift_register_update();
}

void cube_init() {
  uint8_t i;
  for(i=0;i<CUBE_SIZE;i++)
    data[i] = 0;
}

void cube_set_all(uint16_t value) {
  uint8_t i;
  for(i=0;i<CUBE_SIZE;i++)
    data[i] = value;
}
void cube_set_level(uint8_t level, uint16_t value) {
  data[level] = value;
}

void cube_draw() {
  uint8_t i;
  for(i=0;i<CUBE_SIZE;i++)
    cube_draw_level(i);
}

int main(void)
{
  ROW_DDR  |= ROW_MASK;
  ROW_PORT &= ~ROW_MASK;

  uart_init();
  shift_register_init();
  cube_init();
  
  unsigned int i, j;

  while(1) {
    cube_set_all(0xFFFF);
    for (i=0;i<4;i++)
    {
      cube_draw_level(i);
      _delay_ms(1000);

      for (j=0;j<4;j++) {
        cube_set_level(i, 15 << (j*4));
        cube_draw_level(i);
        _delay_ms(100);
      }	
      for (j=2;j>-1;j--) {
        cube_set_level(i, 15 << (j*4));
        cube_draw_level(i);
        _delay_ms(100);
      }	

      shift_register_clear();
    }
  }
	
  ROW_PORT &= ~ROW_MASK;
  ROW_PORT |= _BV(ROW2_PIN);
  while(1)
  {
    char data = uart_read_char();
  
    shift_register_write(data);
    shift_register_update();
    
    uart_put_char('[');
    uart_put_char(data);
    uart_put_char(']');			
  }
}
