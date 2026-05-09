#include <avr/io.h>
void init_phase_pwm()
{
    DDRB |= (1 << PB5);
    TCCR1A |= (1 << COM1A1);
    TCCR1B = (1 << CS11) | (1 << WGM13);
    ICR1 = 20000;
    OCR1A = 1500;
}
void init_phase_pwm3()
{
    DDRE |= (1 << PE3);
    TCCR3A |= (1 << COM3A1);
    TCCR3B = (1 << CS11) | (1 << WGM13);
    ICR3 = 20000;
    OCR3A = 1500;
}
void init_phase_pwm4()
{
    DDRH |= (1 << PH3);
    TCCR4A |= (1 << COM1A1);
    TCCR4B = (1 << CS11) | (1 << WGM13);
    ICR4 = 20000;
    OCR4A = 1500;
}

void init_phase_pwm5()
{
    DDRL |= (1 << PL3);
    TCCR5A |= (1 << COM1A1);
    TCCR5B = (1 << CS11) | (1 << WGM13);
    ICR5 = 20000;
    OCR5A = 1500;
}
void init_all_PWM()
{
    init_phase_pwm();
    init_phase_pwm3();
    init_phase_pwm4();
    init_phase_pwm5();
}