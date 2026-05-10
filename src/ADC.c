#include <avr/io.h>
#include "ADC.h"

void init_adc(void)
{
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // /128 prescaler -> 125 kHz ADC clock
    ADMUX = (1 << REFS0);                                 // AVcc reference, channel 0
    ADCSRA |= (1 << ADEN) | (1 << ADIE);                  // enable ADC + conversion-complete interrupt
}

void select_channel(uint8_t channel)
{
    ADMUX = (1 << REFS0) | (channel & 0x07);              // keep AVcc, set MUX4..0 (channels 0-7)
}
