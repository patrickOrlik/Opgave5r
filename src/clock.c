#include <stdio.h>
#include "clock.h"
#include "stdbool.h"
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

#define Sec 0
#define Min 1
#define Hour 2



void CTC_init (void)
{
    TCCR0B |= (1<<CS11)|(1<<CS10);// sets prescaler to 64
    TCCR0B |= (1<<WGM12); // enables CTC mode 
    OCR0A = 249; // sets output compare match amount
    TIMSK0 |= (1<<OCIE1A); // enable interrupt
}

void external_int()
{
    DDRE &=~(1<<5);
    PORTE |=(1<<5); //internal pullup on digital pin 18

    EICRB = 0x02; //falling edge
    EIMSK |= (1<<INT5);
}

void checkCLOCKstate()

{
    int len = strlen(GlobalRxBuffer);
     char tmpbuffer[128];
    if( len < 11){
         sscanf(GlobalRxBuffer,"%d:%d:%d",&Clock[Hour],&Clock[Min],&Clock[Sec]);
         sprintf(tmpbuffer,"du har indstillet uret til %d:%d:%d \n,",Clock[Hour],Clock[Min],Clock[Sec]);
        putstringuart(tmpbuffer);
    }
    else {
    sscanf(GlobalRxBuffer,"%d:%d:%d %d:%d:%d",&Clock[Hour],
        &Clock[Min],&Clock[Sec],&Alarm[Hour],&Alarm[Min],&Alarm[Sec]);
    sprintf(tmpbuffer,"du har indstillet uret til %d:%d:%d \n  og Alarmen er indstillet til %d:%d:%d \n,",Clock[Hour],Clock[Min],Clock[Sec],Alarm[Hour],Alarm[Min],Alarm[Sec]);
    putstringuart(tmpbuffer);
    }

}


bool Alarmreached(){
      if (Clock[Hour]==Alarm[Hour] &&
    Clock[Min]==Alarm[Min]  &&
    Clock[Sec]==Alarm[Sec]){
    
    return true;
    }
    else{
     return false;
    }
}




