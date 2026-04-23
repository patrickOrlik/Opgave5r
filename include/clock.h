#include "stdbool.h"




#define Sec 0
#define Min 1
#define Hour 2

// Function declarations
void clockFunc(void);
void CTC_init(void);
void external_int(void);
void checkCLOCKstate(void);
bool Alarmreached(void);
void getstring(char* string);
bool buttonclick(void);

// Extern global variables (defined in main.c)
extern int Clock[3];
extern int Alarm[3];
extern char ClockBuffer[32];
extern char GlobalRxBuffer[];
extern bool button;



