#ifndef UART_H_
#define UART_H_

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

void uart_init(void);
void putcharUsart(char txmsg);
char getcharuart0(void);
void putstringuart(const char *str);
void putstring_P(const char *str);

#endif
