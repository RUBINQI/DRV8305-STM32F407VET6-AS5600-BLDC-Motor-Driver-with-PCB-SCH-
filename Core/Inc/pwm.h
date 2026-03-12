#ifndef __PWM_H
#define __PWM_H

#include "main.h"

// PWM Initialization and Control
void PWM_Init(void);
void PWM_Start(void);
void PWM_Stop(void);
void PWM_SetDuty(uint32_t channel, uint32_t duty);

#endif
