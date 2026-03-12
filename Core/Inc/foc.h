#ifndef __FOC_H
#define __FOC_H

#include "main.h"

// Open Loop Control Function
void Motor_OpenLoop_Drive(void);

// Zero Alignment and Polarity Check
void Motor_Align_Zero(void);
void Motor_Check_Polarity(void);

// Getter for observed data (for VOFA)
float FOC_GetElectricalAngle(void);

// Global variable for Closed Loop (Plan B)
extern float global_actual_angle;
extern volatile uint16_t spy_raw_angle;   // 间谍变量
extern volatile uint16_t spy_elec_angle;   // 变量

extern float FOC_Ia, FOC_Ib, FOC_Ic;
extern float Ialpha, Ibeta;
extern float Id, Iq;
extern float elec_angle;
extern float Vq,Vd;
extern float Id_target, Iq_target;
extern float target_torque_nm; // 目标扭矩 (Nm)

void FOC_Set_Torque_Nm(float torque);
void FOC_Update_Angle(void);
void FOC_Interrupt_Routine(uint16_t adc_a, uint16_t adc_b, uint16_t adc_c);

#endif
