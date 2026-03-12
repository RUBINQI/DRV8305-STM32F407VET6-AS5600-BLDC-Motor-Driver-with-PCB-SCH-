#include "drv8305.h"
#include "gpio.h" // 根据实际情况包含
#include "usart.h"
static SPI_HandleTypeDef *drv_hspi;

volatile uint16_t verify;
static uint8_t dccal_enabled = 0;
// ========= 基础 SPI 读写函数  =========

uint16_t DRV8305_ReadRegister(uint8_t regAddr) {

    uint16_t txData = (1 << 15) | ((regAddr & 0x0F) << 11);
    uint16_t rxData = 0;

    HAL_GPIO_WritePin(DRV8305_CS_GPIO_Port, DRV8305_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(drv_hspi, (uint8_t*)&txData, (uint8_t*)&rxData, 1, 100);
    HAL_GPIO_WritePin(DRV8305_CS_GPIO_Port, DRV8305_CS_Pin, GPIO_PIN_SET);
    
    for(volatile int i = 0; i < 10; i++) { __NOP(); } 

    return rxData & 0x07FF; 
}

void DRV8305_WriteRegister(uint8_t regAddr, uint16_t data) {
    uint16_t txData = (0 << 15) | ((regAddr & 0x0F) << 11) | (data & 0x07FF);
    uint16_t rxData = 0;

    HAL_GPIO_WritePin(DRV8305_CS_GPIO_Port, DRV8305_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(drv_hspi, (uint8_t*)&txData, (uint8_t*)&rxData, 1, 100);
    HAL_GPIO_WritePin(DRV8305_CS_GPIO_Port, DRV8305_CS_Pin, GPIO_PIN_SET);
    
    for(volatile int i = 0; i < 10; i++) { __NOP(); } 
}

// ========= 状态控制与读取 =========

uint8_t DRV8305_GetFaultStatus(void) {
    return HAL_GPIO_ReadPin(nFAULT_GPIO_Port, nFAULT_Pin);
}


// 清除锁存的故障 
void DRV8305_ClearFaults(void) {
    //读出寄存器 0x09 的当前真实值
    uint16_t current_val = DRV8305_ReadRegister(0x09);
  // 使用按位或 (OR) 操作，仅将 Bit 1 (CLR_FLTS) 置为 1，其余位原封不动
    current_val |= 0x0002; 
    DRV8305_WriteRegister(0x09, current_val);
}

// gain_setting: 0=10V/V, 1=20V/V, 2=40V/V, 3=80V/V
void DRV8305_SetShuntGain(uint8_t gain_setting) {
    uint16_t current_val = DRV8305_ReadRegister(0x0A);
    // 清除原来的 CH1, CH2, CH3 增益位 (位 0~5)
    current_val &= ~(0x3F); 
    // 将三个通道设置为相同的增益
    current_val |= (gain_setting << 4) | (gain_setting << 2) | gain_setting;
    DRV8305_WriteRegister(0x0A, current_val);
}

void DRV8305_EnableDCCal(void)
{
    uint16_t reg = DRV8305_ReadRegister(0x0A);
    reg |= (CSA_CAL_A | CSA_CAL_B | CSA_CAL_C);
    DRV8305_WriteRegister(0x0A, reg);
    dccal_enabled = 1;
}

void DRV8305_DisableDCCal(void)
{
    uint16_t reg = DRV8305_ReadRegister(0x0A);
    reg &= ~(CSA_CAL_A | CSA_CAL_B | CSA_CAL_C);
    DRV8305_WriteRegister(0x0A, reg);
    dccal_enabled = 0;
}

// ========= 初始化序列优化 =========

void DRV8305_Init(SPI_HandleTypeDef *hspi) {
    drv_hspi = hspi;
    
    // 1. 确保 EN_GATE 为低电平 (待机模式，绝对安全)
    DRV8305_Disable();

    // 2. 唤醒设备
    HAL_GPIO_WritePin(WAKE_GPIO_Port, WAKE_Pin, GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(WAKE_GPIO_Port, WAKE_Pin, GPIO_PIN_SET);
    HAL_Delay(10); // 等待内部稳压器和逻辑电平就绪

    // 配置 0x05 (高侧门极)：拉满 1.25A/1A 峰值电流，2000ns 峰值时间
    // 二进制: 0000 0011 1011 1011 -> 十六进制: 0x03BB
    DRV8305_WriteRegister(0x05, 0x03BB);

    // 配置 0x06 (低侧门极)：拉满 1.25A/1A 峰值电流，2000ns 峰值时间
    // 二进制: 0000 0011 1011 1011 -> 十六进制: 0x03BB
    DRV8305_WriteRegister(0x06, 0x03BB);

    // 配置 0x07 (驱动时序)：消隐时间(TBLANK)和抗噪时间(TVDS)拉满到 8us，躲避米勒平台振荡
    // 二进制: 0000 0000 0000 1111 -> 十六进制: 0x000F
    DRV8305_WriteRegister(0x07, 0x000F);

    // 配置 0x0C (VDS保护)：保护阈值拉宽到 2.13V，防止瞬态尖峰误触发
    // 二进制: 0000 0000 1111 1000 -> 十六进制: 0x00F8
    DRV8305_WriteRegister(0x0C, 0x00F8); 

    // 默认设置电流放大器增益 0=10V/V, 1=20V/V, 2=40V/V, 3=80V/V
    DRV8305_SetShuntGain(3);
   // DRV8305_ReadRegister(0x0A);
   // verify = DRV8305_ReadRegister(0x0A);
    // 4. 清除一切历史遗留故障标志 (比如刚上电时的欠压警告)
    // 写入 0x0002，对应寄存器 0x09 的 CLR_FLTS 位，绝不能写错导致休眠
    DRV8305_ClearFaults();

    // 5. 初始化完毕。依然保持 EN_GATE 为低。
    // 交由主程序在 PWM 稳定输出 50% 之后，再显式调用 DRV8305_Enable()。
}

void DRV8305_Enable(void) {
    HAL_GPIO_WritePin(EN_GATE_GPIO_Port, EN_GATE_Pin, GPIO_PIN_SET);
    // 电荷泵起振大约需要 1~2 毫秒，10ms 足够稳定
    HAL_Delay(10); 
}

void DRV8305_Disable(void) {
    HAL_GPIO_WritePin(EN_GATE_GPIO_Port, EN_GATE_Pin, GPIO_PIN_RESET);
}
