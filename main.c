#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "board-def.h"

// TCNT1 per millisecond
#define TCNT1_MS (F_CPU / 256 / 1000)

struct Step {
	uint8_t state;
	int delay;
};

uint8_t text_steps_len = 46;
uint8_t tsi = 0; // index of text steps
struct Step text_steps[] = {
		{ 0b00000001, 250},
		{ 0b00000011, 250},
		{ 0b00000111, 250},
		{ 0b00001111, 250},
		{ 0b00011111, 250},
		{ 0b00111111, 250},
		{ 0b01111111, 250},
		{ 0b00000000, 600},
		{ 0b01111111, 600},
		{ 0b00000000, 600},
		{ 0b01111111, 600},
		{ 0b00000000, 600},
		{ 0b01111111, 600},
		{ 0b00000000, 600},
		{ 0b01111111, 3500},
		{ 0b00000000, 150},
		{ 0b01111111, 150},
		{ 0b00000000, 100},
		{ 0b01111111, 100},
		{ 0b00000000, 100},
		{ 0b01111111, 100},
		{ 0b00000000, 100},
		{ 0b01111111, 100},
		{ 0b00000000, 100},
		{ 0b01111111, 100},
		{ 0b00000000, 100},
		{ 0b01111111, 100},
		{ 0b00000000, 50},
		{ 0b01111111, 50},
		{ 0b00000000, 50},
		{ 0b01111111, 50},
		{ 0b00000000, 50},
		{ 0b01111111, 50},
		{ 0b00000000, 50},
		{ 0b01111111, 50},
		{ 0b00000000, 50},
		{ 0b01111111, 50},
		{ 0b00000000, 50},
		{ 0b01111111, 50},
		{ 0b00000000, 25},
		{ 0b01111111, 25},
		{ 0b00000000, 25},
		{ 0b01111111, 25},
		{ 0b00000000, 25},
		{ 0b01111111, 25},
		{ 0b00000000, 1250}
};

uint8_t logo_steps_len = 12;
uint8_t lsi = 0; // index of logo steps
struct Step logo_steps[] = {
		{ 0b00000001, 550},
		{ 0b00000011, 550},
		{ 0b00000111, 550},
		{ 0b00000000, 550},
		{ 0b00000111, 550},
		{ 0b00000000, 550},
		{ 0b00000111, 550},
		{ 0b00000000, 550},
		{ 0b00000111, 500},
		{ 0b00000000, 550},
		{ 0b00000111, 3500},
		{ 0b00000000, 1250}
};

void set_pin(volatile uint8_t *port, uint8_t pin, uint8_t value) {
	if (value)
		*port |= _BV(pin);
	else
		*port &= ~_BV(pin);
}

/*
 * dispatch logo state
 */
void dispatch_logo(uint8_t state) {
	set_pin(&PORT_EH, PIN_EH, state & 0b001);
	set_pin(&PORT_SS, PIN_SS, state & 0b010);
	set_pin(&PORT_BS, PIN_BS, state & 0b100);
}

/*
 * dispatch text state
 */
void dispatch_text(uint8_t state) {
	set_pin(&PORT_P, PIN_P, state & 0b0000001);
	set_pin(&PORT_R1, PIN_R1, state & 0b0000010);
	set_pin(&PORT_D, PIN_D, state & 0b0000100);
	set_pin(&PORT_H, PIN_H, state & 0b0001000);
	set_pin(&PORT_S, PIN_S, state & 0b0010000);
	set_pin(&PORT_R2, PIN_R2, state & 0b0100000);
	set_pin(&PORT_A, PIN_A, state & 0b1000000);
}

/*
 * timer 1 compare match interrupt A
 * set text state
 */
ISR(TIMER1_COMPA_vect) {

	OCR1A += (uint16_t) text_steps[tsi].delay * TCNT1_MS;
	dispatch_text(text_steps[tsi].state);

	tsi = ++tsi < text_steps_len ? tsi : 0;

}

/*
 * timer 1 compare match interrupt B
 * set text state
 */
ISR(TIMER1_COMPB_vect) {

	OCR1B += (uint16_t) logo_steps[lsi].delay * TCNT1_MS;
	dispatch_logo(logo_steps[lsi].state);

	lsi = ++lsi < logo_steps_len ? lsi : 0;

}

/*
 *
 */
void init(void) {

	// adjust timer 1 mode to normal (0000)
	// start from 0 to 0xffff and overflowing
	TCCR1A &= ~((1 << WGM11) | (1 << WGM10));
	TCCR1B &= ~((1 << WGM13) | (1 << WGM12));

	// adjust timer 1 clock to 1/256 of micro-controller clock ==> 1/256 * 1000000 = 3906 cycle per second
	TCCR1B &= ~((1 << CS10) | (1 << CS11));
	TCCR1B |= (1 << CS12);

	TCNT1 = 0;
	OCR1A = 1;
	OCR1B = 1;

	// timer interrupt flag of OCR1A and TCNT1 compare match
	// will be 0 when try to set it 1
	TIFR |= (1 << OCF1A);

	// timer interrupt flag of OCR1B and TCNT1 compare match
	// will be 0 when try to set it 1
	TIFR |= (1 << OCF1B);

	// enable OCR1A and TCNT1 compare match A interrupt
	TIMSK |= (1 << OCIE1A);

	// enable OCR1B and TCNT1 compare match B interrupt
	TIMSK |= (1 << OCIE1B);
}

int main(void) {
	sei();
	DDRA = 0xFF;
	DDRC = 0xFF;
	init();
	while (1);
}
