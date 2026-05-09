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
#include "UART.h"
#define KEYBOARD 49
#define JOYSTICK 48

enum cord
{
    X1,
    Y1,
    X2,
    Y2
};
typedef enum
{
    keyboardState,
    joystickState
} controleState;
bool choosingState = false;
volatile char bruh;
unsigned int value = 0;
volatile int RX0_COMPLETE_FLAG = 0;
volatile bool Adcready = false;
volatile char Adcres[10] = {0};
volatile unsigned int channel = 0;
volatile unsigned int Adcvalues[4] = {0};
char tnpbuff[4] = {0};
volatile uint16_t *PWMbuffer[4] = {&OCR1A, &OCR3A, &OCR4A, &OCR5A};
char welcomestring[128] = "Hello and welcome to roboarm.\nPress 1 for Keyboard controle\nPress 0 for Joystick controle";
char keyboardstatestring[264] = "You have chosen the keyboard controls are\n Extend crane: w-\t-Subtract crane: s-\t-Rotate left: a-\t-Rotate right: d-\n-Lift crane: i-\t-Lower crane: o-\t-Open claws: k-\t-Close claws: l\n";
char joystickstatestring[64] = "you have now entered joystick controls\n";
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
void setpwm_UART(uint8_t cardinalcord, uint8_t cardinaldirection)
{
    if (cardinaldirection == 1 && *PWMbuffer[cardinalcord] < 2500)
    {
        *PWMbuffer[cardinalcord] += 10;
    }
    else if (cardinaldirection == 0 && *PWMbuffer[cardinalcord] > 500)
    {
        *PWMbuffer[cardinalcord] -= 10;
    }
}
void updateposition_UART()
{
    switch (bruh)
    {
    case 'a':
        setpwm_UART(X1, 1);
        break;
    case 'd':
        setpwm_UART(X1, 0);
        break;
    case 'w':
        setpwm_UART(Y1, 1);
        break;
    case 's':
        setpwm_UART(Y1, 0);
        break;
    case 'i':
        setpwm_UART(Y2, 1);
        break;
    case 'o':
        setpwm_UART(Y2, 0);
        break;
    case 'k':
        setpwm_UART(X2, 1);
        break;
    case 'l':
        setpwm_UART(X2, 0);
        break;

    default:
        break;
    }
}

void setpwm(volatile unsigned int value[])
{
    int tempval[4];
    for (int i = 0; i < 4; i++)
    {
        tempval[i] = map(value[i], 0, 1023, 500, 2500);
    }

    for (int channels = 0; channels < 4; channels++)
    {

        if (tempval[channels] > *PWMbuffer[channels])
        {

            *PWMbuffer[channels] = *PWMbuffer[channels] + 10;
        }
        else if (tempval[channels] < *PWMbuffer[channels])
        {

            *PWMbuffer[channels] = *PWMbuffer[channels] - 10;
        }
    }
}

void init_adc()
{
    ADCSRA |= (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2); // intern clock 125khz
    ADMUX = (1 << REFS0);                                 // Voltage reference selection
    ADCSRA |= (1 << ADEN) | (1 << ADIE);                  // enable adc and interrupt complete
}
void select_channel(char channel)
{
    ADMUX = 0x40;
    ADMUX |= channel;
}

int main()
{
    uart_init();
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
    controleState ctState;
    map(Adcvalues[X1], 0, 1023, 500, 2500);
    while (1)
    {
        if (!choosingState)
        {
            putstringuart(welcomestring);
            while (RX0_COMPLETE_FLAG == 0)
                ;
            switch (bruh)
            {
            case KEYBOARD:
                putstringuart(keyboardstatestring);
                ctState = keyboardState;
                choosingState = true;
                RX0_COMPLETE_FLAG = 0;
                break;
            case JOYSTICK:
                putstringuart(joystickstatestring);
                ctState = joystickState;
                choosingState = true;
                RX0_COMPLETE_FLAG = 0;
                break;
            default:
                break;
            }
        }
        switch (ctState)
        {
        case keyboardState:
            if (RX0_COMPLETE_FLAG == 1 && bruh == '0')
            {
                putstringuart(joystickstatestring);
                ctState = joystickState;
                RX0_COMPLETE_FLAG = 0;
            }
            else if (RX0_COMPLETE_FLAG == 1)
            {

                updateposition_UART();

                RX0_COMPLETE_FLAG = 0;
            }
            break;
        case joystickState:
            if (Adcready)
            {
                if (RX0_COMPLETE_FLAG == 1 && bruh == '0')
                {
                    putstringuart(keyboardstatestring);
                    ctState = keyboardState;
                    RX0_COMPLETE_FLAG = 0;
                }
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
                _delay_ms(1);
            }
            break;
        default:
            break;
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
ISR(USART0_RX_vect)
{
    bruh = UDR0;
    RX0_COMPLETE_FLAG = 1;
}
