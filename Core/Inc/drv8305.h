#ifndef __DRV8305_H
#define __DRV8305_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

// DRV8305 寄存器地址 (4 bits: 14-11)
#define DRV8305_REG_WARNINGS        0x01
#define DRV8305_REG_OV_VDS_STATUS   0x02
#define DRV8305_REG_IC_STATUS       0x03
#define DRV8305_REG_VGS_STATUS      0x04
#define DRV8305_REG_HS_GATE_DRIVE   0x05
#define DRV8305_REG_LS_GATE_DRIVE   0x06
#define DRV8305_REG_GATE_DRIVE_CTRL 0x07
#define DRV8305_REG_IC_OPERATION    0x09
#define DRV8305_REG_SHUNT_AMP_CTRL  0x0A
#define DRV8305_REG_VREG_CTRL       0x0B
#define DRV8305_REG_VDS_SENSE_CTRL  0x0C

// SPI 命令格式
#define DRV8305_READ_FLAG           (1 << 15)
#define DRV8305_WRITE_FLAG          (0 << 15)
#define DRV8305_ADDR_SHIFT          11
#define DRV8305_DATA_MASK           0x07FF

// 引脚定义 (用户应在 main.h 或 CubeMX 中定义)
// 示例:
// #define DRV8305_EN_GATE_Pin       GPIO_PIN_0
// #define DRV8305_EN_GATE_GPIO_Port GPIOB
// #define DRV8305_nFAULT_Pin        GPIO_PIN_1
// #define DRV8305_nFAULT_GPIO_Port  GPIOB
// #define DRV8305_WAKE_Pin          GPIO_PIN_2
// #define DRV8305_WAKE_GPIO_Port    GPIOB
// #define DRV8305_nSCS_Pin          GPIO_PIN_4
// #define DRV8305_nSCS_GPIO_Port    GPIOA

// 函数声明
void DRV8305_Init(SPI_HandleTypeDef *hspi);
uint16_t DRV8305_ReadRegister(uint8_t regAddr);
void DRV8305_WriteRegister(uint8_t regAddr, uint16_t data);
void DRV8305_Enable(void);
void DRV8305_Disable(void);
uint8_t DRV8305_GetFaultStatus(void);
void DRV8305_EnableDCCal(void);
void DRV8305_DisableDCCal(void);

#define CSA_CAL_A   (1U << 8)
#define CSA_CAL_B   (1U << 9)
#define CSA_CAL_C   (1U << 10)


extern volatile uint16_t verify;

#ifdef __cplusplus
}
#endif

#endif /* __DRV8305_H */
