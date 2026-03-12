#ifndef __VOFA_H
#define __VOFA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

// VOFA JustFloat 协议
// 数据格式: 浮点数组 + 帧尾 (00 00 80 7F)

void VOFA_Init(UART_HandleTypeDef *huart);
void VOFA_Send(float *data, uint8_t count);

#ifdef __cplusplus
}
#endif

#endif /* __VOFA_H */
