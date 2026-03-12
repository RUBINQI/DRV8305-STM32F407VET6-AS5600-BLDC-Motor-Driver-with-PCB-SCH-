#ifndef __LED_H
#define __LED_H

#include "main.h"

// LED Initialization
void LED_Init(void);

// Individual Control
void LED_Red_On(void);
void LED_Red_Off(void);
void LED_Red_Toggle(void);

void LED_Green_On(void);
void LED_Green_Off(void);
void LED_Green_Toggle(void);

void LED_Blue_On(void);
void LED_Blue_Off(void);
void LED_Blue_Toggle(void);

// Status Indication
void LED_SetRed(void);   // Only Red ON
void LED_SetGreen(void); // Only Green ON
void LED_SetBlue(void);  // Only Blue ON
void LED_SetWhite(void); // All ON

// Error Loops
void LED_Error_Loop(void); // Infinite Red Blink loop

#endif
