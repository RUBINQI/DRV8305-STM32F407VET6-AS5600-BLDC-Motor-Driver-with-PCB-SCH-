#include "foc.h"
#include "tim.h"
#include "arm_math.h"
#include "adc.h"

#define PWM_MAX_DUTY    4199.0f         // TIM1 ARR 值 (决定 PWM 频率)
#define POLE_PAIRS      7.0f            // 电机极对数
#define FOC_PERIOD      0.00005f        // FOC 控制周期 (1/20kHz = 50us)
#define ANGLE_COEFF     (6.2831853f / 4096.0f) // 角度换算常数 (2PI / 12bit)

// 数学常数定义 (提高可读性)
#define SQRT3_BY_2      0.8660254f      // sqrt(3) / 2
#define ONE_BY_SQRT3    0.5773503f      // 1 / sqrt(3)
#define TWO_PI          6.2831853f      // 2 * PI

// FOC 核心变量定义
float zero_offset_rad = 1.287010f;  // 机械零点偏移(需校准)
int8_t sensor_direction = 1;      // 传感器机角度递增方向 (1 或 -1)
volatile uint16_t spy_raw_angle = 0; // 原始角度 (由主循环 I2C 读取)

// FOC 状态变量 (全局暴露，供 VOFA 观测)
float FOC_Ia = 0.0f, FOC_Ib = 0.0f, FOC_Ic = 0.0f; // 相电流 (A)
float Ialpha = 0.0f, Ibeta = 0.0f;                 // Clarke 变换后电流
float Id = 0.0f, Iq = 0.0f;                        // Park 变换后电流 (DQ轴)
float elec_angle = 0.0f;                           // 电角度 (rad)

// FOC 控制目标
float Id_target = 0.0f;     // D轴目标电流 (弱磁时才需非0)
float Iq_target = 0.2f;     // Q轴目标电流 (扭矩电流)
float target_torque_nm = 0.0f; // 用户设置的目标扭矩

// FOC 中间电压变量
float Vd = 0.0f, Vq = 0.0f;        // Park 逆变换输入
float Valpha = 0.0f, Vbeta = 0.0f; // SVPWM 输入

// 3. PI 控制器
typedef struct 
{
    float kp;
    float ki;
    float integral;
    float limit;
} PI_Controller;

PI_Controller pid_d = { .kp = 1.0f, .ki = 200.0f, .integral = 0.0f, .limit = 0.8f };
PI_Controller pid_q = { .kp = 1.0f, .ki = 200.0f, .integral = 0.0f, .limit = 0.8f };

// 电机参数 (2208-80T)
#define MOTOR_KV        100.0f              // 电机 KV 值 (RPM/V)
#define MOTOR_KT        (8.27f / MOTOR_KV)  // 估算的扭矩常数 (Nm/A) ≈ 0.0827 Nm/A

// -----------------------------------------------------------------------------
// 内部使用的通用 PI 函数 (带周期参数)
// -----------------------------------------------------------------------------
static float PI_Run_Generic(PI_Controller *pid, float target, float feedback, float dt)
{
    float error = target - feedback;

    // 积分项累加
    pid->integral += pid->ki * error * dt; 

    // 积分限幅 (抗饱和)
    if(pid->integral > pid->limit) pid->integral = pid->limit;
    if(pid->integral < -pid->limit) pid->integral = -pid->limit;

    // 计算输出
    float output = pid->kp * error + pid->integral;

    // 输出限幅
    if(output > pid->limit) output = pid->limit;
    if(output < -pid->limit) output = -pid->limit;

    return output;
}

/**
 * @brief 设置目标扭矩 (纯力矩模式)
 */
void FOC_Set_Torque_Nm(float torque)
{
    target_torque_nm = torque;
    // 1. 扭矩 -> 电流转换: Iq = Torque / Kt
    float iq_req = torque / MOTOR_KT;
    
    // 2. 安全限幅 (2208电机最大电流约 0.5A)
    const float max_current = 0.5f;
    if(iq_req > max_current) iq_req = max_current;
    if(iq_req < -max_current) iq_req = -max_current;
    
    // 3. 更新 FOC 目标值
    Iq_target = iq_req;
    Id_target = 0.0f;
}

/**
 * @brief PI 控制器计算函数
 * @param pid PI 控制器结构体指针
 * @param target 目标值
 * @param feedback 反馈值
 * @return 控制输出
 */
static float PI_Run(PI_Controller *pid, float target, float feedback)
{
    return PI_Run_Generic(pid, target, feedback, FOC_PERIOD);
}

// =================================================================================
// 4. FOC 中断服务函数 (20kHz 高频运行)
// =================================================================================
void FOC_Interrupt_Routine(uint16_t adc_a, uint16_t adc_b, uint16_t adc_c) {

    // ---------------------------------------------------------
    // 步骤 1: 电流采样与物理量转换
    // ---------------------------------------------------------
    // 公式: I = (ADC_Raw - Offset) * Coeff
    // Coeff 包含 Vref, Gain(80), Rshunt(0.05)
    FOC_Ia = ((float)((int32_t)adc_a - adc_ia_offset)) * CURRENT_COEFF;
    FOC_Ib = ((float)((int32_t)adc_b - adc_ib_offset)) * CURRENT_COEFF;
    // FOC_Ic = ((float)((int32_t)adc_c - adc_ic_offset)) * CURRENT_COEFF;
    //   C相电流冗余，Clarke变换只需Ia, Ib

    // 调试用：利用 Kirchhoff 定律 Ia+Ib+Ic=0 检查采样误差
    FOC_Ic = -(FOC_Ia + FOC_Ib);

    // ---------------------------------------------------------
    // 步骤 2: 角度计算 (机械角度 -> 电角度)
    // ---------------------------------------------------------
    float raw_rad = (float)spy_raw_angle * ANGLE_COEFF;
    float mech_angle = (float)sensor_direction * (raw_rad - zero_offset_rad);
    
    elec_angle = mech_angle * POLE_PAIRS;
    
    // 归一化到 [0, 2PI]
    // 虽然 while 循环比 if 慢微秒级，但逻辑最安全，防止多圈溢出
    while (elec_angle < 0.0f)     elec_angle += TWO_PI;
    while (elec_angle >= TWO_PI)  elec_angle -= TWO_PI;

    // 分别计算正余弦-弧度制
    float sin_theta = arm_sin_f32(elec_angle);
    float cos_theta = arm_cos_f32(elec_angle);

    // ---------------------------------------------------------
    // 步骤 3: Clarke 变换 (3相 -> 2相静止)
    // ---------------------------------------------------------
    // Ia, Ib -> Ialpha, Ibeta
    arm_clarke_f32(FOC_Ia, FOC_Ib, &Ialpha, &Ibeta);
    
    // ---------------------------------------------------------
    // 步骤 4: Park 变换 (2相静止 -> 2相旋转)
    // ---------------------------------------------------------
    // Ialpha, Ibeta -> Id, Iq
    arm_park_f32(Ialpha, Ibeta, &Id, &Iq, sin_theta, cos_theta);

    // ---------------------------------------------------------
    // 步骤 5: 电流环 PI 控制
    // ---------------------------------------------------------
    // 目标: Id -> 0, Iq -> Target
    Vd = PI_Run(&pid_d, Id_target, Id);
    Vq = PI_Run(&pid_q, Iq_target, Iq);

    // ---------------------------------------------------------
    // 步骤 6: 反 Park 变换 (2相旋转 -> 2相静止)
    // ---------------------------------------------------------
    // Vd, Vq -> Valpha, Vbeta
    arm_inv_park_f32(Vd, Vq, &Valpha, &Vbeta, sin_theta, cos_theta);

    // ---------------------------------------------------------
    // 步骤 7: SVPWM 生成 (简化版)
    // ---------------------------------------------------------
    // 使用逆 Clarke 变换计算三相电压，并进行归一化处理
    // 标准逆 Clarke 公式:
    // Va = Valpha
    // Vb = -0.5 * Valpha + (sqrt(3)/2) * Vbeta
    // Vc = -0.5 * Valpha - (sqrt(3)/2) * Vbeta
    //
    // 此处额外乘以 (1/sqrt(3)) 系数，是为了适配 SVPWM 的电压范围
    
    float Va = Valpha * ONE_BY_SQRT3;
    float Vb = (-0.5f * Valpha + SQRT3_BY_2 * Vbeta) * ONE_BY_SQRT3;
    float Vc = (-0.5f * Valpha - SQRT3_BY_2 * Vbeta) * ONE_BY_SQRT3;

    // 归一化 PWM (假设 Valpha/Vbeta 为归一化电压)
    // 中心对齐 PWM: 0.5 为 0V, 0.0/1.0 为峰值
    float v_a = 0.5f + Va;
    float v_b = 0.5f + Vb;
    float v_c = 0.5f + Vc;

    // 安全限幅
    if (v_a > 1.0f) v_a = 1.0f; else if (v_a < 0.0f) v_a = 0.0f;
    if (v_b > 1.0f) v_b = 1.0f; else if (v_b < 0.0f) v_b = 0.0f;
    if (v_c > 1.0f) v_c = 1.0f; else if (v_c < 0.0f) v_c = 0.0f;

    // ---------------------------------------------------------
    // 步骤 8: 更新 PWM 占空比
    // ---------------------------------------------------------
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, (uint32_t)(v_a * PWM_MAX_DUTY));
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, (uint32_t)(v_b * PWM_MAX_DUTY));
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, (uint32_t)(v_c * PWM_MAX_DUTY));
    
}
