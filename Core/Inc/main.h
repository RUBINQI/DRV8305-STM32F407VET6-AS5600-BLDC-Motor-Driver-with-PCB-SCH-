/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BEEP_Pin GPIO_PIN_2
#define BEEP_GPIO_Port GPIOE
#define I_A_Pin GPIO_PIN_0
#define I_A_GPIO_Port GPIOC
#define I_B_Pin GPIO_PIN_1
#define I_B_GPIO_Port GPIOC
#define I_C_Pin GPIO_PIN_2
#define I_C_GPIO_Port GPIOC
#define AS5600_ANGLE_Pin GPIO_PIN_3
#define AS5600_ANGLE_GPIO_Port GPIOC
#define nFAULT_Pin GPIO_PIN_0
#define nFAULT_GPIO_Port GPIOA
#define nFAULT_EXTI_IRQn EXTI0_IRQn
#define WAKE_Pin GPIO_PIN_1
#define WAKE_GPIO_Port GPIOA
#define EN_GATE_Pin GPIO_PIN_2
#define EN_GATE_GPIO_Port GPIOA
#define DRV8305_CS_Pin GPIO_PIN_4
#define DRV8305_CS_GPIO_Port GPIOA
#define DRV8305_SCK_Pin GPIO_PIN_5
#define DRV8305_SCK_GPIO_Port GPIOA
#define DRV8305_MISO_Pin GPIO_PIN_6
#define DRV8305_MISO_GPIO_Port GPIOA
#define DRV8305_MOSI_Pin GPIO_PIN_7
#define DRV8305_MOSI_GPIO_Port GPIOA
#define LED_BLUE_Pin GPIO_PIN_9
#define LED_BLUE_GPIO_Port GPIOD
#define LED_RED_Pin GPIO_PIN_10
#define LED_RED_GPIO_Port GPIOD
#define LED_GREEN_Pin GPIO_PIN_11
#define LED_GREEN_GPIO_Port GPIOD
#define AS5600_SCL_Pin GPIO_PIN_6
#define AS5600_SCL_GPIO_Port GPIOB
#define AS5600_SDA_Pin GPIO_PIN_7
#define AS5600_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
// Map DRV8305 Driver Pins to CubeMX Defines
#define DRV8305_EN_GATE_Pin       EN_GATE_Pin
#define DRV8305_EN_GATE_GPIO_Port EN_GATE_GPIO_Port
#define DRV8305_nFAULT_Pin        nFAULT_Pin
#define DRV8305_nFAULT_GPIO_Port  nFAULT_GPIO_Port
#define DRV8305_WAKE_Pin          WAKE_Pin
#define DRV8305_WAKE_GPIO_Port    WAKE_GPIO_Port
#define DRV8305_nSCS_Pin          DRV8305_CS_Pin
#define DRV8305_nSCS_GPIO_Port    DRV8305_CS_GPIO_Port
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
