#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <math.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include "I2C.h"
#include "ssd1306.h"
#include "clock.h"

enum cord
{
    X1,
    Y1,
    X2,
    Y2
};
unsigned int value = 0;
volatile bool Adcready = false;
volatile char Adcres[10] = {0};
volatile unsigned int channel = 0;
volatile unsigned int Adcvalues[4] = {0};
char tnpbuff[4] = {0};
volatile uint16_t *PWMbuffer[4] = {&OCR1A, &OCR3A, &OCR4A, &OCR5A};

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
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

void setpwm(volatile unsigned int value[])
{
    // value[X1] = map(value[X1],0,1023,500,2500);
    int tempval[4];
    for (int i = 0; i < 4; i++)
    {
        tempval[i] = map(value[i], 0, 1023, 500, 2500);
    }
    bool DONESTATE[4] = {false, false, false, false};
    // unsigned int tmp = map(value[X1],0,1023,500,2500);
    // while (!DONESTATE[X1] || !DONESTATE[X2] || !DONESTATE[Y1] || !DONESTATE[Y2])
    // {
    for (int channels = 0; channels < 4; channels++)
    {
        
        // if((Adcvalues[channels]-tempval[channels])>50){
        //     tempval[channels] = Adcvalues[channels];

        // }
        // if (!DONESTATE[channels])
        // {
        if (tempval[channels] > *PWMbuffer[channels])
        {
            // while (tempval[channels] > *PWMbuffer[channels])
            //{
            *PWMbuffer[channels] = *PWMbuffer[channels] + 10;
            // _delay_us(250);
            //}
        }
        else if (tempval[channels] < *PWMbuffer[channels])
        {
            // while (tempval[channels] < *PWMbuffer[channels])
            //{
            *PWMbuffer[channels] = *PWMbuffer[channels] - 10;
            // _delay_us(250);
            //}
        }
        // else
        // {
        //     DONESTATE[channels] = true;
        // }
        // _delay_us(100);
    }
    // }
}

void init_adc()
{
    ADCSRA |= (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2); // intern clock 125khz
    ADMUX = (1 << REFS0);                                // Voltage reference selection
    ADCSRA |= (1 << ADEN) | (1 << ADIE);                  // enable adc and interrupt complete
}
void select_channel(char channel)
{
    ADMUX = 0x40;
    ADMUX |= channel;
}

int main()
{
    init_phase_pwm();
    init_phase_pwm3();
    init_phase_pwm4();
    init_phase_pwm5();
    init_adc();
    sei();
    CTC_init();
    I2C_Init();
    InitializeDisplay();
    clear_display();
    char buffer[16];
    map(Adcvalues[X1], 0, 1023, 500, 2500);
    while (1)
    {
        if (Adcready)
        {

            sprintf(buffer, "X1:%4d", Adcvalues[X1]);
            sendStrXY(buffer, 0, 0);
            sprintf(buffer, "Y1:%4d", Adcvalues[Y1]);
            sendStrXY(buffer, 2, 0);
            sprintf(buffer, "X2:%4d", Adcvalues[X2]);
            sendStrXY(buffer, 4, 0);
            sprintf(buffer, "Y2:%4d", Adcvalues[Y2]);
            sendStrXY(buffer, 6, 0);
            Adcready = false;
            setpwm(Adcvalues);
            // setpwm(map(Adcvalues[Y1],0,1023,500,2500),&OCR3A);
            // setpwm(map(Adcvalues[X2],0,1023,500,2500),&OCR4A);
            // setpwm(map(Adcvalues[Y2],0,1023,500,2500),&OCR5A);
            _delay_ms(1);
        }
    }
}
ISR(ADC_vect)
{

    Adcready = true;
    Adcvalues[channel] = ADC;
}

ISR(TIMER0_COMPA_vect)
{

    // Select next channel
    select_channel(channel);
    // Start ADC conversion
    ADCSRA |= (1 << ADSC); // enables adc

    channel++;
    if (channel > 3)
    {
        channel = 0;
    }
}