#include "led.h"

// LED Initialization
void LED_Init(void)
{
    // Initialize all LEDs to OFF or a specific state
    // According to user previous code, init was all ON (White). 
    // Let's keep it or set to OFF. Usually OFF or Green is better. 
    // Let's set to White as per previous logic "上电白灯常亮".
    LED_SetWhite();
}

// Red LED
void LED_Red_On(void) {
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
}
void LED_Red_Off(void) {
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
}
void LED_Red_Toggle(void) {
    HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
}

// Green LED
void LED_Green_On(void) {
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
}
void LED_Green_Off(void) {
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
}
void LED_Green_Toggle(void) {
    HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
}

// Blue LED
void LED_Blue_On(void) {
    HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_RESET);
}
void LED_Blue_Off(void) {
    HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_SET);
}
void LED_Blue_Toggle(void) {
    HAL_GPIO_TogglePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
}

// Status Helpers
void LED_SetRed(void) {
    LED_Red_On();
    LED_Green_Off();
    LED_Blue_Off();
}
void LED_SetGreen(void) {
    LED_Red_Off();
    LED_Green_On();
    LED_Blue_Off();
}
void LED_SetBlue(void) {
    LED_Red_Off();
    LED_Green_Off();
    LED_Blue_On();
}
void LED_SetWhite(void) {
    LED_Red_On();
    LED_Green_On();
    LED_Blue_On();
}

// Error Loop
void LED_Error_Loop(void) {
    while(1) {
        LED_Red_Toggle();
        HAL_Delay(100);
    }
}
