#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <math.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include "I2C.h"
#include "ssd1306.h"
#include "clock.h"
#include "UART.h"
#include "PWM.h"
#include "ADC.h"
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
volatile int RX0_COMPLETE_FLAG = 0;
volatile bool Adcready = false;
volatile unsigned int channel = 0;
volatile unsigned int Adcvalues[4] = {0};
volatile uint16_t *PWMbuffer[4] = {&OCR1A, &OCR3A, &OCR4A, &OCR5A};

const char welcomestring[] PROGMEM =
    "Hello and welcome to roboarm.\nPress 1 for Keyboard controle\nPress 0 for Joystick controle";
const char keyboardstatestring[] PROGMEM =
    "You have chosen the keyboard controls are\n Extend crane: w\t-Subtract crane: s\t-Rotate left: a\t-Rotate right: d\n-Lift crane: i\t-Lower crane: o\t-Open claws: k\t-Close claws: l\nYou can always switch byt pressing 0\n";
const char joystickstatestring[] PROGMEM =
    "you have now entered joystick controls\nYou can always switch byt pressing 0\n";

void updateposition_UART()
{
    switch (bruh)
    {
    case 'a':
        setpwm_UART(X1, 1, PWMbuffer);
        break;
    case 'd':
        setpwm_UART(X1, 0, PWMbuffer);
        break;
    case 'w':
        setpwm_UART(Y1, 1, PWMbuffer);
        break;
    case 's':
        setpwm_UART(Y1, 0, PWMbuffer);
        break;
    case 'i':
        setpwm_UART(Y2, 1, PWMbuffer);
        break;
    case 'o':
        setpwm_UART(Y2, 0, PWMbuffer);
        break;
    case 'k':
        setpwm_UART(X2, 1, PWMbuffer);
        break;
    case 'l':
        setpwm_UART(X2, 0, PWMbuffer);
        break;

    default:
        break;
    }
}

int main()
{
    uart_init();
    init_all_PWM();
    init_adc();
    sei();
    CTC_init();
    I2C_Init();
    InitializeDisplay();
    clear_display();
    char buffer[16];
    controleState ctState = joystickState;
    while (1)
    {
        if (!choosingState)
        {
            putstring_P(welcomestring);
            while (RX0_COMPLETE_FLAG == 0)
                ;
            switch (bruh)
            {
            case KEYBOARD:
                putstring_P(keyboardstatestring);
                ctState = keyboardState;
                choosingState = true;
                RX0_COMPLETE_FLAG = 0;
                break;
            case JOYSTICK:
                putstring_P(joystickstatestring);
                ctState = joystickState;
                choosingState = true;
                RX0_COMPLETE_FLAG = 0;
                break;
            default:
                RX0_COMPLETE_FLAG = 0;
                break;
            }
        }
        switch (ctState)
        {
        case keyboardState:
            if (RX0_COMPLETE_FLAG == 1 && bruh == '0')
            {
                putstring_P(joystickstatestring);
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
                    putstring_P(keyboardstatestring);
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
                setpwm(Adcvalues, PWMbuffer);
            }
            break;
        default:
            break;
        }
    }
}

ISR(ADC_vect)
{
    Adcvalues[channel] = ADC;
    channel = (channel + 1) & 0x03;
    Adcready = true;
}

ISR(TIMER0_COMPA_vect)
{
    select_channel(channel);
    ADCSRA |= (1 << ADSC);
}

ISR(USART0_RX_vect)
{
    bruh = UDR0;
    RX0_COMPLETE_FLAG = 1;
}
