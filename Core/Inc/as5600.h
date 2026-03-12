#ifndef __AS5600_H
#define __AS5600_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

// ==========================================
// 1. AS5600 I2C 8位通信地址 (极度重要)
// 原始7位地址是 0x36，左移1位后是 0x6C。
// 这里直接定死为 0x6C，.c 文件中绝不可再写 << 1 ！！！
// ==========================================
#define AS5600_ADDR             0x6C

// ==========================================
// 2. AS5600 寄存器地址定义
// ==========================================
#define AS5600_REG_ZMCO         0x00
#define AS5600_REG_ZPOS_H       0x01
#define AS5600_REG_ZPOS_L       0x02
#define AS5600_REG_MPOS_H       0x03
#define AS5600_REG_MPOS_L       0x04
#define AS5600_REG_MANG_H       0x05
#define AS5600_REG_MANG_L       0x06
#define AS5600_REG_CONF_H       0x07
#define AS5600_REG_CONF_L       0x08
#define AS5600_REG_RAW_ANGLE    0x0C // 指向 RAW_ANGLE 高位，用于读取12位原始角度
#define AS5600_REG_RAW_ANGLE_H  0x0C
#define AS5600_REG_RAW_ANGLE_L  0x0D
#define AS5600_REG_ANGLE_H      0x0E // 指向 ANGLE 高位，用于读取修正后角度
#define AS5600_REG_ANGLE_L      0x0F
#define AS5600_REG_STATUS       0x0B
#define AS5600_REG_AGC          0x1A
#define AS5600_REG_MAGNITUDE_H  0x1B
#define AS5600_REG_MAGNITUDE_L  0x1C

// ==========================================
// 3. 状态位定义 (严格按照数据手册第20页修正)
// ==========================================
#define AS5600_STATUS_MD        (1 << 5) // Bit 5 (0x20): 检测到磁铁 (Magnet was Detected)
#define AS5600_STATUS_ML        (1 << 4) // Bit 4 (0x10): 磁场过弱 (Magnet too Weak)
#define AS5600_STATUS_MH        (1 << 3) // Bit 3 (0x08): 磁场过强 (Magnet too Strong)

// ==========================================
// 4. 函数声明
// ==========================================
void AS5600_Init(I2C_HandleTypeDef *hi2c);
void AS5600_ReadAngleDMA(void);
uint16_t AS5600_GetRawAngle(void);
uint16_t AS5600_GetAngle(void);
uint8_t AS5600_GetStatus(void);
uint8_t AS5600_IsMagnetDetected(void);
uint8_t AS5600_IsMagnetTooStrong(void);
uint8_t AS5600_IsMagnetTooWeak(void);
float AS5600_GetAngleDegrees(void);

#ifdef __cplusplus
}
#endif

#endif /* __AS5600_H */
