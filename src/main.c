#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include "I2C.h"
#include "ssd1306.h"
#include "clock.h"
#define X1 0
#define Y1 1 
#define X2 2
#define Y2 3

unsigned int value = 0;
bool Adcready = false;
volatile char Adcres[10] = {0};
unsigned int channel = 0;
unsigned int Adcvalues[4] = {0};
char tnpbuff [4]= {0};

void init_phase_pwm()
{
    DDRB |= (1<<PB5);
    TCCR1A |= (1<<COM1A1);
    TCCR1B =(1<<CS11)|(1<<WGM13);
    ICR1 = 20000;
    OCR1A = 1500;
}

void init_adc(){
ADCSRA |= (1<<ADPS0)|(1<<ADPS1)|(1<<ADPS2); // intern clock 125khz
ADMUX |= (1<<REFS0); // Voltage reference selection
ADCSRA |= (1<<ADEN) | (1<<ADIE); // enable adc and interrupt complete

}
void select_channel(char channel){
ADMUX = 0x40;
ADMUX |= channel;
}


int main(){
  init_phase_pwm();
  init_adc();
  sei();
  CTC_init();
  I2C_Init();
  InitializeDisplay();
  clear_display();
  char buffer[16];
  while(1){
   if (Adcready) {
   
 sprintf(buffer, "X1:%4d", Adcvalues[X1]);
 sendStrXY(buffer, 0, 0);
 sprintf(buffer, "Y1:%4d", Adcvalues[Y1]);
 sendStrXY(buffer, 2, 0);
 sprintf(buffer, "X2:%4d", Adcvalues[X2]);
 sendStrXY(buffer, 4, 0);
 sprintf(buffer, "Y2:%4d", Adcvalues[Y2]);
 sendStrXY(buffer, 6, 0);
 Adcready= false;
 
_delay_ms(5);

    }


  }


}
ISR(ADC_vect){

  Adcready = true;
  Adcvalues[channel] = ADC;
}


ISR(TIMER0_COMPA_vect) {

    // Select next channel
    select_channel(channel);
    // Start ADC conversion
    ADCSRA |= (1<<ADSC); //enables adc
   
    channel++;
    if (channel > 3) {
        channel = 0;
    }
}

