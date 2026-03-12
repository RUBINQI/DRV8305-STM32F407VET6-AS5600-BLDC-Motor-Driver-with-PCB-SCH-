#include "as5600.h"

static I2C_HandleTypeDef *as5600_hi2c;
static uint8_t as5600_rx_buf[2];
extern volatile uint16_t spy_raw_angle; // Defined in foc.c

// ========================================================
// 初始化 AS5600
// ========================================================
void AS5600_Init(I2C_HandleTypeDef *hi2c) {
    as5600_hi2c = hi2c;
}

// ========================================================
// 读取原始角度 (0-4095) - FOC 闭环最核心函数
// ========================================================
uint16_t AS5600_GetRawAngle(void) {
    uint8_t data[2] = {0, 0};

    // 绝不使用 << 1，因为你的头文件已经是 (0x36 << 1) 了
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(as5600_hi2c, AS5600_ADDR, AS5600_REG_RAW_ANGLE, I2C_MEMADD_SIZE_8BIT, data, 2, 100);

    if (status != HAL_OK) {
        // 返回 0xFFFF(65535) 作为明显错误标志，与真实的 0 度区分开
        return 0xFFFF;
    }

    // (data[0] & 0x0F) 用于屏蔽最高4位的状态位杂波，确保数据只占 12 位
    return (((uint16_t)data[0] & 0x0F) << 8) | data[1];
}

// ========================================================
// 读取角度 (经过芯片内部零点修正后的值)
// ========================================================
uint16_t AS5600_GetAngle(void) {
    uint8_t data[2] = {0, 0};

    if (HAL_I2C_Mem_Read(as5600_hi2c, AS5600_ADDR, AS5600_REG_ANGLE_H, I2C_MEMADD_SIZE_8BIT, data, 2, 100) != HAL_OK) {
        return 0xFFFF;
    }

    return (((uint16_t)data[0] & 0x0F) << 8) | data[1];
}

// ========================================================
// 读取状态寄存器 (0x0B)
// ========================================================
uint8_t AS5600_GetStatus(void) {
    uint8_t status = 0;

    HAL_I2C_Mem_Read(as5600_hi2c, AS5600_ADDR, AS5600_REG_STATUS, I2C_MEMADD_SIZE_8BIT, &status, 1, 100);

    return status;
}

// ========================================================
// 检测是否有磁铁 (1:检测到, 0:未检测到)
// ========================================================
uint8_t AS5600_IsMagnetDetected(void) {
    // 强制使用手册规定的 0x20 (Bit 5)，防止头文件宏定义出错
    return (AS5600_GetStatus() & 0x20) ? 1 : 0;
}

// ========================================================
// 检测磁场是否过强
// ========================================================
uint8_t AS5600_IsMagnetTooStrong(void) {
    // 强制使用手册规定的 0x08 (Bit 3)
    return (AS5600_GetStatus() & 0x08) ? 1 : 0;
}

// ========================================================
// 检测磁场是否过弱
// ========================================================
uint8_t AS5600_IsMagnetTooWeak(void) {
    // 强制使用手册规定的 0x10 (Bit 4)
    return (AS5600_GetStatus() & 0x10) ? 1 : 0;
}

// ========================================================
// 获取角度 (度) - 用于人机交互观察
// ========================================================
float AS5600_GetAngleDegrees(void) {
    uint16_t raw_angle = AS5600_GetRawAngle();

    // 如果通讯失败，返回 0.0f (或者你也可以让它返回负数报警)
    if (raw_angle == 0xFFFF) {
        return 0.0f;
    }

    return (float)raw_angle * 360.0f / 4096.0f;
}

// ========================================================
// DMA 非阻塞读取角度
// ========================================================
void AS5600_ReadAngleDMA(void) {
    if (as5600_hi2c->State == HAL_I2C_STATE_READY) {
        HAL_I2C_Mem_Read_DMA(as5600_hi2c, AS5600_ADDR, AS5600_REG_RAW_ANGLE, I2C_MEMADD_SIZE_8BIT, as5600_rx_buf, 2);
    }
}

// ========================================================
// I2C 接收完成回调 (DMA 完成后自动调用)
// ========================================================
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
    if (hi2c->Instance == as5600_hi2c->Instance) {
        // 1. 更新角度
        spy_raw_angle = ((uint16_t)(as5600_rx_buf[0] & 0x0F) << 8) | as5600_rx_buf[1];
    }
}
