#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000L
#define BAUD 38400UL

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
void delay_ms(uint16_t x);

int main (void) {
	DDRC = 0xFF;
	uart_init();

	while(1) {
		uart_putc('h');
		uart_putc('a');
		uart_putc('l');
		uart_putc('l');
		uart_putc('o');
		uart_putc(' ');
		delay_ms(200);
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
	unsigned char result = 0xFF;

	PORTC = result;
/*	switch (command) {
		case 0x01:
			switch (received) {
				case 0: result = 0xFF; break;
				case 1: result = 0x00; break;
				case 2: result = 0xFF; break;
				case 3: result = 0x00; break;
			}
			
			PORTC = result;
			command = 0x00;
			break;
		default:
			command = received;
	}*/
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

// General short delays
void delay_ms(uint16_t x) {
	uint8_t y, z;
	for ( ; x > 0 ; x--){
		for ( y = 0 ; y < 80 ; y++){
			for ( z = 0 ; z < 40 ; z++){
				asm volatile ("nop");
			}
		}
	}
}
