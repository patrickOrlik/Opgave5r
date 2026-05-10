#ifndef PWM_H_
#define PWM_H_

#include <stdint.h>

void init_phase_pwm(void);
void init_phase_pwm3(void);
void init_phase_pwm4(void);
void init_phase_pwm5(void);
void init_all_PWM(void);
void setpwm(volatile unsigned int value[], volatile uint16_t *pwmBuffer[4]);
void setpwm_UART(uint8_t cardinalcord, uint8_t cardinaldirection, volatile uint16_t *pwmBuffer[4]);
long map(long x, long in_min, long in_max, long out_min, long out_max);

#endif
