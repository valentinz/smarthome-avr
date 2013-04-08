#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000L
#define BAUD 9600UL

#define UBRR_VAL ((F_CPU+BAUD*8)/(BAUD*16)-1)   // clever runden
#define BAUD_REAL (F_CPU/(16*(UBRR_VAL+1)))     // Reale Baudrate
#define BAUD_ERROR ((BAUD_REAL*1000)/BAUD) // Fehler in Promille, 1000 = kein Fehler.
 
#if ((BAUD_ERROR<990) || (BAUD_ERROR>1010))
  #error Systematischer Fehler der Baudrate grösser 1% und damit zu hoch! 
#endif

unsigned char command = 0x00;

void uart_init(void);

int uart_putc(unsigned char c);
uint8_t uart_getc(void);

int main (void) {
	DDRC = 0xFF;
	PORTC = 0x00;
	uart_init();

	while(1) {
	}
   
	return 0;
}

void uart_init(void) {
	// enable uart
	UCSRB |= (1<<TXEN);                // UART TX einschalten
	UCSRC |= (1<<URSEL)|(3<<UCSZ0);    // Asynchron 8N1 

	UBRRH = UBRR_VAL >> 8;
	UBRRL = UBRR_VAL & 0xFF;

	UCSRB |= (1<<RXCIE) | (1<<RXEN);

	// enable interrupts
	sei();
}

ISR (USART_RXC_vect) {
	unsigned char received = UDR;
	unsigned char result = 0x00;

	switch (command) {
	case 0x01: // Write Aeration level 0x01 [Level,8-bit]
		   // Result: 0x01
		result = PORTC;
		/*
		 *	Dec	PC7	PC6	PC5	PC4
		 *	3	0 	1 	1 	x	Level 0
		 *	2	0 	1 	0 	x	Level 1
		 *	0	0 	0 	0 	x	Level 2
		 *	4	1 	0 	0 	x	Level 3 
		 *	x => no change
		 */
		switch (received) {
		case 0: result &= 0x7F; result |= 0x60; break;
		case 1: result &= 0x5F; result |= 0x40; break;
		case 2: result &= 0x1F; break;
		case 3: result &= 0x9F; result |= 0x80; break;
		}
		
		PORTC = result;
		uart_putc(0x01);
		command = 0x00;
		break;
	case 0x02: 
	default:
		switch (received) {
		case 0x01: // Write Aeration level 0x01 [Level,8-bit]
			   // Result: 0x01
			command = received;
			uart_putc(0x00);
			break;
		case 0x02: // Get Aeration level 0x02
			   // Result: 0x00 [Level,8-bit] [~Level,8-bit]
			switch (PORTC >> 5) {
			case 3:  result = 0x00; break;
			case 2:  result = 0x01; break;
			case 0:  result = 0x02; break;
			case 4:  result = 0x03; break;
			default: result = 0xFF; break;
			}
			uart_putc(0x00);
			uart_putc(result);
			uart_putc(~result);
			break;
		default:
			command = 0x00;
			uart_putc(0xFF);
		}
	}
}

int uart_putc(unsigned char c) {
	while (!(UCSRA & (1<<UDRE)))  /* warten bis Senden moeglich */
	{
	}
	UDR = c;                      /* sende Zeichen */
	return 0;
}

uint8_t uart_getc(void) {
	while (!(UCSRA & (1<<RXC)))   // warten bis Zeichen verfuegbar
		;
	return UDR;                   // Zeichen aus UDR an Aufrufer zurueckgeben
}
