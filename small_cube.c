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
#include <avr/interrupt.h>

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

volatile uint16_t data[CUBE_SIZE];

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

void timer_init() {
  TIMSK0 = _BV(OCIE0A);
  TCCR0A = _BV(WGM01);
  TCCR0B = _BV(CS02) | _BV(CS00); // Clock / 1024
  OCR0A = 60;          // 0.001024*244 ~= .25 SIG_OUTPUT_COMPARE0A will be triggered 4 times per second.

  sei(); 
}

int main(void)
{
  ROW_DDR  |= ROW_MASK;
  ROW_PORT &= ~ROW_MASK;

  uart_init();
  shift_register_init();
  cube_init();
  timer_init();
  
  unsigned int i, j;
    cube_set_all(0);
    for (i=0;i<4;i++)
    {
   //   cube_draw_level(i);
      cube_set_level(i, 0x0);
      _delay_ms(100);

      for (j=0;j<4;j++) {
        cube_set_level(i, 15 << (j*4));
  //      cube_draw_level(i);
        _delay_ms(20);
      }	
      for (j=2;j>-1;j--) {
        cube_set_level(i, 15 << (j*4));
  //      cube_draw_level(i);
        _delay_ms(20);
      }	

    }
  for(i=0;i<5;i++) {
    cube_init();
    cube_set_level(1, 1632);
    cube_set_level(2, 1632);
    _delay_ms(100);
    cube_set_level(1, 63903);
    cube_set_level(2, 63903);
    cube_set_level(0, 0xFFFF);
    cube_set_level(3, 0xFFFF);
    
    _delay_ms(100);
  }


  while(1) {
    cube_init();
    
  cube_init();

    for (i=0;i<4;i++) {
      cube_set_level(3, 15);
      cube_set_level(2, 1);
      cube_set_level(1, 7);
      cube_set_level(0, 1);
        data[0] = data[0] + ( data[0] << 4 );
        data[1] = data[1] + ( data[1] << 4 );
        data[2] = data[2] + ( data[2] << 4 );
        data[3] = data[3] + ( data[3] << 4 );
        _delay_ms(300);
      

      for (j=0;j<4;j++) {
        data[0] = data[0] << 4;
        data[1] = data[1] << 4;
        data[2] = data[2] << 4;
        data[3] = data[3] << 4;
        _delay_ms(300);
      }
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

volatile int counter = 0;

ISR(SIG_OUTPUT_COMPARE0A)
{
  cube_draw_level(counter % 4);

  counter++; 
}
