#include "debug.h"
#include "usart.h"
#include "drv8305.h"
#include "led.h"
#include <stdio.h>
#include <string.h>

// Redirect printf to UART
int _write(int file, char *ptr, int len)
{
  HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, 100);
  return len;
}

// Debug Initialization
void Debug_Init(void)
{
    // Send a startup message to UART
    uint8_t hello[] = "System Starting...\r\n";
    HAL_UART_Transmit(&huart1, hello, sizeof(hello)-1, 100);
}

// Check DRV8305 Registers and Faults
void Debug_Check_DRV8305(void)
{
    // Read Status Registers
    uint16_t status_01 = DRV8305_ReadRegister(0x01); // Warning & Watchdog
    uint16_t status_02 = DRV8305_ReadRegister(0x02); // OV/VDS Faults
    uint16_t status_03 = DRV8305_ReadRegister(0x03); // IC Faults
    uint16_t status_05 = DRV8305_ReadRegister(0x05); // HS Gate Drive Control (Validation)
    uint16_t status_06 = DRV8305_ReadRegister(0x0c); // HS Gate Drive Control (Validation)

    // Print to UART
    char msg[64];
    sprintf(msg, "寄存器地址：0x01 信息：0x%04X\r\n", status_01);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);
    
    sprintf(msg, "寄存器地址：0x02 信息：0x%04X\r\n", status_02);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);
    
    sprintf(msg, "寄存器地址：0x03 信息：0x%04X\r\n", status_03);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);

    sprintf(msg, "寄存器地址：0x05 信息：0x%04X (Validation)\r\n", status_05);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);

    sprintf(msg, "寄存器地址：0x0c 信息：0x%04X (Validation)\r\n", status_06);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);
    
         // Check nFAULT Pin
    if (DRV8305_GetFaultStatus() == 0) { // 0 means Fault (Low Active)
         // Fault detected
         // We can print a message here too
         uint8_t fault_msg[] = "nFAULT Detected! Skipping Error Loop for Debugging.\r\n";
         HAL_UART_Transmit(&huart1, fault_msg, sizeof(fault_msg)-1, 100);
         
         // Enter Error Loop (Red Blink)
         // LED_Error_Loop(); // Disable blocking for debug
         LED_Red_On(); // Turn on Red LED to indicate fault but continue
         LED_Green_Off();
         LED_Blue_Off();
    }
}
