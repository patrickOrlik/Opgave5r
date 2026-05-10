#include <avr/io.h>
#include "clock.h"

void CTC_init(void)
{
    TCCR0B |= (1 << CS01) | (1 << CS00); // prescaler 64
    TCCR0A |= (1 << WGM01);              // CTC mode
    OCR0A = 249;                         // 16 MHz / 64 / 250 = 1 kHz
    TIMSK0 |= (1 << OCIE0A);             // enable compare-A interrupt
}
