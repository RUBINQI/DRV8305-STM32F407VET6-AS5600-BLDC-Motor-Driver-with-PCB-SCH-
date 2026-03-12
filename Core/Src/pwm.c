#include "pwm.h"
#include "tim.h"

// PWM Initialization
// Note: Basic TIM1 Init is done in tim.c (MX_TIM1_Init)
// This function sets up initial state safely
void PWM_Init(void)
{
    // Set initial duty cycle to 50% (Zero Volts Diff) BEFORE starting PWM
    // ARR = 4199, so 50% is ~2100
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 2100);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 2100);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 2100);
}

// Start PWM Output
void PWM_Start(void)
{
    // Enable PWM Channels
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
    
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
    
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
    
    // Enable Main Output (MOE) - Critical for Advanced Timers
    __HAL_TIM_MOE_ENABLE(&htim1);

    HAL_Delay(3);
}

// Stop PWM Output
void PWM_Stop(void)
{
    // Disable Main Output
    __HAL_TIM_MOE_DISABLE(&htim1);
    
    // Stop Channels
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
    
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
    
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);
}

// Set Duty Cycle for a specific channel
void PWM_SetDuty(uint32_t channel, uint32_t duty)
{
    __HAL_TIM_SET_COMPARE(&htim1, channel, duty);
}
