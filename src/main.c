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
#include "PWM.h"
#include "ADC.h"

// definitions for directions and controls
#define KEYBOARD 49
#define JOYSTICK 48
#define FORWARD 'w'
#define BACKWARDS 's'
#define LEFT 'a'
#define RIGHT 'd'
#define LIFT 'i'
#define DESCEND 'o'
#define CLOSE 'k'
#define OPEN 'l'

enum cord // Enums til joystick & keyboard koordinator.
{
    X1,
    Y1,
    X2,
    Y2
};
typedef enum
{
    keyboardState,
    joystickState,
    notmet
} controlState;
bool choosingState = false; // Used for choosing controls
volatile char directionvar; // Used for storing direction byte from RXreg
volatile int RX0_COMPLETE_FLAG = 0; // Used for interrupt complete flag
volatile bool Adcready = false; // Used for checking if Adc is ready.
volatile unsigned int channel = 0; // used for selecting channel
volatile unsigned int Adcvalues[4] = {0}; // Used for storing ADCvalues
volatile uint16_t *PWMbuffer[4] = {&OCR1A, &OCR3A, &OCR4A, &OCR5A};

// welcome strings and other user messages.
char welcomestring[128] = "Hello and welcome to roboarm.\nPress 1 for Keyboard controle\nPress 0 for Joystick controle";
char keyboardstatestring[264] = "You have chosen the keyboard controls are\n Extend crane: w\t-Subtract crane: s\t-Rotate left: a\t-Rotate right: d\n-Lift crane: i\t-Lower crane: o\t-Open claws: k\t-Close claws: l\nYou can always switch byt pressing 0\n";
char joystickstatestring[128] = "you have now entered joystick controls\nYou can always switch byt pressing 0\n";


void updateposition_UART()
{
    switch (directionvar) // switch cases that checks for user input and updates position.
    {
    case LEFT:
        setpwm_UART(X1, 1, PWMbuffer);
        break;
    case RIGHT:
        setpwm_UART(X1, 0, PWMbuffer);
        break;
    case FORWARD:
        setpwm_UART(Y1, 1, PWMbuffer);
        break;
    case BACKWARDS:
        setpwm_UART(Y1, 0, PWMbuffer);
        break;
    case LIFT:
        setpwm_UART(Y2, 1, PWMbuffer);
        break;
    case DESCEND:
        setpwm_UART(Y2, 0, PWMbuffer);
        break;
    case CLOSE:
        setpwm_UART(X2, 1, PWMbuffer);
        break;
    case OPEN:
        setpwm_UART(X2, 0, PWMbuffer);
        break;

    default:
        break;
    }
}

int main()
{
    uart_init(); // Initializes UART
    init_all_PWM(); // Initializes all PWM timers 
    init_adc(); // Initializes ADC
    sei(); // Enables global interrupt
    CTC_init(); // Enables timer in CTC mode
    I2C_Init(); // Enables I2C
    InitializeDisplay(); // Initializes OLED Display
    clear_display(); // Clears OLED display
    char Joystickbuffer[16]; // buffer for storing ADC joystick values as a string 
    controlState ctState; 
    while (1)
    {
        
        if (!choosingState)// checks if controlstate has been set
        {
            putstringuart(welcomestring);//puts welcome string in Serial monitor
            while (RX0_COMPLETE_FLAG == 0); // waits for receive complete



            switch (directionvar)// switch case checking for which mode is chosen
            {
            case KEYBOARD: 
                putstringuart(keyboardstatestring); //Prints controls to Serial monitor
                ctState = keyboardState;// sets controlstate to keyboard
                choosingState = true; // sets that a state has been chosen
                break;
            case JOYSTICK:
                putstringuart(joystickstatestring); // prints mode selected to Serial monitor
                ctState = joystickState; // sets controlstate to joystick
                choosingState = true; // sets that a state has  been chosen
                break;
            default:
                ctState = notmet; // if requirements not met for either case then loop.

                break;
            }
            RX0_COMPLETE_FLAG = 0; //sets receive complete 0.
        }
        
        switch (ctState) // checks for which controlstate has been chosen.
        {
        case keyboardState:
            if (RX0_COMPLETE_FLAG == 1 && directionvar == '0')// checks if we have received a 0 if yes we switch control mode
            {
                putstringuart(joystickstatestring);
                ctState = joystickState;
                RX0_COMPLETE_FLAG = 0;
            }
            else if (RX0_COMPLETE_FLAG == 1) //if anything else is received call updateposition function.
            {

                updateposition_UART();

                RX0_COMPLETE_FLAG = 0;
            }
            break;
        case joystickState:
            if (Adcready) // waits for update of ADCvalue
            {
                if (RX0_COMPLETE_FLAG == 1 && directionvar == '0')// checks if 0 has been sent via Uart if yes switch controls.
                {
                    putstringuart(keyboardstatestring);
                    ctState = keyboardState;
                    RX0_COMPLETE_FLAG = 0;
                }
                // Sends ADC values of joysticks to OLED display.
                sprintf(Joystickbuffer, "X1:%4d", Adcvalues[X1]);
                sendStrXY(Joystickbuffer, 0, 0);
                sprintf(Joystickbuffer, "Y1:%4d", Adcvalues[Y1]);
                sendStrXY(Joystickbuffer, 2, 0);
                sprintf(Joystickbuffer, "X2:%4d", Adcvalues[X2]);
                sendStrXY(Joystickbuffer, 4, 0);
                sprintf(Joystickbuffer, "Y2:%4d", Adcvalues[Y2]);
                sendStrXY(Joystickbuffer, 6, 0);
                Adcready = false;
                setpwm(Adcvalues, PWMbuffer); // updates servo motor positions.
                _delay_ms(1); // tiny delay to make sure Oled updates correctly
            }
            break;
            case notmet: // if no confirmed controlstate loop back.
            break;

        default:

            break;
        }
    }
}
// ADC service routine stores ADC value in selected channel.
ISR(ADC_vect)
{
    Adcready = true;
    Adcvalues[channel] = ADC;
}
// Timer interrupt service routine. updates channel and enables adc 
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
// USART interrupt service routine. stores char received in directionvar
ISR(USART0_RX_vect)
{
    directionvar = UDR0;
    RX0_COMPLETE_FLAG = 1;
}
