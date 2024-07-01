#ifndef __PWM__H
#define __PWM__H

#define PWM_GARBAGE 7
#define PWM_RECOVERABLE_GARBAGE 5

void pwm_write(int pwm_pin);
void pwm_stop(int pwm_pin);

#endif
