#include "vofa.h"
#include <string.h>

static UART_HandleTypeDef *vofa_huart;
static const uint8_t vofa_tail[4] = {0x00, 0x00, 0x80, 0x7f}; // JustFloat 协议帧尾
static uint8_t vofa_dma_buffer[64]; // DMA 发送缓冲区

// VOFA 初始化
void VOFA_Init(UART_HandleTypeDef *huart) {
    vofa_huart = huart;
}

// 发送浮点数组 - DMA 版本
void VOFA_Send(float *data, uint8_t count) {
    if (vofa_huart == NULL) return;

    // 检查 DMA 是否忙碌，如果忙则跳过本次发送（非阻塞）
    if (vofa_huart->gState != HAL_UART_STATE_READY) return;

    // 防止缓冲区溢出
    if ((count * sizeof(float) + 4) > sizeof(vofa_dma_buffer)) return;

    // 拼装数据：数据部分 + 帧尾
    memcpy(vofa_dma_buffer, data, count * sizeof(float));
    memcpy(vofa_dma_buffer + (count * sizeof(float)), vofa_tail, 4);

    // 启动 DMA 发送
    HAL_UART_Transmit_DMA(vofa_huart, vofa_dma_buffer, count * sizeof(float) + 4);
}
