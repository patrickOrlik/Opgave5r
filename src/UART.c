#include <avr/io.h>
#include <avr/pgmspace.h>
#include "UART.h"

#define BAUD 19200
#define MYUBRRF (F_CPU/(16UL * BAUD) - 1)

void uart_init(void)
{
    UBRR0H = (unsigned char)(MYUBRRF >> 8);
    UBRR0L = (unsigned char)MYUBRRF;

    // Enable RX, TX, and RX-complete interrupt
    UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);

    // 8-bit characters
    UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);
}

void putcharUsart(char txmsg)
{
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = txmsg;
}

char getcharuart0(void)
{
    while (!(UCSR0A & (1 << RXC0)));
    return UDR0;
}

void putstringuart(const char *str)
{
    while (*str)
    {
        putcharUsart(*str);
        str++;
    }
}

void putstring_P(const char *str)
{
    char c;
    while ((c = pgm_read_byte(str++)))
    {
        putcharUsart(c);
    }
}
