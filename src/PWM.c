#include <avr/io.h>
// rescales a value to a given Range
long map(long x, long in_min, long in_max, long out_min, long out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


// these functions initializes pwm for different timers
// with a 7.5% duty cycle.
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
// Function that calls all pwm initialization functions.
void init_all_PWM()
{
    init_phase_pwm();
    init_phase_pwm3();
    init_phase_pwm4();
    init_phase_pwm5();
}


//
void setpwm(volatile unsigned int value[],volatile uint16_t *pwmBuffer[4])
{
    int tempval[4]; 
// For loop that goes through the adc values and rescales them.
    for (int i = 0; i < 4; i++)
    {
        tempval[i] = map(value[i], 0, 1023, 500, 2500);//Maps the value to tempval.
    }
// for loop that updates each channel with a new OCRA value (duty cycle)
// the function slowly increments the old value towards the current value. 
    for (int channels = 0; channels < 4; channels++)
    {

        if (tempval[channels] > *pwmBuffer[channels])
        {

            *pwmBuffer[channels] = *pwmBuffer[channels] + 10;
        }
        else if (tempval[channels] < *pwmBuffer[channels])
        {

            *pwmBuffer[channels] = *pwmBuffer[channels] - 10;
        }
    }
}

// function receives a specified servomotor(Cardinalcord) and updates its direction(duty cycle).
void setpwm_UART(uint8_t cardinalcord, uint8_t cardinaldirection,volatile uint16_t *pwmBuffer[4])
{
    // direction is foward and our Duty cycle is larger than 12.5%
    // then it ensures that the duty cycle wont exceed 12.5%
    if (cardinaldirection == 1 && *pwmBuffer[cardinalcord] < 2500)
    {
        *pwmBuffer[cardinalcord] += 10;
        if (*pwmBuffer[cardinalcord]>2500)
        {
            *pwmBuffer[cardinalcord] = 2500;
        }
    }
    // same as before ensures duty cycle doesnt go below 2.5%
    else if (cardinaldirection == 0 && *pwmBuffer[cardinalcord] > 500)
    {
        *pwmBuffer[cardinalcord] -= 10;
        if (*pwmBuffer[cardinalcord]<500)
        {
            *pwmBuffer[cardinalcord] = 500;
        }
    }
}