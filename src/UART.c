#include <avr/io.h>
#define BAUD 19200
#define MYUBRRF (F_CPU/(16UL * BAUD) -1)

void uart_init(void) {

    
    UBRR0H = (unsigned char)(MYUBRRF >> 8);
    UBRR0L = (unsigned char)MYUBRRF;

    UCSR0A = (1<<U2X0);
    
    
    // Enable receiver and transmitter
    UCSR0B = (1 << TXEN0)|(1 << RXEN0)|(1<<RXCIE0);
    
    UCSR0C = (1 << UCSZ00)|(1<<(UCSZ01));
}

void putcharUsart(char txmsg)
{
    while(!(UCSR0A & (1<<UDRE0)));
    UDR0 = txmsg;
}

char getcharuart0()
{
    while(!(UCSR0A & (1<<RXC0)));
    return UDR0;
}
void putstringuart(char *str)
{
    while (*str)
    {
        putcharUsart(*str);
        str++;
    }
}
