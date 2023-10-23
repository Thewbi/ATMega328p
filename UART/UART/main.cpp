/*
 * main.cpp
 *
 * Created: 07.09.2023 14:42:10
 * Author : U5353
 */ 

#define F_CPU 16000000UL

#define BAUDRATE 9600

// Teiler = (Systemtakt / (16 * BAUDRATE)) - 1
#define BAUD_PRESCALLER (((F_CPU / (16UL * BAUDRATE))) - 1)

#define LINE_FEED 10

#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>
#include <stdio.h>
#include <string.h>

void usart_init(void);
unsigned char usart_receive(void);
void usart_put_byte(unsigned char data);
void usart_put_string(unsigned char* string_ptr);

unsigned char my_string[] = "Command received!";

// new data arrived over the UART
volatile int uart_recv_interrupt = 0;

const unsigned char uart_recv_buffer_size = 128;
unsigned char uart_recv_buffer_index = 0;
unsigned char uart_recv_buffer[uart_recv_buffer_size] = { 0 };

int main(void)
{
	// enable global interrupt
	sei();
	
	// initialize UART
	usart_init();
    
    while(1)
	{
		if (uart_recv_interrupt)
		{
			// all buffer content that ends in a LINE_FEED is interpreted as user-input
			if (uart_recv_buffer[uart_recv_buffer_index - 1] == LINE_FEED)
			{
				usart_put_string(uart_recv_buffer);
				
				uart_recv_interrupt = 0;
				
				// reset the buffer to 0
				uart_recv_buffer_index = 0;
				memset(uart_recv_buffer, 0, sizeof(uart_recv_buffer));
			}
		}
	    
		_delay_ms(10);
    }
	
    return 0;
}

void usart_init(void)
{
	// Baudrate einstellen
	UBRR0H = (unsigned char)(BAUD_PRESCALLER >> 8);
	UBRR0L = (unsigned char)(BAUD_PRESCALLER);
	
	// asynchroner USART Modus
	UCSR0C &= ~((1 << UMSEL01) | (1 << UMSEL00));
	
	// 8-Datenbits
	UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01);
	
	// 1-Stoppbit
	UCSR0C &= ~(1 << USBS0);
	
	// USART Sende- und Empfangseinheiten aktivieren
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	
	// receive interrupt aktivieren
	UCSR0B |= (1 << RXCIE0);
}

unsigned char usart_receive(void)
{
	// Daten verfügbar?
	while ((UCSR0A & (1 << RXC0)) == 0);
	
	return UDR0;
}

void usart_put_byte(unsigned char data)
{
	// Senderegister frei?
	while ((UCSR0A & (1 << UDRE0)) == 0);
	
	UDR0 = data;
}void usart_put_string(unsigned char* string_ptr)
{
	while (*string_ptr != 0x00)
	{
		usart_put_byte(*string_ptr);
		string_ptr++;
	}
}
ISR(USART_RX_vect)
{
	// communicate the arrival of a new character to the main loop
	uart_recv_interrupt = 1;
	
	// store the received byte in the receive buffer
	uart_recv_buffer[uart_recv_buffer_index] = UDR0;
	
	// increment index
	uart_recv_buffer_index++;
	if (uart_recv_buffer_index > uart_recv_buffer_size)
	{
		uart_recv_buffer_index = 0;
	}
}
